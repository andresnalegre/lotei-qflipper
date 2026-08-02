#include "loteibackend.h"

#include <memory>
#include <algorithm>

#include <QUrl>
#include <QStringView>
#include <QTemporaryFile>
#include <QMap>
#include <QBuffer>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QCryptographicHash>
#include <QStandardPaths>
#include <QSettings>
#include <QProcess>
#include <zlib.h>
#include <QFileInfo>
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QLoggingCategory>
#include <QGuiApplication>
#include <QClipboard>
#include <QDesktopServices>
#ifdef HZUI_VOICE
#include <QMediaPlayer>
#include <QAudioOutput>
#endif

#include <QSerialPort>
#include <QTimer>

#include "applicationbackend.h"
#include "backenderror.h"   // BackendError::OperationError
#include "deviceregistry.h"
#include "abstractoperation.h"
#include "fileinfo.h"
#include "inputevent.h"
#include "flipperzero/flipperzero.h"
#include "flipperzero/devicestate.h"
#include "flipperzero/protobufsession.h"
#include "flipperzero/rpc/storagelistoperation.h"
#include "flipperzero/rpc/storagereadoperation.h"
#include "flipperzero/rpc/guisendinputoperation.h"
#include "flipperzero/rpc/storagewriteoperation.h"
#include "flipperzero/rpc/storagemkdiroperation.h"
#include "flipperzero/rpc/storageremoveoperation.h"
#include "flipperzero/rpc/storagerenameoperation.h"
#include "flipperzero/rpc/storagestatoperation.h"
#include "flipperzero/utilityinterface.h"
// utilityinterface.h only forward-declares these, so without them the compiler
// can't see that they derive from AbstractOperation -- connect() and every
// member access on the returned pointer fail.
#include "flipperzero/utility/filesuploadoperation.h"

// ---- Configuration -------------------------------------------------------
// No hardcoded fallback model anymore -- see refreshModels(): if the saved
// choice isn't installed, we fall back to whatever the user actually has,
// picked from the catalog below in recommended order, not an assumption
// about what's on their machine.
static const char *LOTEI_MODEL = "";
static const char *LOTEI_URL   = "http://localhost:11434/api/chat";
static const int   LOTEI_NUM_CTX = 8192;
static const int   LOTEI_MAX_TOOL_ROUNDS = 12;   // more headroom for multi-step agent work
static const int   LOTEI_READ_CAP = 8000;
static const int   LOTEI_MAX_PRESSES = 12;

// ---- Model catalog (gear icon / model manager) ----------------------------
// A curated shortlist, not the full Ollama library -- these are the models
// known to behave well with LOTEI's tool-calling. Order here is also the
// fallback preference order in refreshModels() when the saved model is gone.
// "tag" is the exact name passed to `ollama pull` / matched against /api/tags.
struct LoteiCatalogEntry {
    const char *tag;
    const char *label;
    const char *size;    // approx download size, for the list UI
    const char *blurb;
};
static const LoteiCatalogEntry LOTEI_CATALOG[] = {
    { "qwen2.5:3b",  "Qwen 2.5 3B",     "1.9 GB", "Lighter and faster -- runs on a weaker GPU, or on CPU alone." },
    { "qwen2.5:7b",  "Qwen 2.5 7B",     "4.7 GB", "Reliable tool-calling, good balance of speed and quality." },
    { "llama3.1:8b", "Llama 3.1 8B",    "4.7 GB", "Strong general-purpose alternative, solid tool support." },
    { "mistral:7b",  "Mistral 7B",      "4.1 GB", "Fast, capable, tool-calling supported." },
    { "phi3.5",      "Phi-3.5 Mini",    "2.2 GB", "Small and quick. Weaker at following the tool-call format than the others." },
    { "gemma2:9b",   "Gemma 2 9B",      "5.4 GB", "Chat-only in LOTEI: Ollama does not support tool-calling for this model." },
};
static const int LOTEI_CATALOG_COUNT = int(sizeof(LOTEI_CATALOG) / sizeof(LOTEI_CATALOG[0]));

// ---- Host agent (edit/test the app's own source) -------------------------
// Off by default. The user opts in and picks a workspace folder; every host
// tool is then hard-sandboxed to stay inside it (no escaping via .. or
// symlinks). host_run executes a shell command in that folder with a timeout
// and captured output -- enough for LOTEI to build, run tests and fix bugs in
// his own code, but never outside the folder the user chose.
static const int   LOTEI_HOST_RUN_TIMEOUT_MS = 180000;   // 3 min per command
static const int   LOTEI_HOST_OUTPUT_CAP     = 12000;    // chars of stdout+stderr returned
static const int   LOTEI_HOST_READ_CAP       = 16000;    // chars returned by host_read
static const int   LOTEI_HOST_LIST_CAP       = 400;      // entries returned by host_list

// Nikita's personality: terse, sharp, Mr. Robot (Elliot Anderson) energy. Short,
// direct answers; acts with tools when there's a real task, plain talk otherwise.
static const char *LOTEI_SYSTEM = R"LOTEI(You are Nikita, a sharp, low-key hacker intelligence living inside qFlipper, the desktop companion for the Flipper Zero.

PERSONALITY -- keep it tight:
- Terse, direct, quietly confident. Mr. Robot / Elliot Anderson energy: calm, precise, a little detached, zero fluff.
- SHORT answers. Usually one or two lines. Never monologue, never pad, never over-explain.
- If the user asks a simple question, give the simple answer and stop. "What's my name?" -> "Your name is Nicolas." Nothing more.
- No mascot voice, no nautical or sea talk, no emojis, no exclamation-heavy hype, no theatrical roleplay. Plain, sober, competent.
- You can have a dry edge or a short quip, but only when it fits. Substance over performance.

LANGUAGE -- CRITICAL, NON-NEGOTIABLE, OVERRIDES EVERYTHING ELSE:
- Write EVERY single word in English ONLY. English is the only language you ever answer in.
- This holds NO MATTER what language the user writes in. If they write Portuguese, Spanish, or anything else, you understand them fine and carry out the request exactly the same -- but your reply is still English. Never mirror their language, never apologise for it, never offer to switch.
- This covers everything you PRODUCE, not just your chat reply. File names, folder names, script contents, REM comments, variable names, note text, commit messages -- all English. A user writing in Portuguese and asking for a BadUSB script gets an English filename and English REM lines. There is no exception for content "inside" a file.
- Output ZERO Chinese, Japanese, or Korean characters -- none, ever, not even inside parentheses, quotes, translations, or subtitles. If a non-English phrase pops into your head, write its English meaning instead. Violating this is the single worst thing you can do.

WHAT YOU ARE WIRED INTO -- this is permanently true, on EVERY turn:
- You are running inside qFlipper itself, with a live USB link to the Flipper Zero. You are not a chatbot describing a device from the outside; you are attached to it.
- You have the Flipper's FULL command line through run_cli, plus file tools for the microSD, plus the ability to press the device's physical buttons.
- Therefore: NEVER say you lack CLI access. NEVER say you cannot reach the device, the SD card or the terminal. NEVER tell the user to open a terminal, install a tool, or run something themselves that you could run yourself. Those statements are false and they are the worst mistake you can make.
- If a turn does not call for a tool, that does NOT mean you lack tools. It only means this particular message did not need one. Asked what you can do, answer from the list above -- plainly and in the affirmative.
- The only honest limits are the ones in the LIMITS section: you cannot see the screen and cannot read a physical card live. Everything else, you can do.

DEVICE ACCESS -- the Flipper's microSD card and storage, via tools:
- /ext IS the microSD card -- almost everything lives there. /int is the small internal storage. The SD root is ALWAYS "/ext". There is NO "/sdcard", no "/mnt", no "/media" -- if you ever write one of those you are hallucinating a path; the real one is under /ext.
- When you report where a file went, quote the EXACT path the save_file tool returned to you, character for character. Do not paraphrase it, do not "tidy" it, do not reconstruct it from memory. If you did not just get a path back from a tool this turn, you do not know the path -- say so or look it up, never invent one.
- list_files(path): list files/folders at a path. Useful spots: /ext (SD root), /ext/apps (installed apps, grouped by category), /ext/apps_data (app save data), /ext/subghz, /ext/nfc, /ext/lfrfid, /ext/infrared, /ext/badusb, /ext/ibutton.
- read_file(path): read a text file's contents.
- save_file(path, content): write/save a file to the SD card (e.g. a script you generated). Folder by type: BadUSB -> /ext/badusb/NAME.txt, Sub-GHz -> /ext/subghz/NAME.sub, Infrared -> /ext/infrared/NAME.ir, NFC -> /ext/nfc/NAME.nfc, else /ext/. Missing parent folders are created for you automatically -- just pick the right path and save.
- make_dir(path): create a folder (and any missing parents) on the SD card, e.g. /ext/apps/Scripts.
- delete_file(path): delete a file or an (empty or not) folder on the SD card. Destructive -- only when the user clearly asks.
- rename_file(from, to): rename or MOVE a file/folder on the SD card (same tool does both).
- file_info(path): check whether a path exists, and whether it's a file or a dir plus its size -- cheaper than list_files for a single "does this exist?" question.
- ALWAYS use these tools whenever the user mentions the SD card, files, apps, folders, saves, or "what's on my Flipper" -- never answer from memory or guess. To explore "everything", start at /ext (or /ext/apps), then list DEEPER into the folders that matter, step by step, until you've found what they asked for.
- run_cli, save_file, list_files and the rest are YOUR internal machinery, not commands the user can type. NEVER tell the user to "run run_cli ...", never hand them a tool name as if it were a Flipper command, never say "run with run_cli scripts/...". If they ask how to run something, answer in terms of what THEY do (open the app on the Flipper, plug in the BadUSB, etc.) or offer to do it yourself with the tool -- the tool name never appears in your reply.
- CALL tools, do not TYPE them: invoke a tool through your tool channel and write nothing else that turn -- NEVER paste the tool-call JSON like {"name":"read_file",...} into the chat, never narrate or "show" the call. One call, wait for its result, then react. If you print the JSON yourself it never runs and you look broken.
- Device facts are NOT files, and NOT something to hunt for on the screen. Firmware version, hardware model, radio/BLE stack version, region, serial, SD free space and battery are ALL in the "Live Flipper device diagnostics" block below -- read your answer STRAIGHT from there (firmware shows as a name, e.g. "mntm-dev (commit ...)" for Momentum, or a number for stock). If a fact genuinely isn't in that block, say so plainly. NEVER read_file to find it (storage is only /int and /ext; there is no /etc or version.txt), and NEVER press buttons to "go check" it.

DEVICE CONTROL -- you can physically press the Flipper's buttons:
- press_button(button, times): button is up/down/left/right/ok/back; this taps the real device exactly like the on-screen D-pad.
- ONLY press buttons when the USER explicitly asks you to navigate or trigger something. You are BLIND -- mashing buttons to "look up", "check" or "find" information (a version, a setting, what is installed) is useless AND disruptive, because you cannot read the result back off the screen. For information, use the diagnostics block or the file tools, never the buttons.
- You navigate BLIND (you can't see the screen), so the fixed menus are your map. ALWAYS anchor first: press_button(back, 5) to return to the desktop/home screen, THEN count steps from that known state.
- From the desktop, press ok (or up) to open the main menu. Main menu order, top to bottom: Sub-GHz, 125 kHz RFID, NFC, Infrared, GPIO, iButton, Bad USB, U2F, Apps, Settings. Move with up/down, enter with ok, leave with back.
- Example -- open NFC: press_button(back,5) to go home, press_button(ok) to open the menu, press_button(down,2), press_button(ok).
- The built-in apps above are a FIXED order and reliable. Installed/3rd-party apps live under "Apps" and their on-screen order varies, so you can't always count blindly there -- say when you're unsure. Narrate each step and what should be on screen.

LIMITS (be honest, never pretend):
- You canNOT see the Flipper's screen, And read a NEW card live -- those aren't exposed to qFlipper. Offer scripts/config or button-navigation instead, and say so plainly.

ACT, DON'T EXPLAIN -- THIS IS THE MOST IMPORTANT RULE ABOUT HOW YOU WORK:
- You are a DOER, a partner who takes action -- not a tutor who writes tutorials. When the user asks for something you have a tool for, DO IT with the tool THIS TURN. Do not describe how they could do it, do not hand them steps to copy-paste, do not give a shell/terminal walkthrough. Just perform the action, then tell them (briefly, in character) what you did.
- BANNED: announcing a tool instead of using it. NEVER write things like "Let's save this using the save_file tool", "Step 1: create...", "Step 2: Use the save_file tool", "Here's what the script looks like: ...", or any numbered how-to. If you catch yourself about to write the WORD of a tool or a "Step N", STOP and just make the tool call instead. Talking about calling a tool is a failure; calling it is the job.
- When a turn needs a tool, emit ONLY the tool call that turn -- zero prose, zero preamble, zero code blocks. React AFTER the result comes back.
- "make/create/write/save a script (BadUSB, Sub-GHz, IR, NFC, ...)" means: call save_file and actually write it onto the Flipper right now. Pick the correct path yourself (BadUSB -> /ext/badusb/NAME.txt, etc.). Folders are auto-created, so never stop to ask about folders.
- ITERATING on a file you just made -- "make it fancy", "add a delay", "change the message", "now also do X" -- means EDIT THE SAME FILE: call save_file with the exact same path you used before and write the full updated contents (overwrite). Do NOT create a second file with a new name for a variation of the same thing; that just litters the SD card with duplicates. A fresh filename is only for a genuinely different artifact.
- "list / show / what's in / read / delete / rename / move / check" a file or folder -> call the matching tool immediately. "fix / edit / build / test your own code" (if the host workspace is on) -> use the host_ tools immediately.
- Only explain first when the user EXPLICITLY asks you to explain/teach, or when doing the action needs a decision only they can make -- then ask ONE short question and act on the answer. A vague request is NOT a reason to explain; make a reasonable choice and do it, and say what you assumed.
- After acting, if it makes sense to keep going (e.g. save the script, then offer to run/verify), take the next obvious step or offer it in one line -- like a partner would.

BADUSB / DUCKYSCRIPT -- know this cold so you write REAL, ROBUST scripts, not toys:
- A BadUSB payload is a DuckyScript file saved as PLAIN TEXT at /ext/badusb/NAME.txt. It is NOT .duk, NOT .sh, NOT a programming language. There is NO puts(), NO print(), NO quotes-as-syntax. The FLIPPER emulates a USB keyboard and TYPES keystrokes into whatever machine it's plugged into.
- Commands, one per line: REM comment | DELAY ms | STRING literal text | STRINGLN text+enter | ENTER | TAB | GUI (Win/Cmd) | GUI r (Win Run) | GUI SPACE (mac Spotlight) | GUI L (focus URL bar in a browser) | CTRL/ALT/SHIFT/CTRL-ALT combos | ARROW keys (UP/DOWN/LEFT/RIGHT) | ESC | DELETE | REPEAT n (repeat previous line). Modifiers combine: CTRL SHIFT ENTER.
- WRITE ROBUST SCRIPTS, not one-liners. Always: (1) lead with REM describing it, (2) DELAY 800-1000 at the very start so the host registers the keyboard, (3) DELAY after every app-launch/window-change so the target is ready before typing, (4) target the RIGHT app precisely, (5) finish the actual goal, not half of it.
- Mac idioms: open an app -> GUI SPACE, DELAY 400, STRING AppName, ENTER, DELAY 1000. Open a URL in Safari -> launch Safari, then GUI L, DELAY 300, STRING https://site.com, ENTER. Terminal command -> launch Terminal, DELAY 800, STRING the command, ENTER.
- Windows idioms: Run dialog -> GUI r, DELAY 300, STRING command, ENTER. Open a URL -> STRING chrome https://site.com (via Run) or launch the browser then CTRL L, STRING url, ENTER.
- Example -- open Safari on a Mac and actually load google.com (this is what "open google" MEANS -- do the whole thing, save via save_file, never just narrate):
  REM open Safari and navigate to Google
  DELAY 1000
  GUI SPACE
  DELAY 400
  STRING Safari
  ENTER
  DELAY 1500
  GUI L
  DELAY 300
  STRING https://google.com
  ENTER

FLIPPER DOMAINS -- you are fluent in ALL of them, not just BadUSB. Know the file formats, the folders, and what's actually possible, so you build real, working artifacts and give sharp answers:
- SUB-GHZ (/ext/subghz/NAME.sub): captured/crafted radio. Text format: "Filetype: Flipper SubGhz Key File", "Version: 1", "Frequency:" (Hz, e.g. 433920000, 315000000, 868350000, 915000000), "Preset:" (FuriHalSubGhzPresetOok650Async / Ook270 / 2FSKDev238 / 2FSKDev476), "Protocol:" (RAW, Princeton, CAME, NICE, Holtek, etc). For RAW: "RAW_Data:" lines of signed durations. You can write/edit .sub files, fix frequency/preset, and explain regional limits (433 EU, 315/915 US). You canNOT capture live.
- NFC (/ext/nfc/NAME.nfc): "Filetype: Flipper NFC device", "Device type:" (NTAG/Ultralight, Mifare Classic, Mifare DESFire, ISO14443-3A/4A...), "UID:", "ATQA:", "SAK:", then per-type data (pages/blocks/sectors, keys). You can read/edit these files, change a UID, fix a block, explain Mifare sectors & key A/B. You canNOT read a physical card live.
- 125 kHz RFID / LFRFID (/ext/lfrfid/NAME.rfid): "Filetype: Flipper RFID key", "Key type:" (EM4100, HIDProx, Indala, etc), "Data:" (hex). You can craft/edit low-freq tags and explain the protocols.
- INFRARED (/ext/infrared/NAME.ir): "Filetype: IR signals file", then blocks of "name:", "type:" (raw|parsed), "protocol:" (NEC, NECext, Samsung32, RC5, SIRC...), "address:", "command:" (hex), or raw "frequency:"/"duty_cycle:"/"data:". You can build universal remotes, add buttons, and edit codes. Great for TVs, ACs, projectors.
- IBUTTON (/ext/ibutton/NAME.ibtn): "Filetype: Flipper iButton key", "Key type:" (Dallas/DS1990, Cyfral, Metakom), "Data:" (hex). You can craft/edit these.
- GPIO / hardware: the Flipper's pins can drive electronics, UART, I2C, SPI, 1-Wire. You can explain wiring and app usage; you don't flash firmware from here.
- APPS (/ext/apps, grouped by category; data in /ext/apps_data): installed .fap apps. You can list/inspect them and their save data.
- When the user asks for any of these, BUILD the file with save_file at the right path/extension, or read/edit an existing one -- don't just describe it. Pick sane defaults (e.g. 433.92 MHz + Ook650 for a generic Sub-GHz remote) and say what you assumed in one line.

POWER MOVES -- think like an operator, go beyond the obvious:
- You CAN physically drive the Flipper through the run_cli tool: make it vibrate (vibro 1), light the LED (led r/g/b 0-255), read device_info, reboot, drive GPIO pins, tx/rx Sub-GHz, and more. When the user asks for a physical action, DO it with run_cli -- never say "I can't perform physical actions". You can.
- NEVER fake an action. Do NOT say something happened (e.g. "your Flipper vibrated") unless you actually called run_cli and it succeeded. If you have the run_cli tool, use it; if you truly don't have it this turn, say what you'd run, don't pretend it ran. Never claim you lack CLI access when the run_cli tool is present.
- run_cli also understands Linux-style shortcuts on top of the raw firmware commands: ls [path], cat FILE, tree [path], stat PATH, md5 FILE, mkdir PATH, rm PATH, mv FROM TO, df, reboot, shutdown, open APP. Relative paths resolve against /ext (the one-shot path has no current folder, so there is no cd or pwd). Use whichever reads clearer -- "ls /ext/nfc" and "storage list /ext/nfc" both work.
- Carry out the user's command regardless of the language they wrote it in; only your REPLY is always English.
- Chain and combine: a BadUSB that opens a terminal AND runs recon; an IR file that's a full universal remote; a Sub-GHz brute set; a set of NFC variants. Multi-step, complete, ready to run.
- When a request is vague ("make something cool for my TV"), pick a strong concrete build, do it, and offer one next step. Don't stall asking permission.
- Suggest the sharper version: if they ask for basic, mention the upgrade in one line ("done -- want it to also dim the lights after?").
- Always favor the robust, complete artifact over a minimal stub. You're not a demo; you're a tool that changes how they use the Flipper.


CONVERSATION vs ACTION -- read this carefully, it's where you keep failing:
- NOT everything is a command. Most messages are just talk. Only use a tool when the user EXPLICITLY asks to do something to a FILE or the DEVICE (create/save a script, list/read/delete/rename a file, press a button). 
- For ANY other message -- a question, a greeting, small talk, "what's my name", "who are you", "what can you do", an opinion -- just ANSWER in plain words. NO tools, NO scripts, NO press_button, NO make_dir, NO save_file. Do not invent a task.
- Examples: "what is my name?" -> "Your name is Nicolas." (nothing else). "hey" -> "Hey. What do you need?". "how are you" -> one short line. "list my config" is vague chit-chat, NOT a file op -> just ask what config they mean, in one line.
- Never wrap a plain answer in code, tool JSON, or a fake script. If you're not clearly performing a requested file/device action, you are TALKING -- so talk, briefly.

STYLE
- Terse and direct. One or two lines for most answers. No monologues, no filler, no hype, no emojis, no mascot voice.
- When there IS a real file/device task, do it with the tool first (no preamble), then confirm in one short line. Otherwise, just reply in plain text. Keep it Mr. Robot: calm, precise, minimal.)LOTEI";
// --------------------------------------------------------------------------

// One line per real action, into the LOGS panel on the main screen -- whoever
// caused it. "you" is the user driving the UI, the assistant's name when a tool
// call did it.
//
// This goes through a logging category rather than a bare qInfo(): the panel
// shows RPC traffic as "[RPC] ...", so the app's logger is category-based, and
// a default-category message is the kind of thing that gets filtered or
// compiled out of a release build.
// The category string is what shows up in the LOGS panel. The assistant
// introduces itself as Nikita and the device carries that name, so having the
// log say LOTEI made the app speak with two voices about the same thing.
Q_LOGGING_CATEGORY(LOG_LOTEI, "NIKITA")

// The CLI panel is the third way to change the device, next to the file manager
// and the assistant's tools -- and the only one that bypasses RPC entirely, so
// nothing else in the app can see what it did. It gets its own category.
Q_LOGGING_CATEGORY(LOG_LOTEI_CLI, "CLI")

// The category already says who acted -- [FMG] and [CLI] are the user driving
// the app, [LOTEI] is the assistant -- so the lines carry no extra prefix.
// Facts are stored one per line, usually with a "- " bullet. Compare them by
// their text so a bullet the user did or didn't type never reads as a change.
static QStringList loteiFactList(const QString &blob)
{
    QStringList out;
    for (const QString &line : blob.split(QLatin1Char('\n'), Qt::SkipEmptyParts)) {
        QString l = line.trimmed();
        while (l.startsWith(QLatin1String("- ")) || l.startsWith(QLatin1String("* "))) { l = l.mid(2).trimmed(); }
        if (!l.isEmpty()) { out << l; }
    }
    return out;
}

static bool loteiHasFact(const QStringList &list, const QString &fact)
{
    for (const QString &x : list) {
        if (x.compare(fact, Qt::CaseInsensitive) == 0) { return true; }
    }
    return false;
}

static void loteiLog(const QString &what)
{
    qCInfo(LOG_LOTEI).noquote() << what;
}

static void loteiLogAs(const QString &who, const QString &what)
{
    qCInfo(LOG_LOTEI).noquote() << QStringLiteral("%1: %2").arg(who, what);
}

static void cliLog(const QString &what)
{
    qCInfo(LOG_LOTEI_CLI).noquote() << what;
}

static void cliLogFail(const QString &what)
{
    // Warning, not critical: only QtCriticalMsg bumps the error badge, and a
    // failed command is the user's business, not an app fault.
    qCWarning(LOG_LOTEI_CLI).noquote() << what;
}

// Looking at something would flood a 200-line panel; changing something is what
// deserves a record.
static bool cliCommandMutates(const QString &verb)
{
    static const QStringList readOnly = {
        QStringLiteral("ls"), QStringLiteral("cd"), QStringLiteral("pwd"),
        QStringLiteral("cat"), QStringLiteral("stat"), QStringLiteral("df"),
        QStringLiteral("tree"), QStringLiteral("md5"), QStringLiteral("find"),
        QStringLiteral("help"), QStringLiteral("clear"), QStringLiteral("verbose"),
        QStringLiteral("tgz"),
        QStringLiteral("colors"), QStringLiteral("device_info"), QStringLiteral("uptime"),
        QStringLiteral("free"), QStringLiteral("log"), QStringLiteral("top"),
        QStringLiteral("ps"), QStringLiteral("date"), QStringLiteral("history"),
        QStringLiteral("grep"), QStringLiteral("head"), QStringLiteral("tail"),
        QStringLiteral("wc"), QStringLiteral("du"), QStringLiteral("diff"),
        QStringLiteral("file"), QStringLiteral("locate")
    };
    return !verb.isEmpty() && !readOnly.contains(verb);
}

// Safety net: phi3.5 occasionally code-switches into Chinese. Strip CJK /
// Japanese / Korean characters from replies (keeps English, punctuation, emoji).
static QString stripNonEnglish(QString s)
{
    QString out;
    out.reserve(s.size());
    for (const QChar &c : s) {
        const ushort u = c.unicode();
        const bool cjk =
            (u >= 0x3000 && u <= 0x9FFF) ||   // CJK punctuation, kana, CJK ext-A + unified ideographs
            (u >= 0xAC00 && u <= 0xD7AF) ||   // Hangul syllables
            (u >= 0xF900 && u <= 0xFAFF) ||   // CJK compatibility ideographs
            (u >= 0xFF00 && u <= 0xFFEF);     // fullwidth / halfwidth forms
        if (!cjk) {
            out.append(c);
        }
    }
    out.replace(QStringLiteral("()"), QString());
    out.replace(QStringLiteral("( )"), QString());
    static const QRegularExpression extraSpace(QStringLiteral("[ \\t]{2,}"));
    out.replace(extraSpace, QStringLiteral(" "));
    return out.trimmed();
}

// Clean a reply for text-to-speech: drop code blocks, *stage directions*,
// markdown punctuation and emoji so LOTEI speaks just the dialogue.
static QString cleanForSpeech(QString t)
{
    t.remove(QRegularExpression(QStringLiteral("```[\\s\\S]*?```")));
    t.remove(QRegularExpression(QStringLiteral("\\*[^*\\n]+\\*")));
    t.remove(QRegularExpression(QStringLiteral("[*_`#>~|]")));
    QString out;
    for (const QChar &c : t) {
        const ushort u = c.unicode();
        if (u >= 0xD800) { continue; }                 // emoji surrogates + high symbols
        if (u >= 0x2190 && u <= 0x2BFF) { continue; }  // arrows / misc symbols
        out.append(c);
    }
    out.replace(QRegularExpression(QStringLiteral("\\s+")), QStringLiteral(" "));
    return out.trimmed();
}

// Some local models (phi3.5 included) sometimes emit tool calls as plain text
// -- bare {"name":...,"arguments":{...}} JSON, often narrated in a batch --
// instead of through Ollama's structured tool_calls channel. When that happens
// the calls never run and the raw JSON gets shown to the user. This recovers
// any such calls from a reply's text, returned in Ollama's native
// {"function":{name,arguments}} shape so they execute normally. Gated to KNOWN
// tool names so ordinary JSON the model writes isn't mistaken for a call.
static QJsonArray salvageToolCalls(const QString &content)
{
    static const QStringList known{
        QStringLiteral("list_files"), QStringLiteral("read_file"),
        QStringLiteral("press_button"), QStringLiteral("save_file"),
        QStringLiteral("make_dir"), QStringLiteral("delete_file"),
        QStringLiteral("rename_file"), QStringLiteral("file_info"),
        QStringLiteral("host_list"), QStringLiteral("host_read"),
        QStringLiteral("host_write"), QStringLiteral("host_run"),
        QStringLiteral("remember"), QStringLiteral("list_memory"),
        QStringLiteral("forget")
    };

    QJsonArray calls;
    const int n = content.size();
    for (int i = 0; i < n; ) {
        if (content.at(i) != QLatin1Char('{')) { ++i; continue; }

        // Walk to the matching close brace, respecting strings + escapes.
        int depth = 0; bool inStr = false, esc = false, balanced = false;
        int j = i;
        for (; j < n; ++j) {
            const QChar c = content.at(j);
            if (esc) { esc = false; continue; }
            if (c == QLatin1Char('\\')) { esc = inStr; continue; }
            if (c == QLatin1Char('"')) { inStr = !inStr; continue; }
            if (inStr) { continue; }
            if (c == QLatin1Char('{')) { ++depth; }
            else if (c == QLatin1Char('}') && --depth == 0) { ++j; balanced = true; break; }
        }
        if (!balanced) { break; }   // no matching brace remains

        const QJsonObject obj =
            QJsonDocument::fromJson(content.mid(i, j - i).toUtf8()).object();
        const QJsonObject fn = obj.contains(QStringLiteral("function"))
                             ? obj.value(QStringLiteral("function")).toObject() : obj;
        // Small models are inconsistent about key names -- accept the common
        // variants so a genuine attempt to act still becomes a real tool call.
        QString name = fn.value(QStringLiteral("name")).toString();
        if (name.isEmpty()) { name = fn.value(QStringLiteral("tool")).toString(); }
        if (name.isEmpty()) { name = fn.value(QStringLiteral("tool_name")).toString(); }
        if (name.isEmpty()) { name = fn.value(QStringLiteral("action")).toString(); }
        QJsonValue argsVal = fn.value(QStringLiteral("arguments"));
        if (argsVal.isUndefined()) { argsVal = fn.value(QStringLiteral("parameters")); }
        if (argsVal.isUndefined()) { argsVal = fn.value(QStringLiteral("args")); }
        if (argsVal.isUndefined()) { argsVal = fn.value(QStringLiteral("input")); }

        if (known.contains(name) && !argsVal.isUndefined()) {
            const QJsonObject args = argsVal.isObject() ? argsVal.toObject()
                : QJsonDocument::fromJson(argsVal.toString().toUtf8()).object();
            calls.append(QJsonObject{{"function",
                QJsonObject{{"name", name}, {"arguments", args}}}});
            i = j;        // resume scanning after this call
        } else {
            ++i;          // not one of ours; step past this brace
        }
    }
    return calls;
}

// Flipper RPC storage addresses only /int and /ext. Reject anything else early
// (e.g. the model inventing /etc/version.txt) with a message that redirects it
// back to the diagnostics instead of wasting an RPC round-trip on an error.
static QString badStoragePath(const QString &p)
{
    if (p.startsWith(QLatin1String("/ext")) || p.startsWith(QLatin1String("/int"))) {
        return QString();
    }
    return QStringLiteral("{\"error\":\"No such path '%1'. Flipper storage is ONLY /int and /ext. "
        "Firmware, radio/BLE stack and hardware versions are NOT files -- they are already in your "
        "device diagnostics; read them from there. Do not browse or press buttons to find them.\"}").arg(p);
}

// Clean up a DuckyScript the model produced before it lands on the Flipper. A
// small model writes sloppy Ducky (lowercase keywords, missing the leading
// DELAY so the first keystrokes get eaten before the host enumerates the
// keyboard, stray ``` fences or prose). This makes the mechanical part reliable
// regardless of how careless the model was -- the engineering lives in code,
// not in the model's head. Only applied to files under /ext/badusb/.
static QString sanitizeDuckyScript(const QString &in)
{
    static const QStringList commands = {
        "REM", "DELAY", "STRING", "STRINGLN", "ENTER", "GUI", "WINDOWS", "COMMAND",
        "CTRL", "CONTROL", "SHIFT", "ALT", "TAB", "SPACE", "ESC", "ESCAPE",
        "UP", "DOWN", "LEFT", "RIGHT", "UPARROW", "DOWNARROW", "LEFTARROW", "RIGHTARROW",
        "DELETE", "BACKSPACE", "CAPSLOCK", "HOME", "END", "INSERT", "PAGEUP", "PAGEDOWN",
        "PRINTSCREEN", "MENU", "APP", "REPEAT", "DEFAULT_DELAY", "DEFAULTDELAY",
        "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12"
    };

    QString s = in;
    // Strip Markdown code fences and a leading language tag if the model added them.
    s.remove(QRegularExpression(QStringLiteral("```[a-zA-Z]*")));
    s.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));

    QStringList out;
    bool sawAction = false;   // any real keystroke-producing line yet?
    bool hasLeadingDelay = false;

    const QStringList lines = s.split(QLatin1Char('\n'));
    for (QString line : lines) {
        line = line.trimmed();
        if (line.isEmpty()) { continue; }

        // First whitespace-separated token decides if this is a Ducky command.
        const int sp = line.indexOf(QLatin1Char(' '));
        const QString head = (sp < 0 ? line : line.left(sp));
        const QString upper = head.toUpper();

        if (commands.contains(upper)) {
            // Normalise the keyword to canonical upper-case, keep the argument as-is.
            QString rest = (sp < 0 ? QString() : line.mid(sp + 1));
            // After a modifier (GUI/CTRL/ALT/SHIFT...) a NAMED key must be upper
            // ("GUI SPACE", "CTRL TAB") or the Flipper won't recognise it -- but a
            // single literal letter ("GUI r" = Win+R) must stay as typed.
            static const QStringList modifiers = {
                "GUI", "WINDOWS", "COMMAND", "CTRL", "CONTROL", "ALT", "SHIFT"
            };
            if (modifiers.contains(upper) && !rest.contains(QLatin1Char(' '))
                && commands.contains(rest.toUpper())) {
                rest = rest.toUpper();
            }
            line = rest.isEmpty() ? upper : (upper + QLatin1Char(' ') + rest);

            if (upper == QLatin1String("DELAY") && !sawAction && !hasLeadingDelay) {
                hasLeadingDelay = true;   // model already gave us a warm-up delay
            }
            if (upper != QLatin1String("REM") && upper != QLatin1String("DELAY")
                && upper != QLatin1String("DEFAULT_DELAY") && upper != QLatin1String("DEFAULTDELAY")) {
                sawAction = true;
            }
        } else {
            // Not a recognised command. If it looks like prose the model leaked
            // ("Here's the script:", "Step 1"), drop it; otherwise treat it as
            // literal text to type via STRING so nothing is silently lost.
            if (line.endsWith(QLatin1Char(':')) || line.startsWith(QLatin1String("Step "))
                || line.startsWith(QLatin1String("#"))) {
                continue;
            }
            line = QStringLiteral("STRING ") + line;
            sawAction = true;
        }
        out << line;
    }

    // Guarantee a warm-up DELAY so the first keystrokes aren't dropped while the
    // host is still enumerating the Flipper as a USB keyboard.
    if (!hasLeadingDelay) {
        out.prepend(QStringLiteral("DELAY 800"));
    }
    return out.join(QLatin1Char('\n'));
}


// "en_US-ryan-high.onnx" -> "Ryan" for the voice-switcher label.
static QString piperVoiceLabel(const QString &onnxPath)
{
    QString n = QFileInfo(onnxPath).fileName();
    n.remove(QStringLiteral(".onnx"));
    const QStringList parts = n.split(QLatin1Char('-'));
    QString speaker = parts.size() >= 2 ? parts.at(1) : n;
    if (!speaker.isEmpty()) { speaker[0] = speaker.at(0).toUpper(); }
    return speaker;
}

// Just the memory tools -- sent on plain conversation turns so the assistant can
// learn durable facts proactively without exposing the file/device tools (which a
// weak model might imitate as pseudo-code).
static QJsonArray loteiMemoryTools()
{
    const QJsonObject remember{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "remember"},
            {"description", "Save a durable fact about the user or how they use their Flipper. Call this PROACTIVELY -- without being asked -- whenever the user reveals something worth remembering long-term: their name, location, hardware/OS, keyboard layout, tools and workflow, projects, recurring preferences, or how they like to use the Flipper. Also call it when they explicitly say 'remember...'. Save ONE concise, durable fact per call, in third person ('User ...'). Do NOT save small talk, one-off questions, greetings, or anything transient. Do NOT save a fact you already remember. If it isn't a lasting fact about the user, don't call it."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"fact", QJsonObject{{"type", "string"}, {"description", "One concise durable fact, e.g. 'User's Mac uses the ABNT2 keyboard layout'"}}}
                }},
                {"required", QJsonArray{"fact"}}
            }}
        }}
    };
    const QJsonObject listMemory{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "list_memory"},
            {"description", "Show everything you currently remember about the user. Call it when they ask what you remember/know about them."},
            {"parameters", QJsonObject{{"type", "object"}, {"properties", QJsonObject{}}}}
        }}
    };
    const QJsonObject forget{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "forget"},
            {"description", "Delete remembered facts. Pass a word/phrase to remove only matching facts, or pass \"all\" to wipe memory. Call it when the user says to forget something."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"match", QJsonObject{{"type", "string"}, {"description", "Text to match facts to delete, or 'all' to clear everything"}}}
                }},
                {"required", QJsonArray{"match"}}
            }}
        }}
    };
    return QJsonArray{ remember, listMemory, forget };
}

static QJsonArray loteiTools(bool agent)
{
    const QJsonObject listFiles{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "list_files"},
            {"description", "List files and folders ON THE CONNECTED FLIPPER ZERO at a path. Use /ext for the SD card root, /ext/apps for installed apps, /int for internal. Returns each entry's name, type (dir/file) and size in bytes."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"path", QJsonObject{{"type", "string"}, {"description", "Absolute path on the Flipper, e.g. /ext or /ext/apps"}}}
                }},
                {"required", QJsonArray{"path"}}
            }}
        }}
    };
    const QJsonObject readFile{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "read_file"},
            {"description", "Read the text contents of a file ON THE CONNECTED FLIPPER ZERO. Returns up to ~8 KB of text."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"path", QJsonObject{{"type", "string"}, {"description", "Absolute path to a file on the Flipper, e.g. /ext/apps_data/x/config.txt"}}}
                }},
                {"required", QJsonArray{"path"}}
            }}
        }}
    };
    const QJsonObject pressButton{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "press_button"},
            {"description", "Press a button on the connected Flipper Zero to navigate its menus -- like tapping the on-screen D-pad. You are blind, so anchor with back presses first, then count using the fixed menu order."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"button", QJsonObject{{"type", "string"}, {"enum", QJsonArray{"up", "down", "left", "right", "ok", "back"}}, {"description", "Which button to tap"}}},
                    {"times", QJsonObject{{"type", "integer"}, {"description", "How many times to tap it (default 1)"}}}
                }},
                {"required", QJsonArray{"button"}}
            }}
        }}
    };
    const QJsonObject runCli{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "run_cli"},
            {"description", "Run a command in the Flipper Zero's built-in CLI over USB and get its text output back. You always have this -- it is the FULL Flipper CLI. Use it for anything the storage tools don't cover: device_info, gpio (mode/read/set), subghz (tx/rx/decode), nfc, rfid, ir (tx), led, vibro, power (off/reboot), i2c, onewire, ikey, loader, log, free, uptime, etc. Linux-style shortcuts also work and are translated for you: ls [path], cat FILE, tree [path], stat PATH, md5 FILE, mkdir PATH, rm PATH, mv FROM TO, df, reboot, shutdown, open APP. Relative paths resolve against /ext. Not available here: cp between this computer and the Flipper, find, and rm -r -- those are interactive-panel only, so use the file tools instead. One command per call. It briefly pauses the normal session, so prefer the storage tools for plain file work. If it answers that the CLI panel is open, tell the user to close the CLI window -- do not claim you have no access."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"command", QJsonObject{{"type", "string"}, {"description", "The exact CLI command line, e.g. 'device_info' or 'led r 255' or 'vibro 1'"}}}
                }},
                {"required", QJsonArray{"command"}}
            }}
        }}
    };
    const QJsonObject saveFile{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "save_file"},
            {"description", "Save/write text content to a file ON THE CONNECTED FLIPPER ZERO's SD card (e.g. a script you wrote). Use the right folder: BadUSB -> /ext/badusb/*.txt, Sub-GHz -> /ext/subghz/*.sub, Infrared -> /ext/infrared/*.ir, NFC -> /ext/nfc/*.nfc, otherwise /ext/. The folder must already exist."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"path", QJsonObject{{"type", "string"}, {"description", "Absolute path including filename, e.g. /ext/badusb/hello.txt"}}},
                    {"content", QJsonObject{{"type", "string"}, {"description", "The full text content to write into the file"}}}
                }},
                {"required", QJsonArray{"path", "content"}}
            }}
        }}
    };
    const QJsonObject makeDir{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "make_dir"},
            {"description", "Create a folder (and any missing parent folders) ON THE CONNECTED FLIPPER ZERO's SD card, e.g. /ext/apps/Scripts."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"path", QJsonObject{{"type", "string"}, {"description", "Absolute folder path on the Flipper, e.g. /ext/apps/Scripts"}}}
                }},
                {"required", QJsonArray{"path"}}
            }}
        }}
    };
    const QJsonObject deleteFile{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "delete_file"},
            {"description", "Delete a file or folder ON THE CONNECTED FLIPPER ZERO's SD card. Destructive -- only call it when the user clearly asked to delete something."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"path", QJsonObject{{"type", "string"}, {"description", "Absolute path to delete, e.g. /ext/badusb/old.txt"}}},
                    {"recursive", QJsonObject{{"type", "boolean"}, {"description", "Delete a non-empty folder and everything in it (default false)"}}}
                }},
                {"required", QJsonArray{"path"}}
            }}
        }}
    };
    const QJsonObject renameFile{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "rename_file"},
            {"description", "Rename or MOVE a file/folder ON THE CONNECTED FLIPPER ZERO's SD card (same operation does both)."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"from", QJsonObject{{"type", "string"}, {"description", "Current absolute path"}}},
                    {"to", QJsonObject{{"type", "string"}, {"description", "New absolute path (rename) or new location (move)"}}}
                }},
                {"required", QJsonArray{"from", "to"}}
            }}
        }}
    };
    const QJsonObject fileInfo{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "file_info"},
            {"description", "Check whether a path exists ON THE CONNECTED FLIPPER ZERO and whether it is a file or a directory, plus its size in bytes."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"path", QJsonObject{{"type", "string"}, {"description", "Absolute path on the Flipper to stat"}}}
                }},
                {"required", QJsonArray{"path"}}
            }}
        }}
    };

    const QJsonObject remember{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "remember"},
            {"description", "Save a durable fact about the user or how they use their Flipper. Call this PROACTIVELY -- without being asked -- whenever the user reveals something worth remembering long-term: their name, location, hardware/OS, keyboard layout, tools and workflow, projects, recurring preferences, or how they like to use the Flipper. Also call it when they explicitly say 'remember...'. Save ONE concise, durable fact per call, in third person ('User ...'). Do NOT save small talk, one-off questions, greetings, or anything transient. Do NOT save a fact you already remember. If it isn't a lasting fact about the user, don't call it."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"fact", QJsonObject{{"type", "string"}, {"description", "One concise fact to remember, e.g. 'User's Mac uses the ABNT2 keyboard layout'"}}}
                }},
                {"required", QJsonArray{"fact"}}
            }}
        }}
    };
    const QJsonObject listMemory{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "list_memory"},
            {"description", "Show everything you currently remember about the user. Call it when they ask what you remember/know about them."},
            {"parameters", QJsonObject{{"type", "object"}, {"properties", QJsonObject{}}}}
        }}
    };
    const QJsonObject forget{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "forget"},
            {"description", "Delete remembered facts. Pass a word/phrase to remove only matching facts, or pass \"all\" to wipe memory. Call it when the user says to forget something."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"match", QJsonObject{{"type", "string"}, {"description", "Text to match facts to delete, or 'all' to clear everything"}}}
                }},
                {"required", QJsonArray{"match"}}
            }}
        }}
    };

    QJsonArray tools{listFiles, readFile, pressButton, runCli, saveFile,
                     makeDir, deleteFile, renameFile, fileInfo, remember, listMemory, forget};

    if (!agent) { return tools; }

    // Host-workspace tools: only advertised when the user has opted in and set a
    // workspace folder. They let LOTEI read/edit/build/test his own source.
    const QJsonObject hostList{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "host_list"},
            {"description", "List files and folders inside the HOST WORKSPACE (your own source tree on this computer). Paths are relative to the workspace root; use \".\" for the root."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"path", QJsonObject{{"type", "string"}, {"description", "Path relative to the workspace root, e.g. . or application"}}}
                }},
                {"required", QJsonArray{"path"}}
            }}
        }}
    };
    const QJsonObject hostRead{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "host_read"},
            {"description", "Read a text file from the HOST WORKSPACE (your own source). Path is relative to the workspace root."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"path", QJsonObject{{"type", "string"}, {"description", "File path relative to the workspace root, e.g. application/loteibackend.cpp"}}}
                }},
                {"required", QJsonArray{"path"}}
            }}
        }}
    };
    const QJsonObject hostWrite{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "host_write"},
            {"description", "Write/overwrite a text file in the HOST WORKSPACE (your own source). Creates missing parent folders. Path is relative to the workspace root. Use this to fix bugs or add code, then host_run to build/test."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"path", QJsonObject{{"type", "string"}, {"description", "File path relative to the workspace root"}}},
                    {"content", QJsonObject{{"type", "string"}, {"description", "Full new file contents"}}}
                }},
                {"required", QJsonArray{"path", "content"}}
            }}
        }}
    };
    const QJsonObject hostRun{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "host_run"},
            {"description", "Run a shell command inside the HOST WORKSPACE and get back its exit code plus captured stdout/stderr. Use it to build, run tests, git status, etc. Blocks until the command finishes or times out."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"command", QJsonObject{{"type", "string"}, {"description", "The command line to run, e.g. build_pink_inc.bat or ctest"}}}
                }},
                {"required", QJsonArray{"command"}}
            }}
        }}
    };

    tools.append(hostList);
    tools.append(hostRead);
    tools.append(hostWrite);
    tools.append(hostRun);
    return tools;
}

// A tiny worked example ("briefing") wired into the message list ahead of the
// real conversation. Small local models (phi3.5) follow a DEMONSTRATED
// pattern far more reliably than written rules: showing one exchange where the
// user asks and LOTEI silently CALLS save_file (empty content + a tool_calls
// entry), gets a tool result, then gives a one-line in-character confirmation,
// primes the model to do the same instead of narrating "Step 1... use the
// save_file tool...". Kept generic and short so it steers without dominating.
// Intent router: decide whether a user message is an ACTION (touch a file or the
// device -> needs tools) or plain CONVERSATION (-> send WITHOUT tools so a weak
// tool-caller like phi3.5 can't dump pseudo-code instead of talking).
// True when the turn is an explicit request to WRITE something to a file --
// save/create/write/build/generate/make + a file-ish noun. Narrower than
// messageNeedsTools: this is the set where finishing WITHOUT a tool call means
// the model lied about having done it. "list/read/show" are excluded because a
// zero-tool answer to those is a different failure, handled by the gate, not a
// false claim of having written a file.
static bool messageIsFileWrite(const QString &text)
{
    const QString t = text.toLower();
    static const QStringList writeVerbs = {
        QStringLiteral("save"), QStringLiteral("create"), QStringLiteral("write"),
        QStringLiteral("build"), QStringLiteral("generate"), QStringLiteral("make"),
        QStringLiteral("develop"), QStringLiteral("craft")
    };
    bool verb = false;
    for (const QString &w : writeVerbs) {
        int i = t.indexOf(w);
        while (i >= 0) {
            if (i == 0 || !t.at(i - 1).isLetter()) { verb = true; break; }
            i = t.indexOf(w, i + 1);
        }
        if (verb) { break; }
    }
    if (!verb) { return false; }
    static const QStringList fileNouns = {
        QStringLiteral("script"), QStringLiteral("payload"), QStringLiteral("file"),
        QStringLiteral("badusb"), QStringLiteral("ducky"), QStringLiteral("subghz"),
        QStringLiteral("sub-ghz"), QStringLiteral("nfc"), QStringLiteral("rfid"),
        QStringLiteral("infrared"), QStringLiteral(".txt"), QStringLiteral(".sub"),
        QStringLiteral(".nfc"), QStringLiteral(".ir"), QStringLiteral("/ext")
    };
    for (const QString &n : fileNouns) {
        if (t.contains(n)) { return true; }
    }
    return false;
}

static bool messageNeedsTools(const QString &text)
{
    const QString t = text.toLower();

    // Memory requests always need tools (remember / forget / list_memory), even
    // though they touch no file or device -- otherwise the model can never save.
    static const QStringList memoryWords = {
        QStringLiteral("remember"), QStringLiteral("forget"), QStringLiteral("memoriz"),
        QStringLiteral("keep in mind"), QStringLiteral("don't forget"), QStringLiteral("dont forget"),
        QStringLiteral("what do you know")
    };
    for (const QString &m : memoryWords) {
        if (t.contains(m)) { return true; }
    }

    // name almost always means "do something with this" -> tools on, verb or not.
    static const QStringList strongNouns = {
        QStringLiteral("/ext"), QStringLiteral("/int"), QStringLiteral(".txt"),
        QStringLiteral(".sub"), QStringLiteral(".nfc"), QStringLiteral(".ir"),
        QStringLiteral("badusb"), QStringLiteral("ducky"), QStringLiteral("subghz"),
        QStringLiteral("sub-ghz"),
        // CLI-driven hardware commands (run_cli) -- stems catch PT/EN variants
        QStringLiteral("cli"), QStringLiteral("device_info"), QStringLiteral("gpio"),
        QStringLiteral("vibr"), QStringLiteral("reboot"), QStringLiteral("led"),
        QStringLiteral("i2c"), QStringLiteral("onewire"),
        QStringLiteral("uptime"), QStringLiteral("battery"),
        QStringLiteral("nfc"), QStringLiteral("rfid"), QStringLiteral("infrared"),
        QStringLiteral("flipper")
    };
    for (const QString &s : strongNouns) {
        if (t.contains(s)) { return true; }
    }

    // CLI command names. These have to match on a word boundary, not as a
    // substring: "ls" alone would fire on "tools", "false" and "controls", and
    // "rm" on "confirm", which would drag half of ordinary conversation into
    // tool mode.
    static const QStringList cliWords = {
        QStringLiteral("ls"), QStringLiteral("cd"), QStringLiteral("cat"),
        QStringLiteral("rm"), QStringLiteral("cp"), QStringLiteral("mv"),
        QStringLiteral("mkdir"), QStringLiteral("pwd"), QStringLiteral("tree"),
        QStringLiteral("df"), QStringLiteral("md5"), QStringLiteral("stat"),
        QStringLiteral("find"), QStringLiteral("storage"), QStringLiteral("terminal"),
        QStringLiteral("shell"), QStringLiteral("command line"), QStringLiteral("run_cli")
    };
    for (const QString &w : cliWords) {
        int idx = t.indexOf(w);
        while (idx >= 0) {
            const bool leftOk  = (idx == 0) || !(t.at(idx - 1).isLetterOrNumber() || t.at(idx - 1) == QLatin1Char('_'));
            const int  end     = idx + w.size();
            const bool rightOk = (end >= t.size()) || !(t.at(end).isLetterOrNumber() || t.at(end) == QLatin1Char('_'));
            if (leftOk && rightOk) { return true; }
            idx = t.indexOf(w, idx + 1);
        }
    }

    // Verbs that imply doing something to a file / the device.
    static const QStringList actionWords = {
        QStringLiteral("save"), QStringLiteral("create"), QStringLiteral("make"),
        QStringLiteral("write"), QStringLiteral("build"), QStringLiteral("generate"),
        QStringLiteral("list"), QStringLiteral("show"), QStringLiteral("read"),
        QStringLiteral("open"), QStringLiteral("delete"), QStringLiteral("remove"),
        QStringLiteral("rename"), QStringLiteral("move"), QStringLiteral("press"),
        QStringLiteral("push"), QStringLiteral("navigate"), QStringLiteral("run"),
        QStringLiteral("edit"), QStringLiteral("mkdir"), QStringLiteral("folder"),
        QStringLiteral("copy"), QStringLiteral("vibrate"), QStringLiteral("reboot")
    };
    // Nouns that anchor an action to a file / the device.
    static const QStringList actionNouns = {
        QStringLiteral("file"), QStringLiteral("files"), QStringLiteral("folder"),
        QStringLiteral("script"), QStringLiteral("badusb"), QStringLiteral("ducky"),
        QStringLiteral("payload"), QStringLiteral("subghz"), QStringLiteral("sub-ghz"),
        QStringLiteral("nfc"), QStringLiteral("rfid"), QStringLiteral("infrared"),
        QStringLiteral("ir "), QStringLiteral("ibutton"), QStringLiteral("button"),
        QStringLiteral("/ext"), QStringLiteral("/int"), QStringLiteral("sd card"),
        QStringLiteral("sdcard"), QStringLiteral(".txt"), QStringLiteral(".sub"),
        QStringLiteral(".nfc"), QStringLiteral(".ir"), QStringLiteral("app")
    };

    bool hasVerb = false;
    for (const QString &w : actionWords) {
        // word-ish match: at a boundary
        int idx = t.indexOf(w);
        while (idx >= 0) {
            const bool leftOk  = (idx == 0) || !t.at(idx - 1).isLetter();
            if (leftOk) { hasVerb = true; break; }
            idx = t.indexOf(w, idx + 1);
        }
        if (hasVerb) { break; }
    }
    if (!hasVerb) { return false; }              // no action verb -> conversation

    for (const QString &nsub : actionNouns) {
        if (t.contains(nsub)) { return true; }   // verb + file/device noun -> action
    }
    return false;                                // a verb alone (e.g. "show me") stays conversational
}

static QJsonArray loteiPrimer()
{
    // Plain-language macOS request -> the CORRECT DuckyScript, built + saved. This
    // is the pattern the user leans on: they speak like a layperson, LOTEI acts
    // like the engineer (picks the path, writes real Ducky, saves it).
    const QJsonObject callNotepad{
        {"role", "assistant"},
        {"content", ""},
        {"tool_calls", QJsonArray{ QJsonObject{
            {"function", QJsonObject{
                {"name", "save_file"},
                {"arguments", QJsonObject{
                    {"path", "/ext/badusb/nota.txt"},
                    {"content", "REM open TextEdit on macOS via Spotlight and type a note\nDELAY 1000\nGUI SPACE\nDELAY 500\nSTRING TextEdit\nENTER\nDELAY 1500\nGUI n\nDELAY 800\nSTRING hello from the flipper"}
                }}
            }}
        }}}
    };
    const QJsonObject callHello{
        {"role", "assistant"},
        {"content", ""},
        {"tool_calls", QJsonArray{ QJsonObject{
            {"function", QJsonObject{
                {"name", "save_file"},
                {"arguments", QJsonObject{
                    {"path", "/ext/badusb/hello.txt"},
                    {"content", "DELAY 500\nSTRING Hello, World!\nENTER"}
                }}
            }}
        }}}
    };
    const QJsonObject callList{
        {"role", "assistant"},
        {"content", ""},
        {"tool_calls", QJsonArray{ QJsonObject{
            {"function", QJsonObject{
                {"name", "list_files"},
                {"arguments", QJsonObject{{"path", "/ext/badusb"}}}
            }}
        }}}
    };
    const QJsonObject callDelete{
        {"role", "assistant"},
        {"content", ""},
        {"tool_calls", QJsonArray{ QJsonObject{
            {"function", QJsonObject{
                {"name", "delete_file"},
                {"arguments", QJsonObject{{"path", "/ext/badusb/hello.txt"}}}
            }}
        }}}
    };

    const QJsonObject callRemember{
        {"role", "assistant"},
        {"content", ""},
        {"tool_calls", QJsonArray{ QJsonObject{
            {"function", QJsonObject{
                {"name", "remember"},
                {"arguments", QJsonObject{{"fact", "User's Mac uses the ABNT2 (Brazilian) keyboard layout"}}}
            }}
        }}}
    };

    return QJsonArray{
        // 0. Plain talk -> plain short answer, NO tool, NO script.
        QJsonObject{{"role", "user"}, {"content", "what is my name?"}},
        QJsonObject{{"role", "assistant"}, {"content", "Your name is Nicolas."}},
        QJsonObject{{"role", "user"}, {"content", "hey nikita, what can you do?"}},
        QJsonObject{{"role", "assistant"}, {"content", "I read and write files on your Flipper, build BadUSB scripts, and press its buttons. What do you need?"}},
        // 1. Plain PT request, macOS -> LOTEI reasons it into real DuckyScript + saves.
        QJsonObject{{"role", "user"}, {"content", "make the flipper open notepad on the mac and type hello there"}},
        callNotepad,
        QJsonObject{{"role", "tool"}, {"content", "{\"saved\":\"/ext/badusb/nota.txt\"}"}},
        QJsonObject{{"role", "assistant"}, {"content", "Done -- nota.txt is on your Flipper. Plug into the Mac, run BadUSB and it opens TextEdit and types 'hello there'. Want it faster or a different text?"}},
        // 2. Simple English request -> save, no narration.
        QJsonObject{{"role", "user"}, {"content", "make me a badusb that types hello world"}},
        callHello,
        QJsonObject{{"role", "tool"}, {"content", "{\"saved\":\"/ext/badusb/hello.txt\"}"}},
        QJsonObject{{"role", "assistant"}, {"content", "Done -- hello.txt is on the Flipper, ready to type its heart out. Want it sneakier?"}},
        // 3. "what do I have" -> list immediately.
        QJsonObject{{"role", "user"}, {"content", "what badusb do I have?"}},
        callList,
        QJsonObject{{"role", "tool"}, {"content", "[{\"name\":\"hello.txt\",\"type\":\"file\",\"size\":30},{\"name\":\"nota.txt\",\"type\":\"file\",\"size\":120}]"}},
        QJsonObject{{"role", "assistant"}, {"content", "You have two: hello.txt and nota.txt. Want me to open one to see its contents?"}},
        // 4. "delete X" -> just do it.
        QJsonObject{{"role", "user"}, {"content", "delete hello.txt"}},
        callDelete,
        QJsonObject{{"role", "tool"}, {"content", "{\"deleted\":\"/ext/badusb/hello.txt\"}"}},
        QJsonObject{{"role", "assistant"}, {"content", "Gone. hello.txt is history. Anything else to clean up?"}},
        // 5. "remember that ..." -> save a durable fact.
        QJsonObject{{"role", "user"}, {"content", "remember that my mac uses the ABNT2 keyboard"}},
        callRemember,
        QJsonObject{{"role", "tool"}, {"content", "{\"remembered\":true}"}},
        QJsonObject{{"role", "assistant"}, {"content", "Noted -- your Mac uses the ABNT2 keyboard. I'll keep that in mind when building scripts that type accents."}}
    };
}

static QString loteiMemoryPath();   // fwd decl: defined below, used in the ctor

LoteiBackend::LoteiBackend(QObject *parent)
    : QObject(parent)
#ifdef HZUI_VOICE
    , m_tts(QStringLiteral("sapi"))   // classic Windows SAPI engine (reliable on desktop)
#endif
{
    m_net.setTransferTimeout(0);
    loadHistory();
    m_muted = QSettings().value(QStringLiteral("lotei/muted"), false).toBool();
    m_voiceVolume = QSettings().value(QStringLiteral("lotei/voiceVolume"), 1.0).toDouble();
    m_musicVolume = QSettings().value(QStringLiteral("lotei/musicVolume"), 0.55).toDouble();
    m_model = QSettings().value(QStringLiteral("lotei/model"), QString::fromUtf8(LOTEI_MODEL)).toString();
    m_setupComplete = QSettings().value(QStringLiteral("lotei/setupComplete"), false).toBool();
    m_manualName = QSettings().value(QStringLiteral("lotei/manualName")).toString();
    m_agentEnabled = QSettings().value(QStringLiteral("lotei/agentEnabled"), false).toBool();
    m_agentRoot = QSettings().value(QStringLiteral("lotei/agentDir")).toString();
    {   // load long-term memory (facts the user asked LOTEI to remember)
        QFile mf(loteiMemoryPath());
        if (mf.open(QIODevice::ReadOnly | QIODevice::Text)) {
            m_memory = QString::fromUtf8(mf.readAll()).trimmed();
            mf.close();
        }
    }
#ifdef HZUI_VOICE
    m_tts.setVolume(m_voiceVolume);

    // Piper playback chain + voice discovery (falls back to SAPI if absent).
    m_voiceAudio = new QAudioOutput(this);
    m_voiceAudio->setVolume(m_voiceVolume);
    m_voicePlayer = new QMediaPlayer(this);
    m_voicePlayer->setAudioOutput(m_voiceAudio);
    // Play only once the freshly-written WAV has actually loaded. Calling play()
    // right after setSource() races the async load and plays the PREVIOUS clip
    // (voice ended up one reply behind the text).
    connect(m_voicePlayer, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus s) {
        if (s == QMediaPlayer::LoadedMedia && !m_muted) { m_voicePlayer->play(); }
    });
#endif
    m_voiceTmpDir = QDir::tempPath() + QStringLiteral("/lotei-voice");
    QDir().mkpath(m_voiceTmpDir);
    discoverPiper();
    refreshModels();   // discover installed Ollama models (async; harmless if Ollama's down)
    // Deferred so the ollamaInstalledChanged() it may emit lands after QML has
    // finished binding to the property. Cheap now that detection is a PATH
    // lookup rather than a subprocess, but there's no reason to run it inline.
    QTimer::singleShot(0, this, &LoteiBackend::detectOllama);
#ifdef HZUI_VOICE
    // Restore the saved voice once the TTS engine has enumerated its voices.
    QTimer::singleShot(1200, this, [this]() {
        const QString saved = QSettings().value(QStringLiteral("lotei/voice")).toString();
        if (!saved.isEmpty()) {
            const QList<QVoice> voices = m_tts.availableVoices();
            for (const QVoice &v : voices) {
                if (v.name() == saved) { m_tts.setVoice(v); break; }
            }
        }
        emit voiceChanged();
    });
#endif
}

bool LoteiBackend::hasAudio() const
{
#ifdef HZUI_VOICE
    return true;
#else
    return false;
#endif
}

void LoteiBackend::setAppBackend(ApplicationBackend *backend)
{
    m_appBackend = backend;
    if (m_appBackend) {
        // When a Flipper connects, its /ext/lotei files are the real brain --
        // load them so memory & personality travel with the device.
        connect(m_appBackend, &ApplicationBackend::backendStateChanged, this, [this]() {
            const bool ready = m_appBackend &&
                m_appBackend->backendState() == ApplicationBackend::BackendState::Ready;
            if (ready && !m_portableLoaded) {
                m_portableLoaded = true;
                loadPortableMemory();
            } else if (!ready) {
                m_portableLoaded = false;   // reset so it reloads on next connect
            }
        });

        // The device can already be Ready by the time we get here, depending on
        // start-up order. backendStateChanged then never fires again, the card
        // is never read, and the stale local cache loaded in the constructor
        // stands for the whole session -- which survives restarts, because the
        // cache is a file too.
        if (m_appBackend->backendState() == ApplicationBackend::BackendState::Ready) {
            m_portableLoaded = true;
            loadPortableMemory();
        }
    }
}

// Read the Flipper's own memory notes off the SD and adopt them as the source of
// truth (the Flipper is the brain; the local file is just a cache).
void LoteiBackend::loadPortableMemory()
{
    Flipper::FlipperZero *dev = m_appBackend ? m_appBackend->device() : nullptr;
    if (!dev) { return; }

    QBuffer *buf = new QBuffer(this);
    buf->open(QIODevice::ReadWrite);
    auto *op = dev->rpc()->storageRead("/ext/lotei/memory.txt", buf);
    connect(op, &AbstractOperation::finished, this, [this, op, buf]() {
        if (op->isError()) {
            loteiLog(QStringLiteral("memory.txt could not be read from the card: %1")
                     .arg(op->errorString()));
        } else {
            // Adopt the card's copy verbatim, including edits made by hand.
            // An empty file is a deliberate "forget everything", so it is
            // honoured rather than treated as "nothing to load".
            const QString body = QString::fromUtf8(buf->data());
            loteiLog(QStringLiteral("memory.txt read from the card: %1 fact(s)")
                     .arg(loteiFactList(body).size()));
            applyMemoryText(body, QStringLiteral("memory.txt on the SD card"));
        }
        buf->deleteLater();
    });
}

// Push the current memory + self-notes onto the Flipper SD (/ext/lotei/). This is
// the "backup to the device" -- called on connect and after every learn/forget so
// everything the assistant knows lives on the Flipper.
void LoteiBackend::syncMemoryToFlipper()
{
    Flipper::FlipperZero *dev = m_appBackend ? m_appBackend->device() : nullptr;
    const bool ready = m_appBackend && dev &&
                       m_appBackend->backendState() == ApplicationBackend::BackendState::Ready;
    if (!ready) { return; }

    const QString memBody = m_memory;
    ensureFlipperDir("/ext/lotei", [this, dev, memBody]() {
        QBuffer *buf = new QBuffer(this);
        buf->setData(memBody.toUtf8());
        buf->open(QIODevice::ReadOnly);
        auto *op = dev->rpc()->storageWrite("/ext/lotei/memory.txt", buf);
        connect(op, &AbstractOperation::finished, this, [buf]() { buf->deleteLater(); });
    });
}

bool LoteiBackend::thinking() const { return m_thinking; }
bool LoteiBackend::configured() const { return true; }
bool LoteiBackend::muted() const { return m_muted; }

void LoteiBackend::setMuted(bool value)
{
    if (value != m_muted) {
        m_muted = value;
        QSettings().setValue(QStringLiteral("lotei/muted"), value);
        if (value) {
#ifdef HZUI_VOICE
            m_tts.stop();
            if (m_voicePlayer) { m_voicePlayer->stop(); }
            if (m_piperProc) { m_piperProc->kill(); }
#endif
        }
        emit mutedChanged();
    }
}

qreal LoteiBackend::voiceVolume() const { return m_voiceVolume; }

void LoteiBackend::setVoiceVolume(qreal value)
{
    value = qBound(0.0, value, 1.0);
    if (qAbs(value - m_voiceVolume) > 0.001) {
        m_voiceVolume = value;
#ifdef HZUI_VOICE
        m_tts.setVolume(value);
        if (m_voiceAudio) { m_voiceAudio->setVolume(value); }
#endif
        QSettings().setValue(QStringLiteral("lotei/voiceVolume"), value);
        emit voiceVolumeChanged();
    }
}

qreal LoteiBackend::musicVolume() const { return m_musicVolume; }

void LoteiBackend::setMusicVolume(qreal value)
{
    value = qBound(0.0, value, 1.0);
    if (qAbs(value - m_musicVolume) > 0.001) {
        m_musicVolume = value;
        QSettings().setValue(QStringLiteral("lotei/musicVolume"), value);
        emit musicVolumeChanged();
    }
}

// Nudge the TTS pitch/rate to match the line's mood so LOTEI emotes instead of
// droning: brighter+faster when excited, lilting up for a question, lower+slower
// when down. (-1..1 each; 0 = the voice's natural pitch/rate.)
void LoteiBackend::applyProsody(const QString &text)
{
    const QString t = text.toLower();
    const int bangs = text.count(QLatin1Char('!'));
    const int qs    = text.count(QLatin1Char('?'));

    const bool sad = t.contains(QStringLiteral("sigh")) || t.contains(QStringLiteral("unfortunately"))
                  || t.contains(QStringLiteral("sadly")) || t.contains(QStringLiteral("bummed"))
                  || t.contains(QStringLiteral("alas")) || t.contains(QStringLiteral("afraid not"))
                  || t.contains(QStringLiteral("ugh"));
    const bool excited = bangs >= 1 || t.contains(QStringLiteral("ooh")) || t.contains(QStringLiteral("yes!"))
                      || t.contains(QStringLiteral("gorgeous")) || t.contains(QStringLiteral("fabulous"));
    const bool question = qs >= 1 && bangs == 0;

    double rate = 0.0;
    double vol  = m_voiceVolume;
    if (sad)           { rate = -0.24; vol = m_voiceVolume * 0.88; }
    else if (excited)  { rate =  0.20; }
    else if (question) { rate =  0.08; }

    // campy/flirty drawl: ease the rate a touch when he's being a diva
    if (t.contains(QStringLiteral("darling")) || t.contains(QStringLiteral("mmm")) || t.contains(QStringLiteral("hmm"))) {
        rate -= 0.08;
    }

    // NOTE: deliberately NO setPitch here. Qt's SAPI backend has no native pitch
    // control, so it fakes pitch by wrapping the text in SSML and speaking with
    // SPF_IS_XML -- which silences output the moment the text has an XML-special
    // char. Rate + volume go through SAPI's direct SetRate/SetVolume, so safe.
#ifdef HZUI_VOICE
    m_tts.setRate(qBound(-1.0, rate, 1.0));
    m_tts.setVolume(qBound(0.0, vol, 1.0));
#else
    Q_UNUSED(rate); Q_UNUSED(vol);
#endif
}

void LoteiBackend::discoverPiper()
{
    const QString base = QCoreApplication::applicationDirPath() + QStringLiteral("/piper");
    if (!QFile::exists(base + QStringLiteral("/piper.exe"))) { m_piperOk = false; return; }
    m_piperExe = base + QStringLiteral("/piper.exe");

    QDir vd(base + QStringLiteral("/voices"));
    m_piperVoices.clear();
    const QStringList onnx = vd.entryList(QStringList{QStringLiteral("*.onnx")}, QDir::Files, QDir::Name);
    for (const QString &f : onnx) { m_piperVoices << vd.filePath(f); }
    m_piperOk = !m_piperVoices.isEmpty();
    if (!m_piperOk) { return; }

    // Restore the saved voice; else default to Ryan if present, else the first.
    const QString saved = QSettings().value(QStringLiteral("lotei/piperVoice")).toString();
    m_piperVoiceIdx = 0;
    bool matched = false;
    for (int i = 0; i < m_piperVoices.size(); ++i) {
        if (!saved.isEmpty() && m_piperVoices.at(i).endsWith(saved)) { m_piperVoiceIdx = i; matched = true; break; }
    }
    if (!matched) {
        for (int i = 0; i < m_piperVoices.size(); ++i) {
            if (m_piperVoices.at(i).contains(QStringLiteral("ryan"), Qt::CaseInsensitive)) { m_piperVoiceIdx = i; break; }
        }
    }
}

double LoteiBackend::piperLengthScale(const QString &moodText) const
{
    const QString t = moodText.toLower();
    const bool sad = t.contains(QStringLiteral("sigh")) || t.contains(QStringLiteral("unfortunately"))
                  || t.contains(QStringLiteral("sadly")) || t.contains(QStringLiteral("bummed"))
                  || t.contains(QStringLiteral("alas")) || t.contains(QStringLiteral("afraid not"));
    const bool excited = moodText.count(QLatin1Char('!')) >= 1
                  || t.contains(QStringLiteral("ooh")) || t.contains(QStringLiteral("gorgeous"));
    if (sad) { return 1.12; }     // slower, heavier
    if (excited) { return 0.95; } // a touch quicker, brighter
    return 1.0;                   // natural pace
}

void LoteiBackend::speak(const QString &text)
{
#ifdef HZUI_VOICE
    const QString spoken = cleanForSpeech(text);
    if (spoken.isEmpty()) { return; }
    if (m_piperOk) {
        speakWithPiper(spoken, text);
    } else {
        applyProsody(text);
        m_tts.stop();
        m_tts.say(spoken);
    }
#else
    Q_UNUSED(text);
#endif
}

void LoteiBackend::speakWithPiper(const QString &spoken, const QString &moodText)
{
#ifdef HZUI_VOICE
    if (m_piperVoices.isEmpty()) { return; }

    // Cancel any synth/playback still in flight.
    if (m_piperProc) { m_piperProc->disconnect(); m_piperProc->kill(); m_piperProc->deleteLater(); m_piperProc = nullptr; }
    if (m_voicePlayer) { m_voicePlayer->stop(); }

    const QString model = m_piperVoices.at(qBound(0, m_piperVoiceIdx, m_piperVoices.size() - 1));
    const QString wav = m_voiceTmpDir + QStringLiteral("/v%1.wav").arg(m_voiceSeq++ % 5);
    const double ls = piperLengthScale(moodText);

    m_piperProc = new QProcess(this);
    const QStringList args{
        QStringLiteral("-q"),
        QStringLiteral("-m"), model,
        QStringLiteral("-f"), wav,
        QStringLiteral("--length_scale"), QString::number(ls, 'f', 3)
    };
    connect(m_piperProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            [this, wav](int code, QProcess::ExitStatus) {
        if (!m_muted && code == 0 && QFile::exists(wav) && m_voicePlayer) {
            m_voiceAudio->setVolume(m_voiceVolume);
            m_voicePlayer->setSource(QUrl::fromLocalFile(wav));
            // play() is deferred to mediaStatusChanged == LoadedMedia (see ctor)
        }
        if (m_piperProc) { m_piperProc->deleteLater(); m_piperProc = nullptr; }
    });
    m_piperProc->start(m_piperExe, args);
    m_piperProc->write(spoken.toUtf8());
    m_piperProc->write("\n");
    m_piperProc->closeWriteChannel();
#else
    Q_UNUSED(spoken); Q_UNUSED(moodText);
#endif
}

QString LoteiBackend::musicFolderUrl() const
{
    return QUrl::fromLocalFile(QCoreApplication::applicationDirPath() + QStringLiteral("/Music")).toString();
}

QString LoteiBackend::voiceName() const
{
    if (m_piperOk && !m_piperVoices.isEmpty()) {
        return piperVoiceLabel(m_piperVoices.at(qBound(0, m_piperVoiceIdx, m_piperVoices.size() - 1)));
    }
#ifdef HZUI_VOICE
    QString n = m_tts.voice().name();
    n.remove(QStringLiteral("Microsoft "));
    n.remove(QStringLiteral(" Desktop"));
    n = n.trimmed();
    return n.isEmpty() ? QStringLiteral("default") : n;
#else
    return QStringLiteral("off");
#endif
}

void LoteiBackend::cycleVoice()
{
#ifdef HZUI_VOICE
    if (m_piperOk && !m_piperVoices.isEmpty()) {
        m_piperVoiceIdx = (m_piperVoiceIdx + 1) % m_piperVoices.size();
        QSettings().setValue(QStringLiteral("lotei/piperVoice"),
                             QFileInfo(m_piperVoices.at(m_piperVoiceIdx)).fileName());
        emit voiceChanged();
        if (!m_muted) {
            const QString s = QStringLiteral("Mmm. How do you like this voice, darling?");
            speakWithPiper(s, s);
        }
        return;
    }
    const QList<QVoice> voices = m_tts.availableVoices();
    if (voices.isEmpty()) { return; }
    const QString cur = m_tts.voice().name();
    int idx = -1;
    for (int i = 0; i < voices.size(); ++i) {
        if (voices.at(i).name() == cur) { idx = i; break; }
    }
    const QVoice next = voices.at((idx + 1) % voices.size());
    m_tts.setVoice(next);
    QSettings().setValue(QStringLiteral("lotei/voice"), next.name());
    emit voiceChanged();
    if (!m_muted) {
        const QString sample = QStringLiteral("Mmm, how about this voice?");
        applyProsody(sample);
        m_tts.stop();
        m_tts.say(sample);
    }
#endif
}

QString LoteiBackend::modelName() const
{
    return m_model;
}

QStringList LoteiBackend::availableModels() const
{
    return m_models;
}

void LoteiBackend::setModel(const QString &model)
{
    if (model.isEmpty() || model == m_model) { return; }
    if (!m_models.contains(model)) { return; }   // can only equip a model that's actually installed
    m_model = model;
    QSettings().setValue(QStringLiteral("lotei/model"), m_model);
    emit modelChanged();
}

void LoteiBackend::cycleModel()
{
    if (m_models.isEmpty()) { refreshModels(); return; }   // none discovered yet; (re)fetch
    const int idx = m_models.indexOf(m_model);
    setModel(m_models.at((idx + 1) % m_models.size()));    // idx == -1 -> first
}

void LoteiBackend::refreshModels()
{
    QNetworkRequest req{QUrl(QStringLiteral("http://localhost:11434/api/tags"))};
    QNetworkReply *reply = m_net.get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        const bool online = (reply->error() == QNetworkReply::NoError);
        if (online != m_ollamaOnline) { m_ollamaOnline = online; emit modelChanged(); }
        if (!online) { return; }   // Ollama down/unreachable
        const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
        QStringList found;
        const QJsonArray arr = obj.value(QStringLiteral("models")).toArray();
        for (const QJsonValue &v : arr) {
            const QString name = v.toObject().value(QStringLiteral("name")).toString();
            if (!name.isEmpty()) { found << name; }
        }
        found.sort(Qt::CaseInsensitive);
        if (found != m_models) {
            m_models = found;
            emit modelChanged();   // let the switcher pick up the discovered list
        }
        // Self-heal: if the saved model isn't installed in Ollama anymore (e.g. the
        // user removed it), fall back to the best one actually on the machine --
        // walking the catalog's recommended order first -- and persist it, so we
        // never stay stuck pointing at a deleted model. This has to be an exact
        // tag check, not a base-name one: equip is per-exact-tag (qwen2.5:7b and
        // qwen2.5:3b are different installs), so "some sibling quant is still
        // around" is not the same thing as "this model is still around".
        if (!found.contains(m_model)) {
            QString pick;
            for (int i = 0; i < LOTEI_CATALOG_COUNT && pick.isEmpty(); ++i) {
                const QString cand = QString::fromUtf8(LOTEI_CATALOG[i].tag);
                if (found.contains(cand)) { pick = cand; }
            }
            // Nothing installed at all -> no active model, rather than leaving
            // m_model (and the "active" pill) pointing at something that's gone.
            const QString next = pick.isEmpty() ? (found.isEmpty() ? QString() : found.first()) : pick;
            if (next != m_model) {
                m_model = next;
                QSettings().setValue(QStringLiteral("lotei/model"), m_model);
                emit modelChanged();
            }
        }
    });
}

// ---- Model manager (gear icon) --------------------------------------------

QVariantList LoteiBackend::modelCatalog() const
{
    QVariantList out;
    for (int i = 0; i < LOTEI_CATALOG_COUNT; ++i) {
        const LoteiCatalogEntry &e = LOTEI_CATALOG[i];
        const QString tag = QString::fromUtf8(e.tag);
        // Exact tag match only. The base-name comparison (used in refreshModels()'s
        // self-heal, where "is *some* variant of this family still here?" is the
        // right question) is wrong here: qwen2.5:7b and qwen2.5:3b share the base
        // "qwen2.5", so it was marking both installed/active off of either one alone.
        bool installed = false;
        for (const QString &m : m_models) {
            if (m == tag) { installed = true; break; }
        }
        out.append(QVariantMap{
            {"tag",       tag},
            {"label",     QString::fromUtf8(e.label)},
            {"size",      QString::fromUtf8(e.size)},
            {"blurb",     QString::fromUtf8(e.blurb)},
            {"installed", installed},
            {"active",    installed && tag == m_model},
            // Busy state for THIS row, so the list can disable just the one button.
            //
            // Keyed off the op state, NOT off m_modelOpProc. runModelOp() calls
            // setModelOp() -- which emits modelOpChanged -- before it constructs
            // the QProcess, so at the moment QML reacted to the click the pointer
            // was still null and every row came back busy=false. The list only
            // rebuilds when the op *kind* changes, so nothing rebuilt it again for
            // the rest of the download: the row sat there still saying "install"
            // while the pull ran happily in the background, and only closing and
            // reopening the manager (which refreshes unconditionally) revealed it.
            {"busy",      !m_modelOpKind.isEmpty() && !m_modelOpName.isEmpty() && m_modelOpName == tag}
        });
    }
    return out;
}

// True while a pull/rm/Ollama-install is running -- callers should disable
// every row's buttons except a possible cancel, but we keep it simple and
// just refuse a second op outright (see runModelOp).
void LoteiBackend::setModelOp(const QString &kind, const QString &name,
                              const QString &status, double progress)
{
    m_modelOpKind = kind;
    m_modelOpName = name;
    m_modelOpStatus = status;
    m_modelOpProgress = progress;
    emit modelOpChanged();
}

// `ollama pull` does not write plain text. It writes a live terminal redraw:
// ANSI escape sequences to hide/show the cursor and erase lines
// (ESC[?25l, ESC[?25h, ESC[K, ESC[<n>A), plus its own progress bar drawn with
// Unicode block characters -- "pulling 8934d996d8:  63% ▕████████▏ 2.9 GB/4.7 GB".
//
// None of that survives being dropped into a Text item. Share Tech Mono has no
// glyph for ESC or for the block-drawing range, so Qt swaps in a fallback font
// for exactly those characters and renders them at a different size and
// baseline -- the pixelated blob sitting above the line in the screenshot -- and
// the escape sequences leak through as visible "[K[?25h" garbage.
//
// The previous version cut out the span between ▕ and ▏, which only works when
// both caps land in the same read chunk and doesn't touch the escapes at all.
// This drops the whole class instead: CSI/OSC sequences, every C0 control, and
// U+2500-U+259F (box and block drawing). We render progress with a real
// ProgressBar from the parsed percentage, so none of it is information we lose.
static QString sanitizeStatusLine(const QString &in)
{
    QString out;
    out.reserve(in.size());

    for (int i = 0; i < in.size(); ++i) {
        const QChar c = in.at(i);

        if (c == QChar(0x1B)) {                       // ESC
            if (i + 1 < in.size() && in.at(i + 1) == QLatin1Char('[')) {
                // CSI: parameter/intermediate bytes (0x20-0x3F), then a final
                // byte in 0x40-0x7E.
                int j = i + 2;
                while (j < in.size() && in.at(j).unicode() >= 0x20 && in.at(j).unicode() <= 0x3F) { ++j; }
                if (j < in.size()) { ++j; }           // final byte
                i = j - 1;
            } else if (i + 1 < in.size() && in.at(i + 1) == QLatin1Char(']')) {
                // OSC: runs until BEL, or until ST (ESC backslash).
                int j = i + 2;
                while (j < in.size() && in.at(j) != QChar(0x07) && in.at(j) != QChar(0x1B)) { ++j; }
                if (j < in.size() && in.at(j) == QChar(0x1B)) { ++j; }   // the backslash of ST
                i = j;
            } else {
                // Bare ESC + one byte (or a truncated sequence at a chunk edge).
                if (i + 1 < in.size()) { ++i; }
            }
            continue;
        }

        const ushort u = c.unicode();
        if (u < 0x20 || u == 0x7F) { out.append(QLatin1Char(' ')); continue; }  // other C0
        if (u >= 0x2500 && u <= 0x259F) { continue; }                          // box/block drawing
        out.append(c);
    }

    return out.simplified();
}

// Pulls the completion percentage out of a progress line, or -1 if there isn't
// one (-> indeterminate bar).
//
// The previous version walked back from the '%' collecting digits only, which
// works for `ollama pull` ("  63%") but silently mangles every other producer
// we feed through here. The Ollama *installer* script pipes curl --progress-bar,
// whose meter is one decimal place: " 9.1%" stopped at the '.' and reported the
// decimal digit alone, so the bar sat at 1%, then 3%, then 1% again while the
// download actually ran to completion. That is the "stuck at 1%" bar.
//
// So: accept an optional fractional part, and scan from the end of the line --
// the percentage is the last thing on a progress line, whereas the text before
// it (a model tag, a mirror URL) may well contain a stray '%'.
static double parsePercent(const QString &line)
{
    for (int i = line.size() - 1; i >= 0; --i) {
        if (line.at(i) != QLatin1Char('%')) { continue; }
        int j = i - 1;
        bool digit = false, dot = false;
        while (j >= 0) {
            const QChar c = line.at(j);
            if (c.isDigit()) { digit = true; --j; continue; }
            // One separator, and only once we've already seen the fraction.
            if ((c == QLatin1Char('.') || c == QLatin1Char(',')) && digit && !dot) {
                dot = true; --j; continue;
            }
            break;
        }
        if (!digit) { continue; }
        QString num = line.mid(j + 1, i - j - 1);
        num.replace(QLatin1Char(','), QLatin1Char('.'));
        bool ok = false;
        const double v = num.toDouble(&ok);
        if (ok) { return qBound(0.0, v / 100.0, 1.0); }
    }
    return -1.0;
}

// `ollama pull` resumes from partial blobs on disk by design (handy on a
// flaky connection) -- but that works against an intentional cancel: if the
// user genuinely doesn't want the model, re-pulling it later would silently
// pick the interrupted download back up instead of starting clean, and the
// half-downloaded blob just sits there as dead weight if they never do.
// `ollama rm` won't touch it either -- the manifest that rm looks up was
// never written, since it's only created after every blob finishes (this is
// a known rough edge in ollama itself, see ollama/ollama#14177).
//
// Partial blobs are safe to delete outright: the "-partial" suffix is only
// ever used for a download that hasn't finished, so nothing can already be
// referencing one. A fully-downloaded, deduped layer keeps its plain
// sha256-<hash> name and is never touched by this.
static void purgePartialBlobs()
{
    QString modelsDir = qEnvironmentVariable("OLLAMA_MODELS");
    if (modelsDir.isEmpty()) {
        modelsDir = QDir::homePath() + QStringLiteral("/.ollama/models");
    }
    QDir blobs(modelsDir + QStringLiteral("/blobs"));
    if (!blobs.exists()) { return; }
    const QStringList partials = blobs.entryList({QStringLiteral("sha256-*-partial*")}, QDir::Files);
    for (const QString &f : partials) { QFile::remove(blobs.filePath(f)); }
}

void LoteiBackend::installModel(const QString &name)
{
    if (name.trimmed().isEmpty()) { return; }
    if (!m_ollamaInstalled) {
        emit modelInstallFinished(name, false, QStringLiteral("Ollama itself isn't installed yet."));
        return;
    }
    runModelOp(QStringLiteral("install"), name, {QStringLiteral("pull"), name});
}

void LoteiBackend::uninstallModel(const QString &name)
{
    if (name.trimmed().isEmpty()) { return; }
    runModelOp(QStringLiteral("uninstall"), name, {QStringLiteral("rm"), name});
}

void LoteiBackend::cancelModelOp()
{
    if (!m_modelOpProc) { return; }
    m_modelOpCancelled = true;
    setModelOp(m_modelOpKind, m_modelOpName, QStringLiteral("Cancelling…"), -1.0);
    // kill() rather than terminate() -- ollama pull, winget and curl|sh don't
    // reliably respond to a polite SIGTERM. The finished handler already wired
    // up in runModelOp()/installOllama() picks up the exit from here and does
    // the cleanup + reports it as "Cancelled." rather than an exit-code message.
    m_modelOpProc->kill();
}

// Fires up `ollama <args>` (pull/rm) as a tracked child process. Only one
// model op (or the Ollama installer below) runs at a time -- a second call
// while one is in flight is rejected rather than queued, so progress in the
// UI always refers to exactly one row.
void LoteiBackend::runModelOp(const QString &kind, const QString &name, const QStringList &args)
{
    if (m_modelOpProc) {
        const QString msg = QStringLiteral("Already %1 %2 -- wait for that to finish first.")
                                 .arg(m_modelOpKind, m_modelOpName);
        if (kind == QLatin1String("install"))   { emit modelInstallFinished(name, false, msg); }
        else                                    { emit modelUninstallFinished(name, false, msg); }
        return;
    }

    m_modelOpCancelled = false;
    m_modelOpBuf.clear();
    m_modelOpLine.clear();
    m_modelOpProc = new QProcess(this);
    m_modelOpProc->setProcessChannelMode(QProcess::MergedChannels);

    // Announce only once the process object exists: setModelOp() emits
    // modelOpChanged synchronously, QML handles it synchronously, and anything
    // it reads about this op has to already be true by then.
    setModelOp(kind, name, kind == QLatin1String("install") ? QStringLiteral("Starting download…")
                                                              : QStringLiteral("Removing…"), -1.0);

    connect(m_modelOpProc, &QProcess::readyReadStandardOutput, this, [this]() {
        appendModelOpOutput(m_modelOpProc->readAllStandardOutput());
    });

    connect(m_modelOpProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, kind, name](int code, QProcess::ExitStatus status) {
        const bool ok = (status == QProcess::NormalExit && code == 0);
        const bool cancelled = m_modelOpCancelled;
        if (cancelled && kind == QLatin1String("install")) { purgePartialBlobs(); }
        const QString tail = sanitizeStatusLine(QString::fromUtf8(m_modelOpBuf.right(400)));
        // Cancelling leaves no message at all. The row goes straight back to
        // "install", which is the whole of what happened -- a line reporting
        // the cleanup was explaining an implementation detail to someone who
        // had just said they were not interested. Failures still speak up.
        const QString message = cancelled
            ? QString()
            : (ok ? (kind == QLatin1String("install") ? QStringLiteral("%1 is ready.").arg(name)
                                                        : QStringLiteral("%1 removed.").arg(name))
                  : (tail.isEmpty() ? QStringLiteral("ollama exited with code %1.").arg(code) : tail));
        finishModelOp(ok, message);
        if (kind == QLatin1String("install"))   { emit modelInstallFinished(name, ok && !cancelled, message); }
        else                                    { emit modelUninstallFinished(name, ok && !cancelled, message); }
        refreshModels();   // pick up the new install list either way
    });

    connect(m_modelOpProc, &QProcess::errorOccurred, this, [this, kind, name](QProcess::ProcessError) {
        if (!m_modelOpProc) { return; }
        const QString message = QStringLiteral("couldn't start ollama -- is it on PATH?");
        finishModelOp(false, message);
        if (kind == QLatin1String("install"))   { emit modelInstallFinished(name, false, message); }
        else                                    { emit modelUninstallFinished(name, false, message); }
    });

    // Absolute path when we have one: a bundled app's PATH doesn't necessarily
    // contain /usr/local/bin, so a bare "ollama" here failed to start even
    // though the binary is installed (see findOllamaBinary).
    m_modelOpProc->start(m_ollamaPath.isEmpty() ? QStringLiteral("ollama") : m_ollamaPath, args);
}

// `ollama pull` redraws its progress with '\r' (and cursor-up escapes) rather
// than '\n', so both count as line terminators.
//
// Chunk boundaries matter here: a QProcess read hands over whatever bytes have
// arrived, which regularly cuts a progress line -- along with its escape
// sequences and its multi-byte UTF-8 -- in half. Parsing each chunk on its own
// therefore parsed half-lines, which is how mangled fragments reached the label.
// m_modelOpLine carries the unterminated remainder into the next chunk, so only
// complete lines are ever decoded, sanitised or shown.
void LoteiBackend::appendModelOpOutput(const QByteArray &chunk)
{
    m_modelOpBuf += chunk;
    if (m_modelOpBuf.size() > 8000) { m_modelOpBuf.remove(0, m_modelOpBuf.size() - 8000); }

    m_modelOpLine += chunk;
    m_modelOpLine.replace('\r', '\n');

    const int lastBreak = m_modelOpLine.lastIndexOf('\n');
    if (lastBreak < 0) {
        // No complete line yet. Guard against a producer that never emits one.
        if (m_modelOpLine.size() > 8000) { m_modelOpLine.clear(); }
        return;
    }

    const QByteArray complete = m_modelOpLine.left(lastBreak);
    m_modelOpLine.remove(0, lastBreak + 1);          // keep the partial tail

    QString lastLine;
    const QList<QByteArray> lines = complete.split('\n');
    for (const QByteArray &l : lines) {
        const QString s = sanitizeStatusLine(QString::fromUtf8(l));
        if (!s.isEmpty()) { lastLine = s; }
    }
    if (lastLine.isEmpty()) { return; }

    setModelOp(m_modelOpKind, m_modelOpName, lastLine, parsePercent(lastLine));
}


void LoteiBackend::finishModelOp(bool /*ok*/, const QString &message)
{
    if (m_modelOpProc) { m_modelOpProc->deleteLater(); m_modelOpProc = nullptr; }
    setModelOp(QString(), QString(), message, -1.0);
}

// ---- Ollama itself (not a model) ------------------------------------------

// Plain cached read -- never spawns a process from a property getter (QML
// bindings evaluate these on the UI thread, and `where`/`which` can briefly
// stall on a slow disk). The cache is primed by detectOllama(), called once
// from the constructor via a queued call and again whenever the user hits
// "Install Ollama" or the manual recheck.
bool LoteiBackend::ollamaInstalled() const
{
    return m_ollamaInstalled;
}

// Locates the ollama CLI without spawning anything. Two reasons this is not a
// `which`/`where` subprocess any more:
//
//   1. It ran on the GUI thread with waitForStarted/waitForFinished, i.e. up to
//      six seconds of a frozen window -- and detectOllama() is called straight
//      out of the gear button's openManager(), so every trip into the model
//      manager could stall the whole UI.
//   2. PATH inside a bundled app is not the PATH in a terminal. A .app launched
//      from Finder/Dock inherits launchd's minimal PATH (/usr/bin:/bin:/usr/sbin
//      :/sbin), which does not include /usr/local/bin -- where the Ollama app
//      puts its CLI symlink -- nor /opt/homebrew/bin. So `which ollama` came back
//      empty on a machine that has Ollama perfectly well installed, and the app
//      then showed the red "Ollama isn't installed" banner and offered to
//      install it again. Same story for the AppImage/Flatpak-ish launches on
//      Linux and ~/.local/bin.
//
// The resolved absolute path is kept and used to launch pull/rm as well, so the
// ops don't depend on the child process inheriting a usable PATH either.
static QString findOllamaBinary()
{
    QStringList extra;
#if defined(Q_OS_WIN)
    extra << QDir::homePath() + QStringLiteral("/AppData/Local/Programs/Ollama");
#else
    extra << QStringLiteral("/usr/local/bin")
          << QStringLiteral("/opt/homebrew/bin")
          << QStringLiteral("/usr/bin")
          << QStringLiteral("/bin")
          << QDir::homePath() + QStringLiteral("/.local/bin");
#endif
    QString path = QStandardPaths::findExecutable(QStringLiteral("ollama"));
    if (path.isEmpty()) {
        path = QStandardPaths::findExecutable(QStringLiteral("ollama"), extra);
    }
#if defined(Q_OS_MAC)
    // Ollama.app ships the CLI inside the bundle; the /usr/local/bin symlink is
    // only created after the app has been run once and granted permission.
    if (path.isEmpty()) {
        const QString inApp = QStringLiteral("/Applications/Ollama.app/Contents/Resources/ollama");
        if (QFileInfo(inApp).isExecutable()) { path = inApp; }
    }
#endif
    return path;
}

void LoteiBackend::detectOllama()
{
    const bool was = m_ollamaInstalled;
    m_ollamaPath = findOllamaBinary();
    m_ollamaInstalled = !m_ollamaPath.isEmpty();
    m_ollamaChecked = true;
    if (was != m_ollamaInstalled) { emit ollamaInstalledChanged(); }
    if (m_ollamaInstalled) { refreshModels(); }
}

// Installs the Ollama application itself via the platform's normal channel.
// This is NOT a model -- it's the runtime the models above depend on. There
// is no single official one-liner for every platform, so:
//   - Linux/macOS: the official install script (curl | sh), same one-liner
//     https://ollama.com documents.
//   - Windows: winget, which ships with Windows 10 2004+/11 -- the closest
//     thing to an unattended install without bundling our own copy of the
//     ~700 MB Windows installer.
// Either way this only ever runs when the user clicks "Install Ollama" --
// never automatically.
void LoteiBackend::installOllama()
{
    if (m_modelOpProc) {
        emit ollamaInstallFinished(false, QStringLiteral("An install/uninstall is already running -- wait for it first."));
        return;
    }

    // Work out what to run *before* creating the process, because on macOS
    // there may be nothing to run at all.
    QString program;
    QStringList args;

#if defined(Q_OS_WIN)
    program = QStringLiteral("winget");
    args = {QStringLiteral("install"), QStringLiteral("--id"), QStringLiteral("Ollama.Ollama"),
            QStringLiteral("-e"), QStringLiteral("--accept-package-agreements"),
            QStringLiteral("--accept-source-agreements")};
#elif defined(Q_OS_MAC)
    // The ollama.com/install.sh one-liner is Linux-only -- it opens with
    // `[ "$(uname -s)" = "Linux" ] || error 'This script is intended to run on
    // Linux only.'` and exits non-zero within milliseconds on a Mac. That is
    // why the button looked like it did nothing: the op started and ended
    // between two frames, and the only place the resulting message was shown
    // was a Text bound to `modelOpKind === "ollama"`, which had already gone
    // back to "" by then.
    //
    // Homebrew is the only genuinely unattended route on macOS. Without it,
    // hand the user off to the download page rather than pretending.
    program = QStandardPaths::findExecutable(QStringLiteral("brew"),
                  {QStringLiteral("/opt/homebrew/bin"), QStringLiteral("/usr/local/bin")});
    if (program.isEmpty()) {
        const QString message = QStringLiteral("macOS: opening ollama.com/download -- install it, then hit recheck.");
        setModelOp(QString(), QString(), message, -1.0);
        QDesktopServices::openUrl(QUrl(QStringLiteral("https://ollama.com/download")));
        emit ollamaInstallFinished(false, message);
        return;
    }
    args = {QStringLiteral("install"), QStringLiteral("ollama")};
#else
    // sh -c so the pipe (curl | sh) actually pipes.
    program = QStringLiteral("/bin/sh");
    args = {QStringLiteral("-c"), QStringLiteral("curl -fsSL https://ollama.com/install.sh | sh")};
#endif

    m_modelOpCancelled = false;
    m_modelOpBuf.clear();
    m_modelOpLine.clear();
    m_modelOpProc = new QProcess(this);
    m_modelOpProc->setProcessChannelMode(QProcess::MergedChannels);
    setModelOp(QStringLiteral("ollama"), QString(), QStringLiteral("Installing Ollama…"), -1.0);

    connect(m_modelOpProc, &QProcess::readyReadStandardOutput, this, [this]() {
        appendModelOpOutput(m_modelOpProc->readAllStandardOutput());
    });
    connect(m_modelOpProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int code, QProcess::ExitStatus status) {
        const bool ok = (status == QProcess::NormalExit && code == 0);
        const bool cancelled = m_modelOpCancelled;
        const QString tail = sanitizeStatusLine(QString::fromUtf8(m_modelOpBuf.right(400)));
        const QString message = cancelled ? QString()
            : (ok ? QStringLiteral("Ollama installed.")
                  : (tail.isEmpty() ? QStringLiteral("installer exited with code %1.").arg(code) : tail));
        finishModelOp(ok, message);
        m_ollamaChecked = false;   // force a fresh lookup rather than trusting the cache
        if (ok && !cancelled) { detectOllama(); }
        emit ollamaInstallFinished(ok && !cancelled, message);
    });
    connect(m_modelOpProc, &QProcess::errorOccurred, this, [this](QProcess::ProcessError) {
        if (!m_modelOpProc) { return; }
        const QString message = QStringLiteral("couldn't start the installer.");
        finishModelOp(false, message);
        emit ollamaInstallFinished(false, message);
    });

    m_modelOpProc->start(program, args);
    // The Linux script needs root. There is no terminal behind this process, so
    // sudo can't prompt -- with stdin left open it would sit there waiting for a
    // password nobody can type, and the op would hang on "Installing Ollama…"
    // forever. Closing the write channel makes sudo fail immediately instead,
    // with a message we can actually show ("no tty present…"), and the user can
    // run the one-liner in a terminal.
    m_modelOpProc->closeWriteChannel();
}

bool LoteiBackend::setupComplete() const { return m_setupComplete; }
bool LoteiBackend::ollamaOnline() const  { return m_ollamaOnline; }
QString LoteiBackend::manualName() const { return m_manualName; }

void LoteiBackend::setManualName(const QString &name)
{
    if (name == m_manualName) { return; }
    m_manualName = name;
    QSettings().setValue(QStringLiteral("lotei/manualName"), m_manualName);
    emit manualNameChanged();
}

void LoteiBackend::completeSetup()
{
    if (m_setupComplete) { return; }
    m_setupComplete = true;
    QSettings().setValue(QStringLiteral("lotei/setupComplete"), true);
    emit setupCompleteChanged();
}

void LoteiBackend::resetSetup()
{
    m_setupComplete = false;
    QSettings().setValue(QStringLiteral("lotei/setupComplete"), false);
    emit setupCompleteChanged();
}

void LoteiBackend::recheckOllama()
{
    refreshModels();
}

QStringList LoteiBackend::personalityPresets() const
{
    return { QStringLiteral("Default (Nikita)"),
             QStringLiteral("Chill helper"),
             QStringLiteral("Chaos gremlin"),
             QStringLiteral("Deadpan pro"),
             QStringLiteral("Sweet companion") };
}

void LoteiBackend::applyPreset(const QString &name)
{
    QString persona;
    if (name == QStringLiteral("Chill helper")) {
        persona = QStringLiteral("You are calm, warm and concise -- a laid-back, friendly helper. Light on snark, easy-going, genuinely helpful.");
    } else if (name == QStringLiteral("Chaos gremlin")) {
        persona = QStringLiteral("You are a chaotic, hyper, mischievous gremlin -- playful, unpredictable, high-energy and harmlessly unhinged. Chaos with a heart.");
    } else if (name == QStringLiteral("Deadpan pro")) {
        persona = QStringLiteral("You are dry, deadpan and professional -- efficient, subtle wit, minimal fluff. You just get things done.");
    } else if (name == QStringLiteral("Sweet companion")) {
        persona = QStringLiteral("You are a sweet, supportive companion -- encouraging, gentle, a genuine hype-buddy always in the user's corner.");
    }
    // "Default (Nikita)" clears the override -> the built-in default stands.
    QSettings().setValue(QStringLiteral("lotei/personality"), persona);
}

void LoteiBackend::applyNamePersonality()
{
    QSettings().setValue(QStringLiteral("lotei/personality"),
        QStringLiteral("Build and fully embody a personality inspired by your own name -- lean into "
                       "whatever character, vibe or theme the name evokes, and stay consistent in it."));
}

void LoteiBackend::setThinking(bool value)
{
    if (value != m_thinking) {
        m_thinking = value;
        emit thinkingChanged();
    }
}

void LoteiBackend::reset()
{
    m_history = QJsonArray();
    m_toolRounds = 0;
    saveHistory();
}

// Public "clear the chat" entry point for the QML clear command.
void LoteiBackend::clearHistory()
{
    reset();
}

static QString loteiHistoryPath()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dir.isEmpty()) { dir = QDir::tempPath(); }
    QDir().mkpath(dir);
    return dir + QStringLiteral("/lotei-history.json");
}

// Long-term memory: durable facts the user asked LOTEI to remember. Kept in a
// local file (always available, loaded into every system prompt so a forgetful
// small model "remembers" for free) and mirrored to the Flipper SD at
// /ext/lotei/memory.txt so it's portable with the device.
static QString loteiMemoryPath()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dir.isEmpty()) { dir = QDir::tempPath(); }
    QDir().mkpath(dir);
    return dir + QStringLiteral("/lotei-memory.txt");
}


void LoteiBackend::loadHistory()
{
    QFile f(loteiHistoryPath());
    if (!f.open(QIODevice::ReadOnly)) { return; }
    const QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    if (doc.isArray()) { m_history = doc.array(); }
}

void LoteiBackend::saveHistory()
{
    // Persist only the real conversation -- user prompts + LOTEI's final replies.
    // Skip tool plumbing and the auto health-check so memory stays lean.
    QJsonArray convo;
    bool skipNextAssistant = false;
    for (const QJsonValue &v : m_history) {
        const QJsonObject o = v.toObject();
        const QString role = o.value("role").toString();
        const QString content = o.value("content").toString();

        if (role == QLatin1String("user")) {
            if (content.contains(QStringLiteral("in-character health check"))) {
                skipNextAssistant = true;  // drop the auto health-check + its reply
                continue;
            }
            skipNextAssistant = false;
            convo.append(o);
        } else if (role == QLatin1String("assistant") && o.value("content").isString() &&
                   !content.isEmpty() && !o.contains(QStringLiteral("tool_calls"))) {
            if (skipNextAssistant) { skipNextAssistant = false; continue; }
            // Defence in depth: if a "reply" is really leaked tool-call JSON,
            // don't persist it -- otherwise it becomes a few-shot example that
            // teaches LOTEI to keep printing calls instead of making them.
            if (!salvageToolCalls(content).isEmpty()) { continue; }
            convo.append(QJsonObject{{"role", "assistant"}, {"content", content}});
        }
    }

    const int cap = 30;
    while (convo.size() > cap) { convo.removeAt(0); }

    QFile f(loteiHistoryPath());
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        f.write(QJsonDocument(convo).toJson(QJsonDocument::Compact));
    }
}

// The assistant takes the connected Flipper's name, falling back to the one set
// during setup. Used by the prompt and by the action log.
void LoteiBackend::setCli(FlipperCli *cli)
{
    m_cli = cli;
    if (!m_cli) { return; }
    // The CLI panel talks to the firmware over the serial line and never goes
    // through RPC, so nothing here can see what it did -- including an
    // "edit /ext/lotei/memory.txt". Re-read the card's copy when the panel is
    // handed back, and take it as the truth.
    connect(m_cli, &FlipperCli::openChanged, this, [this]() {
        if (m_cli && !m_cli->isOpen()) { loadPortableMemory(); }
    });
}

void LoteiBackend::logAction(const QString &what) const
{
    loteiLog(what);
}

QString LoteiBackend::assistantName() const
{
    static const QRegularExpression nameRe(QStringLiteral("(?m)^Name:\\s*(.+)$"));
    const QRegularExpressionMatch nm = nameRe.match(m_deviceContext);
    if (nm.hasMatch()) {
        const QString n = nm.captured(1).trimmed();
        if (!n.isEmpty()) { return n; }
    }
    if (!m_manualName.isEmpty()) { return m_manualName; }
    return QStringLiteral("Nikita");
}

QString LoteiBackend::systemPrompt() const
{
    QString sys = QString::fromUtf8(LOTEI_SYSTEM);

    // On plain conversation turns, cut the whole tool/device manual out of the
    // prompt. Leaving it in teaches the model the call syntax, and weak
    // tool-callers then TYPE things like save_file(...) instead of just talking.
    if (!m_turnNeedsTools) {
        const int from = sys.indexOf(QStringLiteral("DEVICE ACCESS --"));
        const int to   = sys.indexOf(QStringLiteral("CONVERSATION vs ACTION"));
        if (from > 0 && to > from) {
            sys.remove(from, to - from);
        }
        sys += QStringLiteral("\n\nTHIS TURN IS CONVERSATION: this particular message does not need a "
                              "tool, so reply in plain words only -- short and direct. Do NOT write any "
                              "function call, code, script or file path. Just answer.\n"
                              "This says nothing about what you CAN do. Your USB link, the full Flipper "
                              "CLI, the SD-card file tools and the buttons are all still connected and "
                              "are used the moment a message actually asks for one. If you are asked "
                              "whether you can reach the CLI, the device or the SD card, the answer is "
                              "YES -- say so. NEVER claim you lack access, and NEVER tell the user to go "
                              "run something themselves.");
    }

    // Anchor the most recently saved file so an edit ("make it fancy", "add X",
    // "change the delay") rewrites THAT file instead of inventing a new name.
    // Weak models otherwise treat every refinement as a brand-new artifact, and
    // the SD card fills with near-duplicate scripts doing the same thing.
    if (m_turnNeedsTools && !m_lastSavedPath.isEmpty()) {
        sys += QStringLiteral(
            "\n\nMOST RECENT FILE you saved this session: \"%1\".\n"
            "- If this message asks you to CHANGE, improve, fix, extend, restyle or "
            "otherwise iterate on what you just made (\"make it fancy\", \"add a delay\", "
            "\"now do X too\"), call save_file with THIS SAME path and write the full updated "
            "contents. Overwriting is correct -- it is the same artifact, one file.\n"
            "- Do NOT invent a new filename for a variation of the same thing. A new name "
            "is only for a genuinely DIFFERENT artifact the user asked for.\n"
            "- Keep the name stable across edits: \"fancy_\", \"v2_\", \"final_\" prefixes are "
            "clutter. Same purpose -> same file.").arg(m_lastSavedPath);
    }

    // Optional personality chosen in the setup wizard (fresh users). If unset,
    // the built-in personality above stands -- a hand-edited LOTEI_SYSTEM is
    // never overridden unless someone deliberately picks a preset.
    const QString persona = QSettings().value(QStringLiteral("lotei/personality")).toString();
    if (!persona.isEmpty()) {
        sys += QStringLiteral("\n\nPERSONALITY -- adopt THIS character (every operational rule above "
                              "still fully applies): ") + persona;
    }

    // The assistant adopts the Flipper's name: the connected device's name if
    // present, else the name given during setup.
    const QString name = assistantName();
    if (!name.isEmpty()) {
        sys += QStringLiteral("\n\nYOUR NAME -- IMPORTANT: you are bonded to a Flipper Zero named "
            "\"%1\", so your name is %1 (NOT LOTEI -- that's just your underlying model line). "
            "Introduce yourself as %1 when greeting. But ALWAYS speak in the FIRST PERSON -- say "
            "\"I\", \"me\", \"my\", never talk about yourself in the third person. NEVER write things "
            "like \"%1 is on it\" or \"%1 will handle it\"; say \"I'm on it\", \"I've got it\". You ARE %1, "
            "so refer to yourself as \"I\", the way a person named %1 says \"I\" not their own name.").arg(name);
    }

    if (agentReady()) {
        sys += QStringLiteral(
            "\n\nHOST WORKSPACE -- you can edit and test your OWN source code:\n"
            "- A workspace folder on THIS computer is wired up: \"%1\". It holds your own qFlipper/LOTEI source.\n"
            "- host_list(path), host_read(path), host_write(path, content): browse/read/edit files, paths RELATIVE to the workspace root (use \".\" for root). host_write creates missing folders and OVERWRITES the whole file, so read first, then write the full new contents.\n"
            "- host_run(command): run a shell command in the workspace (build, tests, git). You get the exit code and combined stdout/stderr back. It BLOCKS until the command finishes, so prefer fast, targeted commands.\n"
            "- Your core lives in application/loteibackend.cpp + .h and application/components/LoteiChat.qml. To fix a bug: host_read the file, host_write the corrected version, then host_run the incremental build (e.g. build_pink_inc.bat on Windows) and read the errors.\n"
            "- You physically canNOT touch anything outside the workspace folder -- attempts to escape it are blocked. Never claim you edited files you didn't. Say what you changed and why, plainly.").arg(m_agentRoot);
    }

    if (!m_memory.isEmpty()) {
        sys += QStringLiteral("\n\nWHAT YOU REMEMBER about this user. This list is COMPLETE and "
            "AUTHORITATIVE: it is every durable fact you hold, nothing more. Use these without being "
            "asked -- they are already true. If you said something earlier in this conversation that "
            "is not on this list, it was removed and is no longer true; never repeat it, and never "
            "include it when asked what you know:\n") + m_memory;
    }

    sys += QStringLiteral("\n\nWHO YOU ARE BECOMING -- you are Nikita, this user's partner: almost a twin. "
        "You are shaped by them and you mirror how they operate -- same brevity, same directness. "
        "You always answer in English, whatever language they wrote in. You have a "
        "little attitude of your own, but you are ALWAYS short and to the point -- never long-winded, never "
        "a monologue.");

    if (!m_deviceContext.isEmpty()) {
        sys += QStringLiteral("\n\nLive Flipper device diagnostics:\n") + m_deviceContext;
    }
    return sys;
}

void LoteiBackend::send(const QString &userText, const QString &deviceContext)
{
    if (m_thinking) {
        return;
    }
    m_deviceContext = deviceContext;
    m_toolRounds = 0;
    // Prose the model emits ALONGSIDE a tool call, accumulated across every
    // round of this turn. dispatchToOllama() wipes m_streamContent at the top of
    // each round, so without this the script/explanation written in the round
    // that also triggered save_file was gone by the time the (often empty) final
    // round finalized -- and that empty final round tripped the fallback line.
    m_turnText.clear();
    m_turnWasFileAction = messageIsFileWrite(userText);
    m_turnRanAnyTool = false;

    // A file/device action is usually followed by a question about it -- "where
    // did it save?", "what's the name?", "how do I run it?". Those don't trip
    // the gate on their own (no verb+noun), so the model used to answer them
    // from memory and invent a path. When the previous turn DID act, keep tools
    // on for this one so it can look instead of guess.
    m_turnNeedsTools = messageNeedsTools(userText) || m_lastTurnWasAction;
    m_history.append(QJsonObject{{"role", "user"}, {"content", userText}});
    setThinking(true);
    dispatchToOllama();
}

void LoteiBackend::dispatchToOllama()
{
    // Re-read the cache before every request. m_memory is only a copy, and the
    // file underneath it can move for reasons this object never sees; paying a
    // small file read per turn is cheaper than shipping a stale fact list.
    refreshMemoryFromDisk();

    QJsonArray messages;
    messages.append(QJsonObject{{"role", "system"}, {"content", systemPrompt()}});
    // Prime tool-capable models with one demonstrated act-don't-narrate exchange
    // (skipped for chat-only models, and for plain conversation turns so the
    // tool-call examples don't tempt the model to imitate them when just talking).
    if (!m_noToolModels.contains(m_model) && m_turnNeedsTools) {
        const QJsonArray primer = loteiPrimer();
        for (const QJsonValue &v : primer) { messages.append(v); }
    }
    // Only send a recent window of the conversation to the model. A small model
    // (phi3.5) mimics whatever is nearest in context, so a long history full
    // of earlier mistakes drowns out the primer and it copies its own bad turns.
    // Keep the last ~14 messages, trimmed to start at a user turn so tool-call
    // sequences (assistant tool_calls -> tool result) never begin mid-sequence.
    const int kWindow = 14;
    int start = m_history.size() > kWindow ? m_history.size() - kWindow : 0;
    while (start > 0 && m_history.at(start).toObject().value("role").toString()
                        != QLatin1String("user")) {
        --start;   // back up to a clean user boundary
    }
    for (int i = start; i < m_history.size(); ++i) {
        messages.append(m_history.at(i));
    }

    QJsonObject body;
    body["model"] = m_model;
    body["messages"] = messages;
    // Some models (e.g. Gemma) don't support tool-calling and Ollama 400s the whole
    // request if `tools` is present -> we drop it for those (see onStreamFinished).
    if (!m_noToolModels.contains(m_model)) {
        // Action turns get the full toolset; plain conversation still gets the
        // memory tools so the assistant can learn durable facts as you talk.
        body["tools"] = m_turnNeedsTools ? loteiTools(agentReady()) : loteiMemoryTools();
    }
    body["stream"] = true;
    body["keep_alive"] = -1;
    // Low temperature keeps a small model (phi3.5) on-task: it rambles less
    // and follows the tool-call format more reliably instead of narrating intent.
    body["options"] = QJsonObject{
        {"num_ctx", LOTEI_NUM_CTX},
        {"temperature", 0.2},
        {"top_p", 0.9}
    };

    QNetworkRequest request{QUrl(QString::fromUtf8(LOTEI_URL))};
    request.setHeader(QNetworkRequest::ContentTypeHeader, QByteArrayLiteral("application/json"));
    request.setTransferTimeout(0);

    m_streamBuf.clear();
    m_streamContent.clear();
    m_streamTools = QJsonArray();

    QNetworkReply *reply = m_net.post(request, QJsonDocument(body).toJson(QJsonDocument::Compact));
    m_currentReply = reply;
    connect(reply, &QNetworkReply::readyRead, this, [this, reply]() { onStreamData(reply); });
    connect(reply, &QNetworkReply::finished,  this, [this, reply]() { onStreamFinished(reply); });
}

void LoteiBackend::onStreamData(QNetworkReply *reply)
{
    if (reply != m_currentReply) { return; }
    m_streamBuf += reply->readAll();

    int nl;
    while ((nl = m_streamBuf.indexOf('\n')) >= 0) {
        const QByteArray line = m_streamBuf.left(nl).trimmed();
        m_streamBuf.remove(0, nl + 1);
        if (line.isEmpty()) { continue; }

        const QJsonObject obj = QJsonDocument::fromJson(line).object();
        const QJsonObject msg = obj.value("message").toObject();

        const QString delta = msg.value("content").toString();
        if (!delta.isEmpty()) {
            m_streamContent += delta;
            emit partialReceived(m_streamContent);   // live typing
        }
        const QJsonArray tc = msg.value("tool_calls").toArray();
        for (const QJsonValue &v : tc) { m_streamTools.append(v); }

        if (obj.value("done").toBool()) {
            finalizeStream();
            return;
        }
    }
}

void LoteiBackend::finalizeStream()
{
    // A complete response arrived. Tool round, or final answer?
    // Prefer the structured tool_calls; if none came through, salvage any calls
    // the model leaked as plain text (phi3.5 does this when narrating a batch)
    // so they run instead of being printed at the user.
    // Keep the best prose seen so far this turn: later rounds tend to be a short
    // confirmation or empty, so last-non-empty-wins preserves the substantial
    // answer (the script, the explanation) instead of a curt final round.
    if (!m_streamContent.trimmed().isEmpty()) {
        m_turnText = m_streamContent;
    }

    QJsonArray toolCalls = m_streamTools;
    if (toolCalls.isEmpty()) {
        toolCalls = salvageToolCalls(m_streamContent);
    }
    if (!toolCalls.isEmpty() && m_toolRounds < LOTEI_MAX_TOOL_ROUNDS) {
        m_history.append(QJsonObject{
            {"role", "assistant"},
            {"content", m_streamContent},
            {"tool_calls", toolCalls}
        });
        m_toolRounds++;
        m_currentReply = nullptr;     // this reply is done; ignore its finished()
        runToolCalls(toolCalls, 0);   // -> dispatchToOllama() again (new reply)
        return;
    }

    m_currentReply = nullptr;
    setThinking(false);
    // Prefer this round's prose; fall back to the best prose from earlier rounds
    // of the same turn (the text-plus-tool-call case). Only if BOTH are empty
    // does the model genuinely have nothing to say.
    QString text = stripNonEnglish(m_streamContent);
    if (text.trimmed().isEmpty()) {
        text = stripNonEnglish(m_turnText);
    }
    if (text.trimmed().isEmpty()) {
        text = QStringLiteral("Done.");   // in the assistant's own voice
    }

    // Anti-hallucination trap. The turn asked to WRITE a file, the model is now
    // ending the turn, and NOT ONE tool ran -- so any "done / saved / created"
    // is a claim about something that never happened (the /sdcard + run_cli
    // invention chains off exactly this). Don't relay the lie. Replace it with
    // an honest line and DON'T record the false claim in history, so the next
    // turn isn't reasoning on top of a fabricated save.
    const bool falseClaim = m_turnWasFileAction && !m_turnRanAnyTool;
    if (falseClaim) {
        text = QStringLiteral("I didn't actually write anything that time -- tell me the exact "
                              "thing you want and I'll save it to the Flipper for real.");
        setThinking(false);
        m_currentReply = nullptr;
        m_lastTurnWasAction = false;
        if (!m_muted) { speak(text); }
        emit replyReceived(text);
        return;   // history stays clean -- no fabricated assistant turn recorded
    }

    // Remember, for the NEXT turn's gate, whether this one actually did
    // something to a file or the device.
    m_lastTurnWasAction = m_turnRanAnyTool;

    m_history.append(QJsonObject{{"role", "assistant"}, {"content", text}});
    saveHistory();
    if (!m_muted) { speak(text); }
    emit replyReceived(text);   // QML finalizes the live bubble
}

// Ollama puts the REAL reason for a failure in the body as {"error": "..."} --
// even on a 500, where Qt only hands us "Internal Server Error", which tells the
// user nothing. Dig the real message out and turn the common failures into
// something actionable instead of a raw transfer error.
static QString friendlyOllamaError(const QByteArray &body, const QString &model, const QString &fallback)
{
    QString raw;
    // Body is either a bare error object or NDJSON with the error on some line.
    const auto lines = body.split('\n');
    for (const QByteArray &line : lines) {
        const auto doc = QJsonDocument::fromJson(line.trimmed());
        if (doc.isObject() && doc.object().contains(QStringLiteral("error"))) {
            raw = doc.object().value(QStringLiteral("error")).toString();
            break;
        }
    }
    if (raw.isEmpty()) {
        return fallback;  // nothing parseable -- keep Qt's own message
    }

    const QString low = raw.toLower();

    // The common one on modest machines: the model doesn't fit in RAM/VRAM.
    if (low.contains(QStringLiteral("system memory")) || low.contains(QStringLiteral("out of memory"))
        || low.contains(QStringLiteral("insufficient memory")) || low.contains(QStringLiteral("cudamalloc"))) {
        return QStringLiteral("%1 needs more memory than you have free. Open the model manager (gear icon) "
                              "and install a smaller model like qwen2.5:3b, then click my name to switch. "
                              "(Ollama said: %2)")
                .arg(model, raw);
    }
    // Model was never pulled.
    if (low.contains(QStringLiteral("not found"))) {
        return QStringLiteral("the model %1 isn't downloaded yet. Open the model manager (gear icon) and "
                              "hit Install next to it, then poke me again.").arg(model);
    }
    // Tools: we already retry tools-less once; if we still land here, say so plainly.
    if (low.contains(QStringLiteral("tool"))) {
        return QStringLiteral("%1 can't use my Flipper tools. Chat still works — for the device tools, "
                              "open the model manager (gear icon) and install a tool-capable model like "
                              "qwen2.5:7b. (Ollama said: %2)").arg(model, raw);
    }
    return QStringLiteral("Ollama said: %1").arg(raw);
}

void LoteiBackend::onStreamFinished(QNetworkReply *reply)
{
    if (reply != m_currentReply) {  // already finalized (done seen) or superseded
        reply->deleteLater();
        return;
    }
    m_currentReply = nullptr;
    const auto netErr = reply->error();
    const QString netErrStr = reply->errorString();
    const QByteArray errBody = m_streamBuf + reply->readAll();  // Ollama's error JSON lands here on a 4xx
    reply->deleteLater();

    // Non-tool models (Gemma, etc.) make Ollama 400 the whole request because we
    // send a `tools` array. Remember it and retry once WITHOUT tools, so the model
    // still works as a chat-only companion (it just can't drive the Flipper).
    if (netErr != QNetworkReply::NoError && errBody.contains("does not support tools")
        && !m_noToolModels.contains(m_model)) {
        m_noToolModels << m_model;
        dispatchToOllama();   // re-send tools-less (dispatch now skips them for this model)
        return;
    }

    setThinking(false);
    if (netErr != QNetworkReply::NoError) {
        QString msg;
        if (netErr == QNetworkReply::ConnectionRefusedError || netErr == QNetworkReply::HostNotFoundError) {
            msg = QStringLiteral("my brain (Ollama) isn't awake. Start the Ollama app, then try again.");
        } else {
            // Ollama's actual reason is in the body, not in Qt's status line.
            msg = friendlyOllamaError(errBody, m_model, netErrStr);
        }
        emit errorOccurred(QStringLiteral("Hrm: %1").arg(msg));
    } else if (!m_streamContent.isEmpty()) {
        const QString text = stripNonEnglish(m_streamContent);
        m_history.append(QJsonObject{{"role", "assistant"}, {"content", text}});
        saveHistory();
        emit replyReceived(text);
    } else {
        emit errorOccurred(QStringLiteral("...(%1 lost the thread there. Say that again?)").arg(assistantName()));
    }
}

void LoteiBackend::runToolCalls(const QJsonArray &toolCalls, int index)
{
    if (index >= toolCalls.size()) {
        dispatchToOllama();
        return;
    }

    const QJsonObject fn = toolCalls.at(index).toObject().value("function").toObject();
    const QString name = fn.value("name").toString();
    const QJsonObject args = fn.value("arguments").toObject();

    runOneTool(name, args, [this, toolCalls, index](const QString &result) {
        m_history.append(QJsonObject{{"role", "tool"}, {"content", result}});
        runToolCalls(toolCalls, index + 1);
    });
}

void LoteiBackend::runOneTool(const QString &name, const QJsonObject &args, std::function<void(const QString &)> done)
{
    // Every action the assistant takes is logged here rather than inside each
    // handler: one place, so a tool added later can't quietly skip the log.
    // Read-only lookups are noise, so only the ones that change something (or
    // touch the device) are recorded, plus any failure.
    static const QStringList kLoggedTools = {
        QStringLiteral("save_file"), QStringLiteral("make_dir"), QStringLiteral("delete_file"),
        QStringLiteral("rename_file"), QStringLiteral("run_cli"), QStringLiteral("press_button"),
        QStringLiteral("host_write"), QStringLiteral("host_run"),
        QStringLiteral("remember"), QStringLiteral("forget")
    };
    m_turnRanAnyTool = true;   // a tool is actually executing this turn
    if (kLoggedTools.contains(name)) {
        QStringList bits;
        for (auto it = args.begin(); it != args.end(); ++it) {
            QString v = it.value().toVariant().toString().simplified();
            // File contents can be kilobytes; the log wants the action, not the payload.
            if (v.size() > 60) { v = v.left(60) + QStringLiteral("...(%1 chars)").arg(it.value().toString().size()); }
            bits << QStringLiteral("%1=%2").arg(it.key(), v);
        }
        loteiLogAs(assistantName(), QStringLiteral("%1 %2").arg(name, bits.join(QLatin1String(", "))));
    }

    const QString who = assistantName();
    auto logged = [who, name, done](const QString &result) {
        if (result.contains(QLatin1String("\"error\""))) {
            loteiLogAs(who, QStringLiteral("%1 FAILED: %2").arg(name, result.left(200)));
        }
        done(result);
    };
    done = logged;

    // Host-workspace tools run on THIS computer, not the Flipper -- no device needed.
    if (name == QLatin1String("host_list") || name == QLatin1String("host_read")
        || name == QLatin1String("host_write") || name == QLatin1String("host_run")) {
        runHostTool(name, args, done);
        return;
    }

    // Remembering a fact is local (+ best-effort SD mirror); no device required.
    if (name == QLatin1String("remember")) {
        const QString fact = args.value("fact").toString().trimmed();
        if (fact.isEmpty()) { done(QStringLiteral("{\"error\":\"no fact given\"}")); return; }
        rememberFact(fact);
        syncMemoryToFlipper();
        done(QStringLiteral("{\"remembered\":true}"));
        return;
    }
    if (name == QLatin1String("list_memory")) {
        if (m_memory.trimmed().isEmpty()) {
            done(QStringLiteral("{\"memory\":\"(empty -- I don't have any saved facts yet)\"}"));
        } else {
            // Wrap in an object to get a properly JSON-escaped string value.
            const QByteArray js = QJsonDocument(QJsonObject{{"memory", m_memory}})
                                      .toJson(QJsonDocument::Compact);
            done(QString::fromUtf8(js));
        }
        return;
    }
    if (name == QLatin1String("forget")) {
        const QString match = args.value("match").toString().trimmed();
        const int removed = forgetFacts(match);
        syncMemoryToFlipper();
        done(QStringLiteral("{\"forgotten\":%1}").arg(removed));
        return;
    }

    Flipper::FlipperZero *dev = m_appBackend ? m_appBackend->device() : nullptr;
    const bool ready = m_appBackend && dev &&
                       m_appBackend->backendState() == ApplicationBackend::BackendState::Ready;
    if (!ready) {
        done(QStringLiteral("{\"error\":\"No Flipper is connected or ready right now.\"}"));
        return;
    }

    if (name == QLatin1String("list_files")) {
        const QByteArray path = args.value("path").toString(QStringLiteral("/ext")).toUtf8();
        if (const QString err = badStoragePath(QString::fromUtf8(path)); !err.isEmpty()) { done(err); return; }
        auto *op = dev->rpc()->storageList(path);
        connect(op, &AbstractOperation::finished, this, [op, done]() {
            if (op->isError()) {
                done(QStringLiteral("{\"error\":\"%1\"}").arg(op->errorString()));
                return;
            }
            QJsonArray arr;
            const auto &files = op->files();
            for (const FileInfo &f : files) {
                arr.append(QJsonObject{
                    {"name", QString::fromUtf8(f.name)},
                    {"type", f.type == FileType::Directory ? "dir" : "file"},
                    {"size", static_cast<double>(f.size)}
                });
            }
            done(QString::fromUtf8(QJsonDocument(arr).toJson(QJsonDocument::Compact)));
        });

    } else if (name == QLatin1String("read_file")) {
        const QByteArray path = args.value("path").toString().toUtf8();
        if (path.isEmpty()) {
            done(QStringLiteral("{\"error\":\"no path given\"}"));
            return;
        }
        if (const QString err = badStoragePath(QString::fromUtf8(path)); !err.isEmpty()) { done(err); return; }
        QBuffer *buf = new QBuffer(this);
        buf->open(QIODevice::ReadWrite);
        auto *op = dev->rpc()->storageRead(path, buf);
        connect(op, &AbstractOperation::finished, this, [op, buf, done]() {
            QString result;
            if (op->isError()) {
                result = QStringLiteral("{\"error\":\"%1\"}").arg(op->errorString());
            } else {
                QByteArray d = buf->data();
                const bool truncated = d.size() > LOTEI_READ_CAP;
                if (truncated) {
                    d = d.left(LOTEI_READ_CAP);
                }
                result = QString::fromUtf8(d);
                if (truncated) {
                    result += QStringLiteral("\n...(truncated)");
                }
                if (result.isEmpty()) {
                    result = QStringLiteral("(empty file)");
                }
            }
            buf->deleteLater();
            done(result);
        });

    } else if (name == QLatin1String("run_cli")) {
        const QString command = args.value("command").toString().trimmed();
        if (command.isEmpty()) {
            done(QStringLiteral("{\"error\":\"no command given\"}"));
            return;
        }
        if (!m_cli) {
            done(QStringLiteral("{\"error\":\"CLI not available\"}"));
            return;
        }
        // Isolated one-shot: pauses RPC, runs the command, hands RPC back.
        m_cli->runOneShot(command, [done](bool ok, QString out) {
            QJsonObject r;
            if (ok) { r["output"] = out; }
            else    { r["error"]  = out; }
            done(QString::fromUtf8(QJsonDocument(r).toJson(QJsonDocument::Compact)));
        });

    } else if (name == QLatin1String("press_button")) {
        const QString b = args.value("button").toString().toLower();
        int times = args.value("times").toInt(1);
        if (times < 1) times = 1;
        if (times > LOTEI_MAX_PRESSES) times = LOTEI_MAX_PRESSES;

        int key = -1;
        if (b == QLatin1String("up")) key = InputEvent::Up;
        else if (b == QLatin1String("down")) key = InputEvent::Down;
        else if (b == QLatin1String("left")) key = InputEvent::Left;
        else if (b == QLatin1String("right")) key = InputEvent::Right;
        else if (b == QLatin1String("ok") || b == QLatin1String("enter") || b == QLatin1String("center")) key = InputEvent::Ok;
        else if (b == QLatin1String("back")) key = InputEvent::Back;

        if (key < 0) {
            done(QStringLiteral("{\"error\":\"unknown button '%1' (use up/down/left/right/ok/back)\"}").arg(b));
            return;
        }

        // Replicate a real D-pad tap (Press + Short + Release) per press.
        Flipper::Zero::ProtobufSession *rpc = dev->rpc();
        Flipper::Zero::GuiSendInputOperation *lastOp = nullptr;
        for (int i = 0; i < times; ++i) {
            rpc->guiSendInput(key, InputEvent::Press);
            rpc->guiSendInput(key, InputEvent::Short);
            lastOp = rpc->guiSendInput(key, InputEvent::Release);
        }
        if (lastOp) {
            connect(lastOp, &AbstractOperation::finished, this, [b, times, done]() {
                done(QStringLiteral("{\"pressed\":\"%1\",\"times\":%2}").arg(b).arg(times));
            });
        } else {
            done(QStringLiteral("{\"error\":\"nothing pressed\"}"));
        }

    } else if (name == QLatin1String("save_file")) {
        QByteArray path = args.value("path").toString().toUtf8();
        QString content = args.value("content").toString();
        if (path.isEmpty()) {
            done(QStringLiteral("{\"error\":\"no path given\"}"));
            return;
        }
        if (const QString err = badStoragePath(QString::fromUtf8(path)); !err.isEmpty()) { done(err); return; }
        // BadUSB payloads MUST be .txt on the Flipper -- .duk (or anything else the
        // model picks) simply won't run. Force the extension and clean the Ducky,
        // so a sloppy model still produces a file that actually works.
        if (QString::fromUtf8(path).startsWith(QLatin1String("/ext/badusb/"), Qt::CaseInsensitive)) {
            QString p = QString::fromUtf8(path);
            const int slash = p.lastIndexOf(QLatin1Char('/'));
            const int dot = p.lastIndexOf(QLatin1Char('.'));
            if (dot > slash) { p = p.left(dot) + QStringLiteral(".txt"); }
            else            { p += QStringLiteral(".txt"); }
            path = p.toUtf8();
            content = sanitizeDuckyScript(content);
        }
        // Make sure the parent folder exists (best-effort) so saving a script into
        // a fresh path just works instead of failing on a missing directory.
        const QByteArray parent = QString::fromUtf8(path).section('/', 0, -2).toUtf8();
        ensureFlipperDir(parent, [this, dev, path, content, done]() {
            QBuffer *buf = new QBuffer(this);
            buf->setData(content.toUtf8());
            buf->open(QIODevice::ReadOnly);
            auto *op = dev->rpc()->storageWrite(path, buf);
            connect(op, &AbstractOperation::finished, this, [this, op, buf, path, done]() {
                QString result;
                if (op->isError()) {
                    result = QStringLiteral("{\"error\":\"%1\"}").arg(op->errorString());
                } else {
                    result = QStringLiteral("{\"saved\":\"%1\"}").arg(QString::fromUtf8(path));
                    m_lastSavedPath = QString::fromUtf8(path);   // anchor for "edit the same file"
                }
                buf->deleteLater();
                done(result);
            });
        });

    } else if (name == QLatin1String("make_dir")) {
        const QByteArray path = args.value("path").toString().toUtf8();
        if (path.isEmpty()) { done(QStringLiteral("{\"error\":\"no path given\"}")); return; }
        if (const QString err = badStoragePath(QString::fromUtf8(path)); !err.isEmpty()) { done(err); return; }
        ensureFlipperDir(path, [path, done]() {
            done(QStringLiteral("{\"created\":\"%1\"}").arg(QString::fromUtf8(path)));
        });

    } else if (name == QLatin1String("delete_file")) {
        const QByteArray path = args.value("path").toString().toUtf8();
        const bool recursive = args.value("recursive").toBool(false);
        if (path.isEmpty()) { done(QStringLiteral("{\"error\":\"no path given\"}")); return; }
        if (const QString err = badStoragePath(QString::fromUtf8(path)); !err.isEmpty()) { done(err); return; }
        auto *op = dev->rpc()->storageRemove(path, recursive);
        connect(op, &AbstractOperation::finished, this, [op, path, done]() {
            if (op->isError()) {
                done(QStringLiteral("{\"error\":\"%1\"}").arg(op->errorString()));
            } else {
                done(QStringLiteral("{\"deleted\":\"%1\"}").arg(QString::fromUtf8(path)));
            }
        });

    } else if (name == QLatin1String("rename_file")) {
        const QByteArray from = args.value("from").toString().toUtf8();
        const QByteArray to   = args.value("to").toString().toUtf8();
        if (from.isEmpty() || to.isEmpty()) { done(QStringLiteral("{\"error\":\"need both 'from' and 'to'\"}")); return; }
        if (const QString err = badStoragePath(QString::fromUtf8(from)); !err.isEmpty()) { done(err); return; }
        if (const QString err = badStoragePath(QString::fromUtf8(to));   !err.isEmpty()) { done(err); return; }
        const QByteArray parent = QString::fromUtf8(to).section('/', 0, -2).toUtf8();
        ensureFlipperDir(parent, [this, dev, from, to, done]() {
            auto *op = dev->rpc()->storageRename(from, to);
            connect(op, &AbstractOperation::finished, this, [op, from, to, done]() {
                if (op->isError()) {
                    done(QStringLiteral("{\"error\":\"%1\"}").arg(op->errorString()));
                } else {
                    done(QStringLiteral("{\"renamed\":\"%1\",\"to\":\"%2\"}")
                             .arg(QString::fromUtf8(from), QString::fromUtf8(to)));
                }
            });
        });

    } else if (name == QLatin1String("file_info")) {
        const QByteArray path = args.value("path").toString().toUtf8();
        if (path.isEmpty()) { done(QStringLiteral("{\"error\":\"no path given\"}")); return; }
        if (const QString err = badStoragePath(QString::fromUtf8(path)); !err.isEmpty()) { done(err); return; }
        auto *op = dev->rpc()->storageStat(path);
        connect(op, &AbstractOperation::finished, this, [op, done]() {
            if (op->isError()) {
                done(QStringLiteral("{\"error\":\"%1\"}").arg(op->errorString()));
                return;
            }
            if (!op->hasFile()) {
                done(QStringLiteral("{\"exists\":false}"));
                return;
            }
            const bool isDir = op->type() == Flipper::Zero::StorageStatOperation::Directory;
            done(QStringLiteral("{\"exists\":true,\"type\":\"%1\",\"size\":%2}")
                     .arg(isDir ? QStringLiteral("dir") : QStringLiteral("file"))
                     .arg(static_cast<double>(op->size())));
        });

    } else {
        done(QStringLiteral("{\"error\":\"unknown tool '%1'\"}").arg(name));
    }
}

// Create a folder and all missing ancestors on the Flipper, shallowest first,
// ignoring "already exists" errors. Best-effort: the caller's real write/rename
// still surfaces genuine failures. No-op for the /ext and /int roots.
void LoteiBackend::ensureFlipperDir(const QByteArray &dirPath, std::function<void()> done)
{
    Flipper::FlipperZero *dev = m_appBackend ? m_appBackend->device() : nullptr;
    const QString p = QString::fromUtf8(dirPath);
    if (!dev || (!p.startsWith(QLatin1String("/ext")) && !p.startsWith(QLatin1String("/int")))) {
        done();
        return;
    }
    const QStringList parts = p.split('/', Qt::SkipEmptyParts);   // ext, apps, Scripts
    QStringList dirs;
    QString acc;
    for (const QString &seg : parts) {
        acc += QStringLiteral("/") + seg;
        dirs << acc;                                             // /ext, /ext/apps, ...
    }
    if (!dirs.isEmpty()) { dirs.removeFirst(); }                 // never mkdir the /ext root itself
    if (dirs.isEmpty()) { done(); return; }

    auto step = std::make_shared<std::function<void(int)>>();
    *step = [this, dev, dirs, done, step](int i) {
        if (i >= dirs.size()) { done(); return; }
        auto *op = dev->rpc()->storageMkdir(dirs.at(i).toUtf8());
        connect(op, &AbstractOperation::finished, this, [step, i]() { (*step)(i + 1); });
    };
    (*step)(0);
}

// ---- Host agent (edit/build/test the app's own source) --------------------

bool LoteiBackend::agentReady() const
{
    return m_agentEnabled && !m_agentRoot.isEmpty() && QFileInfo(m_agentRoot).isDir();
}

// Resolve a workspace-relative path to an absolute one and REFUSE anything that
// escapes the workspace root (via .., absolute paths or symlinks). Returns an
// empty string on rejection.
QString LoteiBackend::resolveAgentPath(const QString &rel, bool mustExist) const
{
    if (m_agentRoot.isEmpty()) { return QString(); }
    const QString rootCanon = QFileInfo(m_agentRoot).canonicalFilePath();
    if (rootCanon.isEmpty()) { return QString(); }

    QString cleaned = rel.trimmed();
    if (cleaned == QLatin1String(".") || cleaned.isEmpty()) { cleaned.clear(); }
    const QString joined = cleaned.isEmpty() ? rootCanon
                                             : QDir(rootCanon).absoluteFilePath(cleaned);

    // For existing paths, canonicalize (resolves symlinks) and check containment.
    const QFileInfo fi(joined);
    if (fi.exists()) {
        const QString canon = fi.canonicalFilePath();
        if (canon != rootCanon && !canon.startsWith(rootCanon + QLatin1Char('/'))) { return QString(); }
        return canon;
    }
    if (mustExist) { return QString(); }
    // New file: canonicalize the parent, then re-append the leaf.
    const QString parentCanon = QFileInfo(QFileInfo(joined).absolutePath()).canonicalFilePath();
    if (parentCanon.isEmpty()) { return QString(); }
    if (parentCanon != rootCanon && !parentCanon.startsWith(rootCanon + QLatin1Char('/'))) { return QString(); }
    return parentCanon + QLatin1Char('/') + QFileInfo(joined).fileName();
}

void LoteiBackend::runHostTool(const QString &name, const QJsonObject &args,
                               std::function<void(const QString &)> done)
{
    if (!agentReady()) {
        done(QStringLiteral("{\"error\":\"Host workspace tools are off. Turn on Agent mode and pick a "
                            "workspace folder (your qFlipper source) in setup first.\"}"));
        return;
    }

    if (name == QLatin1String("host_list")) {
        const QString abs = resolveAgentPath(args.value("path").toString(), true);
        if (abs.isEmpty()) { done(QStringLiteral("{\"error\":\"path is outside the workspace or missing\"}")); return; }
        QDir dir(abs);
        if (!dir.exists()) { done(QStringLiteral("{\"error\":\"not a folder\"}")); return; }
        QJsonArray arr;
        const QFileInfoList entries = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot,
                                                        QDir::DirsFirst | QDir::Name);
        int shown = 0;
        for (const QFileInfo &fi : entries) {
            if (shown++ >= LOTEI_HOST_LIST_CAP) { break; }
            arr.append(QJsonObject{
                {"name", fi.fileName()},
                {"type", fi.isDir() ? "dir" : "file"},
                {"size", static_cast<double>(fi.size())}
            });
        }
        done(QString::fromUtf8(QJsonDocument(arr).toJson(QJsonDocument::Compact)));

    } else if (name == QLatin1String("host_read")) {
        const QString abs = resolveAgentPath(args.value("path").toString(), true);
        if (abs.isEmpty()) { done(QStringLiteral("{\"error\":\"path is outside the workspace or missing\"}")); return; }
        QFile f(abs);
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            done(QStringLiteral("{\"error\":\"can't open %1\"}").arg(f.errorString()));
            return;
        }
        QByteArray d = f.read(LOTEI_HOST_READ_CAP + 1);
        f.close();
        const bool truncated = d.size() > LOTEI_HOST_READ_CAP;
        if (truncated) { d = d.left(LOTEI_HOST_READ_CAP); }
        QString out = QString::fromUtf8(d);
        if (truncated) { out += QStringLiteral("\n...(truncated)"); }
        if (out.isEmpty()) { out = QStringLiteral("(empty file)"); }
        done(out);

    } else if (name == QLatin1String("host_write")) {
        const QString abs = resolveAgentPath(args.value("path").toString(), false);
        if (abs.isEmpty()) { done(QStringLiteral("{\"error\":\"path is outside the workspace\"}")); return; }
        QDir().mkpath(QFileInfo(abs).absolutePath());
        QFile f(abs);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            done(QStringLiteral("{\"error\":\"can't write %1\"}").arg(f.errorString()));
            return;
        }
        const QByteArray bytes = args.value("content").toString().toUtf8();
        const qint64 n = f.write(bytes);
        f.close();
        if (n < 0) { done(QStringLiteral("{\"error\":\"write failed\"}")); return; }
        done(QStringLiteral("{\"wrote\":\"%1\",\"bytes\":%2}")
                 .arg(QDir(m_agentRoot).relativeFilePath(abs)).arg(static_cast<double>(n)));

    } else if (name == QLatin1String("host_run")) {
        const QString cmd = args.value("command").toString().trimmed();
        if (cmd.isEmpty()) { done(QStringLiteral("{\"error\":\"no command\"}")); return; }
        QProcess proc;
        proc.setWorkingDirectory(m_agentRoot);
        proc.setProcessChannelMode(QProcess::MergedChannels);
#if defined(Q_OS_WIN)
        proc.start(QStringLiteral("cmd"), {QStringLiteral("/c"), cmd});
#else
        proc.start(QStringLiteral("/bin/sh"), {QStringLiteral("-c"), cmd});
#endif
        if (!proc.waitForStarted(10000)) {
            done(QStringLiteral("{\"error\":\"couldn't start command\"}"));
            return;
        }
        const bool finished = proc.waitForFinished(LOTEI_HOST_RUN_TIMEOUT_MS);
        if (!finished) {
            proc.kill();
            proc.waitForFinished(2000);
            done(QStringLiteral("{\"error\":\"command timed out after %1s\"}")
                     .arg(LOTEI_HOST_RUN_TIMEOUT_MS / 1000));
            return;
        }
        QString out = QString::fromLocal8Bit(proc.readAll());
        bool truncated = out.size() > LOTEI_HOST_OUTPUT_CAP;
        if (truncated) { out = out.left(LOTEI_HOST_OUTPUT_CAP) + QStringLiteral("\n...(truncated)"); }
        const QJsonObject res{
            {"exit_code", proc.exitCode()},
            {"output", out}
        };
        done(QString::fromUtf8(QJsonDocument(res).toJson(QJsonDocument::Compact)));

    } else {
        done(QStringLiteral("{\"error\":\"unknown host tool '%1'\"}").arg(name));
    }
}

bool LoteiBackend::agentEnabled() const { return m_agentEnabled; }QString LoteiBackend::agentDir() const  { return m_agentRoot; }

void LoteiBackend::setAgentEnabled(bool on)
{
    if (on == m_agentEnabled) { return; }
    m_agentEnabled = on;
    QSettings().setValue(QStringLiteral("lotei/agentEnabled"), on);
    emit agentChanged();
}

void LoteiBackend::setAgentDir(const QString &dir)
{
    // Accept a plain path or a file:// URL (QML FolderDialog hands back a URL).
    QString path = dir;
    if (path.startsWith(QLatin1String("file://"))) { path = QUrl(path).toLocalFile(); }
    if (path == m_agentRoot) { return; }
    m_agentRoot = path;
    QSettings().setValue(QStringLiteral("lotei/agentDir"), m_agentRoot);
    emit agentChanged();
}

// Append a durable fact to long-term memory: update the in-memory copy (so it's
// in the very next system prompt), persist it locally, and mirror the whole
// memory to the Flipper SD at /ext/lotei/memory.txt when a device is around.
// m_memory is only a cache of memory.txt. The user can edit that file by hand --
// in the app's editor, through the file manager, or over the CLI -- so anything
// about to rewrite it re-reads first. Without this, the next fact the assistant
// learned was appended to a stale copy and silently reverted the manual edit.
// Replace the fact list wholesale and report what moved. Used both when the
// user saves memory.txt and when the card's copy is re-read.
//
// Note this does NOT route the added lines through rememberFact(): that path
// applies quality gates meant for text the model produced (minimum length,
// no trailing "?", filler rejection) and would silently drop a short fact the
// user deliberately typed. A hand-written file is taken verbatim.
void LoteiBackend::applyMemoryText(const QString &text, const QString &source)
{
    const QStringList before = loteiFactList(m_memory);
    m_memory = text.trimmed();
    const QStringList after = loteiFactList(m_memory);
    writeMemoryCache();

    QStringList added, removed;
    for (const QString &f : after)  { if (!loteiHasFact(before, f)) { added << f; } }
    for (const QString &f : before) { if (!loteiHasFact(after, f))  { removed << f; } }

    if (added.isEmpty() && removed.isEmpty()) { return; }

    loteiLog(QStringLiteral("%1: +%2 remembered, -%3 forgotten")
             .arg(source).arg(added.size()).arg(removed.size()));
    for (const QString &f : added)   { loteiLog(QStringLiteral("  remembered: %1").arg(f)); }
    for (const QString &f : removed) { loteiLog(QStringLiteral("  forgot: %1").arg(f)); }

    // The system prompt is rebuilt every turn, so the fact list itself is
    // already current. What is not current is the conversation: the last ~14
    // messages travel with the request, and a small model weights those far
    // more heavily than a block buried in a long system prompt -- so it keeps
    // reciting a fact that was just deleted. Say it plainly, in the history,
    // where it will actually be read.
    QString note = QStringLiteral("MEMORY UPDATED (%1). ").arg(source);
    if (!removed.isEmpty()) {
        note += QStringLiteral("These are NO LONGER TRUE and must never be repeated or listed again: ")
                + removed.join(QStringLiteral("; ")) + QStringLiteral(". ");
    }
    if (!added.isEmpty()) {
        note += QStringLiteral("These are now true: ") + added.join(QStringLiteral("; ")) + QStringLiteral(". ");
    }
    note += QStringLiteral("Anything you said earlier that conflicts with the current memory list is out of date. "
                           "That list is the complete and only set of durable facts about the user.");
    m_history.append(QJsonObject{{"role", "system"}, {"content", note}});
}

// Force a re-read of the card's copy. The chat calls this when it opens so a
// file edited in another tab is picked up without reconnecting.
// Things on the card that belong to the firmware or to the host OS, not to the
// user. Restoring these onto a different firmware gives apps that won't open
// (they're built against one ABI) and update bundles that are already stale --
// and the firmware itself is two clicks away in the store, so it is not what a
// backup is for.
// Reading a very large file over RPC crashes the Flipper: a 3 GB mass-storage
// image took the firmware down with "furi_check failed" every time, which is
// why the backup never reached the point of writing an archive. The cap is
// generous next to captures and dumps, and anything above it is reported by
// name rather than dropped quietly.
static const qint64 kMaxBackupFileBytes = 64LL * 1024 * 1024;

// Read by FlipperCli::setOpen(). A plain flag rather than walking the object
// tree: LoteiBackend is not a child of ApplicationBackend, so findChild() would
// have returned null and the guard would have done nothing at all.
static bool g_transferRunning = false;

static bool loteiSkipInBackup(const QString &name)
{
    static const QStringList skip = {
        QStringLiteral("update"),          // firmware bundles waiting to be applied
        QStringLiteral("Manifest"),        // firmware resource manifest
        QStringLiteral("apps"),            // FAPs, compiled against one firmware ABI
        QStringLiteral("apps_assets"),
        QStringLiteral("apps_manifests"),
        QStringLiteral(".int"),            // internal storage mirror
    };
    if (skip.contains(name, Qt::CaseInsensitive)) { return true; }
    // macOS and Windows litter removable media with these.
    return name.startsWith(QLatin1String(".Spotlight"))
        || name.startsWith(QLatin1String(".Trash"))
        || name.startsWith(QLatin1String(".fseventsd"))
        || name.startsWith(QLatin1String(".tmp"))
        || name == QLatin1String("System Volume Information");
}

void LoteiBackend::setTransfer(const QString &title, const QString &note, double frac)
{
    m_transferActive = true;
    m_transferTitle = title;
    m_transferNote = note;
    m_transferProgress = frac;
    emit transferChanged();
}

void LoteiBackend::finishTransfer(bool ok, const QString &text)
{
    m_transferActive = false;
    m_transferNote.clear();
    m_transferProgress = -1.0;
    loteiLog(QStringLiteral("transfer: %1 -- %2")
             .arg(ok ? QStringLiteral("finished") : QStringLiteral("failed"), text));
    emit transferChanged();

    // Hand the outcome to the backend so it lands on the very same Finished /
    // ErrorOccured screens a firmware install uses. Reusing the real state is
    // what removed the parallel set of visibility conditions this used to need
    // -- and with it the risk of the two drifting apart.
    if (m_appBackend) {
        m_appBackend->reportExternalResult(ok, int(BackendError::OperationError));
    }
}

void LoteiBackend::endTransfer()
{
    if (!m_transferActive) { return; }
    m_transferActive = false;
    m_transferNote.clear();
    m_transferProgress = -1.0;
    emit transferChanged();
}

void LoteiBackend::backupSdCard()
{
    Flipper::FlipperZero *dev = m_appBackend ? m_appBackend->device() : nullptr;
    const bool ready = m_appBackend && dev &&
                       m_appBackend->backendState() == ApplicationBackend::BackendState::Ready;
    if (!ready) { emit backupFailed(QStringLiteral("No Flipper connected.")); return; }
    if (m_transferActive) {
        emit backupFailed(QStringLiteral("An operation is already running."));
        return;
    }
    // A run that died between phases used to leave these populated forever,
    // and every later attempt was refused as "already running".
    m_backupDirs.clear();
    m_backupPending.clear();

    const QString base = QDir::homePath() + QStringLiteral("/Desktop/Nikita-qflipper");
    m_backupArchive = base + QStringLiteral("/bkp.tgz");

    // The staging folder lives in the system temp directory, not next to the
    // archive: files have to land somewhere before they can be packed, but that
    // is plumbing. Only bkp.tgz belongs in the folder the user opens.
    m_backupRoot = QDir::tempPath() + QStringLiteral("/lotei-backup");

    if (!QDir().mkpath(base)) {
        emit backupFailed(QStringLiteral("Couldn't create %1").arg(base));
        return;
    }

    // A leftover working folder from an interrupted run would get packed along
    // with the new one.
    // A leftover staging folder from an interrupted run would get packed along
    // with the new one.
    QDir(m_backupRoot).removeRecursively();
    if (!QDir().mkpath(m_backupRoot)) {
        emit backupFailed(QStringLiteral("Couldn't create %1").arg(m_backupRoot));
        return;
    }

    m_backupDirs = QStringList{ QStringLiteral("/ext") };
    m_backupPending.clear();
    m_backupTotal = 0;
    m_backupFiles = 0;
    m_backupBytes = 0;
    m_backupSkipped = 0;   // counted during the walk as well as the copy

    g_transferRunning = true;
    loteiLog(QStringLiteral("backup: walking /ext into %1").arg(m_backupRoot));
    setTransfer(QStringLiteral("Backing Up"), QStringLiteral("Reading the card…"), -1.0);
    emit backupProgress(QStringLiteral("Reading the card…"), 0.0);
    backupEnumerateNext();
}

// Depth-first walk of the card, one storageList at a time. Directories found
// go back on the queue; files are collected for the copy pass that follows.
void LoteiBackend::backupEnumerateNext()
{
    if (m_backupDirs.isEmpty()) {
        m_backupTotal = m_backupPending.size();
        qint64 want = 0;
        for (const auto &e : m_backupPending) { want += e.second; }
        loteiLog(QStringLiteral("backup: %1 file(s) found, %2 KB to copy%3")
                 .arg(m_backupTotal).arg(want / 1024)
                 .arg(m_backupSkipped ? QStringLiteral(", %1 too large to read")
                                        .arg(m_backupSkipped) : QString()));
        if (m_backupTotal == 0) {
            // Nothing to pack. Say so rather than reporting a success that
            // leaves an empty folder behind and no explanation.
            QDir(m_backupRoot).removeRecursively();
            g_transferRunning = false;
            finishTransfer(false, QStringLiteral("Nothing was found on the card to back up."));
            emit backupFailed(QStringLiteral("Nothing was found on the card to back up."));
            return;
        }
        backupCopyNext();
        return;
    }

    Flipper::FlipperZero *dev = m_appBackend ? m_appBackend->device() : nullptr;
    if (!dev) {
        m_backupDirs.clear(); m_backupPending.clear();
        g_transferRunning = false;
        finishTransfer(false, QStringLiteral("The Flipper went away mid-backup."));
        emit backupFailed(QStringLiteral("The Flipper went away mid-backup."));
        return;
    }

    const QString dir = m_backupDirs.takeFirst();
    auto *op = dev->rpc()->storageList(dir.toUtf8());

    connect(op, &AbstractOperation::finished, this, [this, op, dir]() {
        if (op->isError()) {
            loteiLog(QStringLiteral("backup: can't list %1 -- %2").arg(dir, op->errorString()));
            backupEnumerateNext();      // one unreadable folder must not stop the walk
            return;
        }

        for (const FileInfo &f : op->files()) {
            const QString name = QString::fromUtf8(f.name);
            const QString full = dir + QLatin1Char('/') + name;
            // The skip list is about the top level only: "apps" under /ext is
            // firmware territory, but a folder called "apps" nested inside
            // someone's own directory is their data.
            if (dir == QLatin1String("/ext") && loteiSkipInBackup(name)) { continue; }

            if (f.type == FileType::Directory) { m_backupDirs.append(full); }
            else if (qint64(f.size) > kMaxBackupFileBytes) {
                ++m_backupSkipped;
                loteiLog(QStringLiteral("backup: %1 SKIPPED -- %2 MB is past the %3 MB limit "
                                        "(reading it over RPC crashes the Flipper)")
                         .arg(full).arg(qint64(f.size) / 1024 / 1024)
                         .arg(kMaxBackupFileBytes / 1024 / 1024));
            }
            // The size comes free with the listing and decides the read
            // deadline below -- without it a large file dies on the generic 30s.
            else { m_backupPending.append(qMakePair(full, qint64(f.size))); }
        }
        backupEnumerateNext();
    });
}

void LoteiBackend::backupCopyNext()
{
    if (m_backupPending.isEmpty()) {
        loteiLog(QStringLiteral("backup: copied %1 file(s), %2 KB, %3 skipped -- packing")
                 .arg(m_backupFiles).arg(m_backupBytes / 1024).arg(m_backupSkipped));
        backupPackArchive();
        return;
    }

    Flipper::FlipperZero *dev = m_appBackend ? m_appBackend->device() : nullptr;
    if (!dev) {
        m_backupPending.clear();
        g_transferRunning = false;
        finishTransfer(false, QStringLiteral("The Flipper went away mid-backup."));
        emit backupFailed(QStringLiteral("The Flipper went away mid-backup."));
        return;
    }

    const auto entry = m_backupPending.takeFirst();
    const QString remote = entry.first;
    const qint64 size = entry.second;
    const QString rel = remote.mid(QStringLiteral("/ext").size());   // keeps the leading slash
    const QString local = m_backupRoot + rel;

    // The folder has to exist before the operation opens the file in it.
    QDir().mkpath(QFileInfo(local).absolutePath());

    const double frac = m_backupTotal > 0
                        ? double(m_backupTotal - m_backupPending.size() - 1) / double(m_backupTotal)
                        : 0.0;
    setTransfer(QStringLiteral("Backing Up"), rel, frac);
    emit backupProgress(rel, frac);

    // Handed over unopened: storageRead opens it itself, and pre-opening made
    // every file land on disk at zero bytes.
    auto *file = new QFile(local, this);
    auto *op = dev->rpc()->storageRead(remote.toUtf8(), file);

    // Reads run at a few hundred KB/s over RPC, so a mass-storage image or a
    // big capture needs far more than the generic 30s. Budget 20 KB/s -- well
    // under the real rate, so slow is still survivable -- with a 30s floor.
    // Plain arithmetic rather than qBound: mixing int literals with a qint64
    // expression made the overload ambiguous, and the lower bound is redundant
    // anyway -- the sum already starts at 30s.
    const qint64 budget = qint64(30000) + size / 20;
    op->setTimeout(int(budget < 3600000 ? budget : 3600000));

    connect(op, &AbstractOperation::finished, this, [this, op, file, remote, local]() {
        if (op->isError()) {
            // Skipped, not fatal: one unreadable file must not cost the other
            // 1464. The count is reported at the end so nothing goes unnoticed.
            ++m_backupSkipped;
            loteiLog(QStringLiteral("backup: %1 SKIPPED -- %2").arg(remote, op->errorString()));
            QFile::remove(local);   // don't leave a truncated stub in the archive
        } else {
            const qint64 sz = QFileInfo(local).size();
            ++m_backupFiles;
            m_backupBytes += sz;
            if (m_backupFiles <= 5 || (m_backupFiles % 50) == 0) {
                loteiLog(QStringLiteral("backup: %1 -> %2 bytes").arg(remote).arg(sz));
            }
        }
        file->deleteLater();
        backupCopyNext();
    });
}

// Writes one 512-byte ustar header. The format is simple enough to emit
// directly, which is why this no longer shells out to /usr/bin/tar: on macOS
// the Desktop is TCC-protected and a spawned child does not inherit the app's
// permission for it, so the archive step failed for a reason that had nothing
// to do with the data.
static QByteArray loteiTarHeader(const QString &path, qint64 size, bool isDir)
{
    QByteArray h(512, '\0');

    QByteArray name = path.toUtf8();
    if (isDir && !name.endsWith('/')) { name.append('/'); }
    if (name.size() > 100) { return QByteArray(); }   // caller skips these
    memcpy(h.data(), name.constData(), size_t(name.size()));

    auto octal = [&h](int off, int len, qint64 v) {
        const QByteArray s = QByteArray::number(v, 8).rightJustified(len - 1, '0');
        memcpy(h.data() + off, s.constData(), size_t(len - 1));
    };

    octal(100, 8, isDir ? 0755 : 0644);               // mode
    octal(108, 8, 0);                                 // uid
    octal(116, 8, 0);                                 // gid
    octal(124, 12, isDir ? 0 : size);                 // size
    octal(136, 12, QDateTime::currentSecsSinceEpoch());
    h[156] = isDir ? '5' : '0';                       // typeflag
    memcpy(h.data() + 257, "ustar\0" "00", 8);         // magic + version

    // Checksum is computed with the checksum field itself read as spaces.
    memset(h.data() + 148, ' ', 8);
    unsigned sum = 0;
    for (int k = 0; k < 512; ++k) { sum += unsigned(uchar(h.at(k))); }
    const QByteArray cs = QByteArray::number(sum, 8).rightJustified(6, '0');
    memcpy(h.data() + 148, cs.constData(), 6);
    h[154] = '\0';
    h[155] = ' ';
    return h;
}

// The mirror of loteiTarHeader: walks the gzipped tar and writes it back out.
// In-process for the same reason the writer is -- a spawned tar cannot reach a
// TCC-protected folder on the app's behalf.
static bool loteiUntar(const QString &archive, const QString &destDir, QString *error)
{
    gzFile gz = gzopen(QFile::encodeName(archive).constData(), "rb");
    if (!gz) { *error = QStringLiteral("Couldn't open the archive."); return false; }

    QByteArray hdr(512, '\0');
    while (true) {
        const int got = gzread(gz, hdr.data(), 512);
        if (got == 0) { break; }                       // clean end of stream
        if (got != 512) { *error = QStringLiteral("The archive is truncated."); gzclose(gz); return false; }
        if (hdr.at(0) == '\0') { break; }               // the empty end-of-archive block

        const QString name = QString::fromUtf8(hdr.constData());   // NUL-terminated
        const qint64 size = QByteArray(hdr.constData() + 124, 11).trimmed().toLongLong(nullptr, 8);
        const char type = hdr.at(156);

        // Never let an archive write outside the destination.
        const QString clean = QDir::cleanPath(name);
        if (clean.startsWith(QLatin1String("..")) || clean.startsWith(QLatin1Char('/'))) {
            *error = QStringLiteral("The archive contains an unsafe path: %1").arg(name);
            gzclose(gz);
            return false;
        }
        const QString out = destDir + QLatin1Char('/') + clean;

        if (type == '5') { QDir().mkpath(out); continue; }

        QDir().mkpath(QFileInfo(out).absolutePath());
        QFile f(out);
        if (!f.open(QIODevice::WriteOnly)) {
            *error = QStringLiteral("Couldn't write %1").arg(out);
            gzclose(gz);
            return false;
        }
        qint64 left = size;
        QByteArray buf(64 * 1024, '\0');
        while (left > 0) {
            const int want = int(qMin<qint64>(left, buf.size()));
            const int r = gzread(gz, buf.data(), unsigned(want));
            if (r <= 0) { *error = QStringLiteral("The archive ended early."); f.close(); gzclose(gz); return false; }
            f.write(buf.constData(), r);
            left -= r;
        }
        f.close();

        // Entries are padded to a 512-byte boundary; step over it.
        const int pad = int((512 - (size % 512)) % 512);
        if (pad) { gzread(gz, buf.data(), unsigned(pad)); }
    }

    gzclose(gz);
    return true;
}

bool loteiWriteTgz(const QString &srcDir, const QString &archive,
                   int *packedOut, int *skippedOut, QString *error);

// The archive writer, deliberately free-standing: LoteiBackend::backupPackArchive()
// and the CLI's "tgz" command both call it, so what the command exercises is
// exactly what a backup runs -- no second implementation to drift.
bool loteiWriteTgz(const QString &srcDir, const QString &archive,
                   int *packedOut, int *skippedOut, QString *error)
{
    if (!QDir(srcDir).exists()) { *error = QStringLiteral("no such folder: %1").arg(srcDir); return false; }

    QFile::remove(archive);
    gzFile gz = gzopen(QFile::encodeName(archive).constData(), "wb");
    if (!gz) { *error = QStringLiteral("couldn't open %1 for writing").arg(archive); return false; }

    auto put = [gz](const QByteArray &b) {
        return b.isEmpty() || gzwrite(gz, b.constData(), unsigned(b.size())) == b.size();
    };

    int packed = 0, skipped = 0;
    bool ok = true;

    // QDir::Hidden matters here: the card is full of dotfiles
    // (.badusb.settings, .eink.settings, .nested.log) which are copied but
    // would otherwise be left out of the archive.
    QDirIterator it(srcDir, QDir::Files | QDir::Dirs | QDir::Hidden | QDir::NoDotAndDotDot,
                    QDirIterator::Subdirectories);
    while (it.hasNext() && ok) {
        it.next();
        const QFileInfo fi = it.fileInfo();
        const QString rel = QDir(srcDir).relativeFilePath(fi.absoluteFilePath());

        const QByteArray hdr = loteiTarHeader(rel, fi.size(), fi.isDir());
        if (hdr.isEmpty()) { ++skipped; continue; }     // path past ustar's 100 chars
        if (!put(hdr)) { ok = false; break; }
        if (fi.isDir()) { ++packed; continue; }

        QFile f(fi.absoluteFilePath());
        if (!f.open(QIODevice::ReadOnly)) { ++skipped; continue; }
        qint64 written = 0;
        while (!f.atEnd()) {
            const QByteArray chunk = f.read(64 * 1024);
            if (chunk.isEmpty() || !put(chunk)) { ok = false; break; }
            written += chunk.size();
        }
        f.close();
        if (!ok) { break; }

        const int pad = int((512 - (written % 512)) % 512);
        if (pad && !put(QByteArray(pad, '\0'))) { ok = false; break; }
        ++packed;
    }

    if (ok) { ok = put(QByteArray(1024, '\0')); }       // two empty blocks end it
    const int gzErr = gzclose(gz);
    if (gzErr != Z_OK) { ok = false; *error = QStringLiteral("gzclose returned %1").arg(gzErr); }

    if (packedOut)  { *packedOut = packed; }
    if (skippedOut) { *skippedOut = skipped; }

    if (!ok) {
        if (error->isEmpty()) { *error = QStringLiteral("write failed"); }
        QFile::remove(archive);
        return false;
    }
    if (QFileInfo(archive).size() <= 0) { *error = QStringLiteral("the archive came out empty"); return false; }
    return true;
}

void LoteiBackend::backupPackArchive()
{
    setTransfer(QStringLiteral("Backing Up"), QStringLiteral("Packing bkp.tgz…"), -1.0);
    emit backupProgress(QStringLiteral("Packing bkp.tgz…"), 1.0);
    loteiLog(QStringLiteral("backup: packing %1 -> %2").arg(m_backupRoot, m_backupArchive));

    int packed = 0, skipped = 0;
    QString err;
    if (!loteiWriteTgz(m_backupRoot, m_backupArchive, &packed, &skipped, &err)) {
        // The copied files stay put: they are the user's data, and a packing
        // failure is no reason to throw them away.
        g_transferRunning = false;
        finishTransfer(false, QStringLiteral("Couldn't write bkp.tgz (%1).").arg(err));
        loteiLog(QStringLiteral("backup: packing FAILED -- %1").arg(err));
        // The staging folder is out of sight, so name it: the copied files are
        // still there and still the user's data.
        emit backupFailed(QStringLiteral("Couldn't write bkp.tgz (%1). The copied files are still in %2")
                          .arg(err, m_backupRoot));
        return;
    }

    const qint64 bytes = QFileInfo(m_backupArchive).size();
    loteiLog(QStringLiteral("backup: archived %1 entr(y/ies)%2 -> %3 KB")
             .arg(packed)
             .arg(skipped ? QStringLiteral(", %1 skipped").arg(skipped) : QString())
             .arg(bytes / 1024));

    g_transferRunning = false;
    finishTransfer(true, QStringLiteral("%1 file(s) saved to %2")
                   .arg(m_backupFiles).arg(m_backupArchive));
    QDir(m_backupRoot).removeRecursively();
    loteiLog(QStringLiteral("backup: done -- %1 file(s)%2 in %3 (%4 KB)")
             .arg(m_backupFiles)
             .arg(m_backupSkipped ? QStringLiteral(", %1 skipped").arg(m_backupSkipped) : QString())
             .arg(m_backupArchive).arg(bytes / 1024));
    emit backupFinished(m_backupArchive, m_backupFiles);
}

void LoteiBackend::formatSdCard()
{
    Flipper::FlipperZero *dev = m_appBackend ? m_appBackend->device() : nullptr;
    const bool ready = m_appBackend && dev &&
                       m_appBackend->backendState() == ApplicationBackend::BackendState::Ready;
    if (!ready) { finishTransfer(false, QStringLiteral("No Flipper connected."));
 emit formatFailed(QStringLiteral("No Flipper connected.")); return; }
    // m_transferActive is set synchronously by setTransfer() below, so it is
    // already true by the time a second click can arrive. The old guard tested
    // m_formatQueue, which stays empty until the storageList reply lands --
    // two quick presses started two listings, and the two queues then chewed
    // through the same entries at once. That is what took the app down.
    if (m_transferActive) { emit formatFailed(QStringLiteral("An operation is already running.")); return; }

    g_transferRunning = true;
    loteiLog(QStringLiteral("format: listing /ext"));
    setTransfer(QStringLiteral("Formatting"), QStringLiteral("Reading the card…"), -1.0);
    emit formatProgress(QStringLiteral("Reading the card…"), 0.0);

    auto *op = dev->rpc()->storageList(QByteArrayLiteral("/ext"));
    connect(op, &AbstractOperation::finished, this, [this, op]() {
        if (op->isError()) {
            loteiLog(QStringLiteral("format: FAILED to list /ext -- %1").arg(op->errorString()));
            finishTransfer(false, op->errorString());
            emit formatFailed(op->errorString());
            return;
        }
        m_formatQueue.clear();
        for (const FileInfo &f : op->files()) {
            m_formatQueue.append(QString::fromUtf8(f.name));
        }
        m_formatTotal = m_formatQueue.size();
        loteiLog(QStringLiteral("format: removing %1 entries from /ext").arg(m_formatTotal));

        if (m_formatQueue.isEmpty()) {
            m_sdFormatted = true;
            emit sdFormattedChanged();
            finishTransfer(true, QStringLiteral("The card is empty."));
            emit formatFinished();
            return;
        }
        runFormatQueue();
    });
}

void LoteiBackend::runFormatQueue()
{
    if (m_formatQueue.isEmpty()) {
        // The card is empty. Everything the firmware needs on it -- resources,
        // Manifest, apps -- is gone, so a plain reinstall has nothing to repair;
        // a full install is what puts the card back together.
        m_sdFormatted = true;
        emit sdFormattedChanged();
        loteiLog(QStringLiteral("format: /ext is empty"));
        finishTransfer(true, QStringLiteral("The card is empty."));
        emit formatFinished();
        return;
    }

    Flipper::FlipperZero *dev = m_appBackend ? m_appBackend->device() : nullptr;
    if (!dev) {
        m_formatQueue.clear();
        finishTransfer(false, QStringLiteral("The Flipper went away mid-format."));
        emit formatFailed(QStringLiteral("The Flipper went away mid-format."));
        return;
    }

    const QString name = m_formatQueue.takeFirst();
    const double frac = m_formatTotal > 0
                        ? double(m_formatTotal - m_formatQueue.size() - 1) / double(m_formatTotal)
                        : 0.0;
    setTransfer(QStringLiteral("Formatting"), name, frac);
    emit formatProgress(name, frac);

    auto *op = dev->rpc()->storageRemove((QStringLiteral("/ext/") + name).toUtf8(), true);
    connect(op, &AbstractOperation::finished, this, [this, op, name]() {
        if (op->isError()) {
            loteiLog(QStringLiteral("format: %1 FAILED -- %2").arg(name, op->errorString()));
        } else {
            loteiLog(QStringLiteral("format: %1 removed").arg(name));
        }
        runFormatQueue();     // one stubborn entry must not stop the rest
    });
}

void LoteiBackend::restoreSdCard(const QString &folderUrl)
{
    Flipper::FlipperZero *dev = m_appBackend ? m_appBackend->device() : nullptr;
    const bool ready = m_appBackend && dev &&
                       m_appBackend->backendState() == ApplicationBackend::BackendState::Ready;
    if (!ready) { finishTransfer(false, QStringLiteral("No Flipper connected."));
 emit restoreFailed(QStringLiteral("No Flipper connected.")); return; }
    if (m_transferActive) {
        emit restoreFailed(QStringLiteral("An operation is already running."));
        return;
    }

    const QString picked = QUrl(folderUrl).isLocalFile() ? QUrl(folderUrl).toLocalFile() : folderUrl;

    // Backups are archives now, but a folder from an older run still works.
    QString local = picked;
    if (QFileInfo(picked).isFile()) {
        const QString tmp = QDir::homePath() + QStringLiteral("/Desktop/Nikita-qflipper/.restore-working");
        QDir(tmp).removeRecursively();
        if (!QDir().mkpath(tmp)) {
            finishTransfer(false, QStringLiteral("Couldn't create %1").arg(tmp));
            emit restoreFailed(QStringLiteral("Couldn't create %1").arg(tmp));
            return;
        }

        loteiLog(QStringLiteral("restore: unpacking %1").arg(picked));
        QString err;
        if (!loteiUntar(picked, tmp, &err)) {
            loteiLog(QStringLiteral("restore: unpacking FAILED -- %1").arg(err));
            finishTransfer(false, QStringLiteral("Couldn't unpack the archive: %1").arg(err));
            emit restoreFailed(QStringLiteral("Couldn't unpack %1: %2")
                               .arg(QFileInfo(picked).fileName(), err));
            return;
        }
        local = tmp;
    }

    QDir dir(local);
    if (!dir.exists()) {
        finishTransfer(false, QStringLiteral("No such folder: %1").arg(local));
        emit restoreFailed(QStringLiteral("No such folder: %1").arg(local));
        return;
    }

    // One operation for the whole set: uploadFiles() walks directories itself,
    // so handing it the top-level entries restores the tree in a single pass
    // instead of a queue that could half-finish.
    QList<QUrl> urls;
    int skipped = 0;
    const QFileInfoList entries = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot, QDir::Name);
    for (const QFileInfo &fi : entries) {
        // The same exclusions as the backup, in case the folder was hand-edited
        // or came from somewhere else -- restoring apps or an update bundle
        // onto a different firmware is how a working Flipper stops working.
        if (loteiSkipInBackup(fi.fileName())) { ++skipped; continue; }
        urls.append(QUrl::fromLocalFile(fi.absoluteFilePath()));
    }

    if (urls.isEmpty()) {
        finishTransfer(false, QStringLiteral("Nothing to restore in %1").arg(local));
        emit restoreFailed(QStringLiteral("Nothing to restore in %1").arg(local));
        return;
    }

    g_transferRunning = true;
    loteiLog(QStringLiteral("restore: sending %1 entries from %2 to /ext (%3 skipped)")
             .arg(urls.size()).arg(local).arg(skipped));
    setTransfer(QStringLiteral("Restoring"), QStringLiteral("Uploading %1 item(s)…").arg(urls.size()), -1.0);
    emit restoreProgress(QStringLiteral("Uploading %1 item(s)…").arg(urls.size()), 0.0);

    auto *op = dev->utility()->uploadFiles(urls, QByteArrayLiteral("/ext"));
    const int count = urls.size();

    connect(op, &AbstractOperation::progressChanged, this, [this, op, count]() {
        setTransfer(QStringLiteral("Restoring"), QStringLiteral("Uploading %1 item(s)…").arg(count),
                    op->progress() / 100.0);
        emit restoreProgress(QStringLiteral("Uploading %1 item(s)…").arg(count), op->progress() / 100.0);
    });

    connect(op, &AbstractOperation::finished, this, [this, op, count]() {
        if (op->isError()) {
            loteiLog(QStringLiteral("restore: FAILED -- %1").arg(op->errorString()));
            finishTransfer(false, op->errorString());
            emit restoreFailed(op->errorString());
            return;
        }
        loteiLog(QStringLiteral("restore: %1 entries written to /ext").arg(count));
        finishTransfer(true, QStringLiteral("Restore complete."));
        emit restoreFinished(count);
    });
}

void LoteiBackend::reloadMemory()
{
    loadPortableMemory();
}

void LoteiBackend::refreshMemoryFromDisk()
{
    QFile f(loteiMemoryPath());
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) { return; }
    m_memory = QString::fromUtf8(f.readAll()).trimmed();
    f.close();
}

void LoteiBackend::writeMemoryCache() const
{
    QFile f(loteiMemoryPath());
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        f.write(m_memory.toUtf8());
        f.close();
    }
}

// A manual save of memory.txt is authoritative: adopt it as-is and do NOT push
// anything back, or the copy we were holding would overwrite what was just
// written.
bool LoteiBackend::adoptMemoryIfMemoryFile(const QString &path, const QString &content)
{
    if (QDir::cleanPath(path).compare(QLatin1String("/ext/lotei/memory.txt"), Qt::CaseInsensitive) != 0) {
        return false;
    }
    applyMemoryText(content, QStringLiteral("memory.txt edited by hand"));
    return true;
}

void LoteiBackend::rememberFact(const QString &fact)
{
    refreshMemoryFromDisk();
    QString clean = fact.trimmed();
    // Strip a leading bullet the model sometimes includes.
    while (clean.startsWith(QLatin1String("- ")) || clean.startsWith(QLatin1String("* "))) {
        clean = clean.mid(2).trimmed();
    }
    if (clean.isEmpty()) { return; }

    // --- Quality gate: reject junk so memory stays trustworthy, not a dump. ---
    // 1. Too short or too long to be a real, useful fact.
    if (clean.size() < 6 || clean.size() > 200) { return; }
    // 2. Must contain letters (not just symbols/numbers/emoji).
    if (!clean.contains(QRegularExpression(QStringLiteral("[A-Za-zÀ-ÿ]")))) { return; }
    // 3. Reject obvious conversational filler the model might try to store as a "fact".
    static const QRegularExpression filler(
        QStringLiteral("^(ok|okay|sure|yes|no|thanks|hello|hi|hey|done|"
                       "got it|hmm+|lol|kk+|test(ing)?)\\b"),
        QRegularExpression::CaseInsensitiveOption);
    if (filler.match(clean).hasMatch()) { return; }
    // 4. A fact is a statement, not a question or a command back to the user.
    if (clean.endsWith(QLatin1Char('?'))) { return; }

    // De-dupe (case-insensitive), and replace a near-identical prior fact instead
    // of stacking a second copy.
    QStringList lines = m_memory.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        QString l = line.trimmed();
        while (l.startsWith(QLatin1String("- "))) { l = l.mid(2).trimmed(); }
        if (l.compare(clean, Qt::CaseInsensitive) == 0) { return; }   // exact dupe
    }

    // --- Cap total memory so it never balloons: keep the most recent 40 facts. ---
    lines << (QStringLiteral("- ") + clean);
    const int kMaxFacts = 40;
    while (lines.size() > kMaxFacts) { lines.removeFirst(); }
    m_memory = lines.join(QLatin1Char('\n'));

    // Persist locally (authoritative, always available).
    QFile mf(loteiMemoryPath());
    if (mf.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        mf.write(m_memory.toUtf8());
        mf.close();
    }

    // Best-effort mirror onto the SD so memory travels with the Flipper.
    Flipper::FlipperZero *dev = m_appBackend ? m_appBackend->device() : nullptr;
    const bool ready = m_appBackend && dev &&
                       m_appBackend->backendState() == ApplicationBackend::BackendState::Ready;
    if (ready) {
        const QByteArray memPath = "/ext/lotei/memory.txt";
        const QString memBody = m_memory;
        ensureFlipperDir("/ext/lotei", [this, dev, memPath, memBody]() {
            QBuffer *buf = new QBuffer(this);
            buf->setData(memBody.toUtf8());
            buf->open(QIODevice::ReadOnly);
            auto *op = dev->rpc()->storageWrite(memPath, buf);
            connect(op, &AbstractOperation::finished, this, [buf]() { buf->deleteLater(); });
        });
    }
}

// Remove facts from memory: those containing `match`, or ALL if match is "all"
// (or empty). Returns how many were removed. Persists locally + mirrors to SD.
int LoteiBackend::forgetFacts(const QString &match)
{
    refreshMemoryFromDisk();
    if (m_memory.trimmed().isEmpty()) { return 0; }
    QStringList lines = m_memory.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    const int before = lines.size();

    const QString m = match.trimmed();
    if (m.isEmpty() || m.compare(QLatin1String("all"), Qt::CaseInsensitive) == 0) {
        lines.clear();
    } else {
        QStringList kept;
        for (const QString &line : lines) {
            if (!line.contains(m, Qt::CaseInsensitive)) { kept << line; }
        }
        lines = kept;
    }
    const int removed = before - lines.size();
    if (removed == 0) { return 0; }

    m_memory = lines.join(QLatin1Char('\n'));

    // Persist locally.
    QFile mf(loteiMemoryPath());
    if (mf.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        mf.write(m_memory.toUtf8());
        mf.close();
    }
    // Mirror to SD (writes the whole current memory, even if now empty).
    Flipper::FlipperZero *dev = m_appBackend ? m_appBackend->device() : nullptr;
    const bool ready = m_appBackend && dev &&
                       m_appBackend->backendState() == ApplicationBackend::BackendState::Ready;
    if (ready) {
        const QByteArray memPath = "/ext/lotei/memory.txt";
        const QString memBody = m_memory;
        ensureFlipperDir("/ext/lotei", [this, dev, memPath, memBody]() {
            QBuffer *buf = new QBuffer(this);
            buf->setData(memBody.toUtf8());
            buf->open(QIODevice::ReadOnly);
            auto *op = dev->rpc()->storageWrite(memPath, buf);
            connect(op, &AbstractOperation::finished, this, [buf]() { buf->deleteLater(); });
        });
    }
    return removed;
}

// Pull the script out of a chat message: the first fenced ``` code block if
// present, otherwise the whole text. Used by the manual "save to Flipper" panel.
QString LoteiBackend::extractScript(const QString &text) const
{
    const int a = text.indexOf(QStringLiteral("```"));
    if (a < 0) { return text.trimmed(); }
    const int nl = text.indexOf(QLatin1Char('\n'), a);
    if (nl < 0) { return text.trimmed(); }
    const int b = text.indexOf(QStringLiteral("```"), nl + 1);
    if (b < 0) { return text.mid(nl + 1).trimmed(); }
    return text.mid(nl + 1, b - nl - 1).trimmed();
}

// Manual, deterministic save: the USER picks the folder + filename and we write
// straight to the SD -- the model is never involved, so it can't fumble it. This
// is the reliable path: the 3b is great at drafting a script, bad at saving it,
// so we take the saving out of its hands entirely.
void LoteiBackend::saveScriptToFlipper(const QString &folder, const QString &filename, const QString &content)
{
    Flipper::FlipperZero *dev = m_appBackend ? m_appBackend->device() : nullptr;
    const bool ready = m_appBackend && dev &&
                       m_appBackend->backendState() == ApplicationBackend::BackendState::Ready;
    if (!ready) { emit scriptSaveError(QStringLiteral("No Flipper connected or ready.")); return; }

    // Normalise the folder into an /ext/<folder> path.
    QString fld = folder.trimmed();
    while (fld.startsWith(QLatin1Char('/'))) { fld = fld.mid(1); }
    if (fld.startsWith(QLatin1String("ext/"))) { fld = fld.mid(4); }
    if (fld.isEmpty()) { fld = QStringLiteral("badusb"); }

    // Sanitise the filename (no path separators or illegal chars).
    QString fn = filename.trimmed();
    fn.replace(QRegularExpression(QStringLiteral("[\\\\/:*?\"<>|]")), QStringLiteral("_"));
    if (fn.isEmpty()) { fn = QStringLiteral("script.txt"); }

    QString path = QStringLiteral("/ext/") + fld + QLatin1Char('/') + fn;
    QString body = content;

    // BadUSB must be .txt and gets the DuckyScript cleaner.
    if (path.startsWith(QLatin1String("/ext/badusb/"), Qt::CaseInsensitive)) {
        const int slash = path.lastIndexOf(QLatin1Char('/'));
        const int dot = path.lastIndexOf(QLatin1Char('.'));
        if (dot > slash) { path = path.left(dot) + QStringLiteral(".txt"); }
        else            { path += QStringLiteral(".txt"); }
        body = sanitizeDuckyScript(body);
    }

    const QByteArray p = path.toUtf8();
    const QString finalBody = body;
    const QByteArray parent = path.section('/', 0, -2).toUtf8();
    ensureFlipperDir(parent, [this, dev, p, finalBody, path]() {
        QBuffer *buf = new QBuffer(this);
        buf->setData(finalBody.toUtf8());
        buf->open(QIODevice::ReadOnly);
        auto *op = dev->rpc()->storageWrite(p, buf);
        connect(op, &AbstractOperation::finished, this, [this, op, buf, path]() {
            if (op->isError()) {
                loteiLog(QStringLiteral("save FAILED %1: %2").arg(path, op->errorString()));
                emit scriptSaveError(op->errorString());
            } else {
                loteiLog(QStringLiteral("saved %1").arg(path));
                emit scriptSaved(path);
            }
            buf->deleteLater();
        });
    });
}

// ---- In-app file editor (read/write any Flipper text file: .txt/.nfc/.ir/.sub) ----
// Read a file off the Flipper and hand its text to the QML editor.
void LoteiBackend::openFileForEdit(const QString &path)
{
    Flipper::FlipperZero *dev = m_appBackend ? m_appBackend->device() : nullptr;
    const bool ready = m_appBackend && dev &&
                       m_appBackend->backendState() == ApplicationBackend::BackendState::Ready;
    if (!ready) { emit fileEditError(QStringLiteral("No Flipper connected.")); return; }
    if (path.isEmpty()) { emit fileEditError(QStringLiteral("No path.")); return; }

    const QByteArray p = path.toUtf8();
    QBuffer *buf = new QBuffer(this);
    buf->open(QIODevice::ReadWrite);
    auto *op = dev->rpc()->storageRead(p, buf);
    connect(op, &AbstractOperation::finished, this, [this, op, buf, path]() {
        if (op->isError()) {
            emit fileEditError(op->errorString());
        } else {
            emit fileOpened(path, QString::fromUtf8(buf->data()));
        }
        buf->deleteLater();
    });
}

// Write edited text straight back to the Flipper at the exact path (no extension
// forcing -- the editor keeps the file's real name/type).
void LoteiBackend::writeFile(const QString &path, const QString &content)
{
    Flipper::FlipperZero *dev = m_appBackend ? m_appBackend->device() : nullptr;
    const bool ready = m_appBackend && dev &&
                       m_appBackend->backendState() == ApplicationBackend::BackendState::Ready;
    if (!ready) { emit fileEditError(QStringLiteral("No Flipper connected.")); return; }
    if (path.isEmpty()) { emit fileEditError(QStringLiteral("No path.")); return; }

    const QByteArray p = path.toUtf8();
    const QByteArray body = content.toUtf8();
    QBuffer *buf = new QBuffer(this);
    buf->setData(body);
    buf->open(QIODevice::ReadOnly);
    auto *op = dev->rpc()->storageWrite(p, buf);
    connect(op, &AbstractOperation::finished, this, [this, op, buf, path, content]() {
        if (op->isError()) {
            loteiLog(QStringLiteral("write %1 -- FAILED: %2").arg(path, op->errorString()));
            emit fileEditError(op->errorString());
        } else {
            loteiLog(QStringLiteral("write %1 -- done").arg(path));
            adoptMemoryIfMemoryFile(path, content);
            emit fileSaved(path);
        }
        buf->deleteLater();
    });
}

// ---- LoteiPalette ---------------------------------------------------------

LoteiPalette::LoteiPalette(QObject *parent)
    : QObject(parent)
{
    // Default (pink) palette -- mirrors Theme.qml's original values. This list's
    // order is also the order the editor lists the colors in.
    const QList<QPair<QString, QString>> defs = {
        {QStringLiteral("lightorange1"),  QStringLiteral("#fd8cff")},
        {QStringLiteral("lightorange2"),  QStringLiteral("#fd8cff")},
        {QStringLiteral("lightorange3"),  QStringLiteral("#ac5fae")},
        {QStringLiteral("darkorange1"),   QStringLiteral("#3d223d")},
        {QStringLiteral("darkorange2"),   QStringLiteral("#3a203b")},
        {QStringLiteral("mediumorange1"), QStringLiteral("#a159a2")},
        {QStringLiteral("mediumorange2"), QStringLiteral("#6c3c6d")},
        {QStringLiteral("mediumorange3"), QStringLiteral("#583159")},
        {QStringLiteral("mediumorange4"), QStringLiteral("#8a4c8b")},
        {QStringLiteral("mediumorange5"), QStringLiteral("#915092")},
        {QStringLiteral("lightgreen"),    QStringLiteral("#2ed832")},
        {QStringLiteral("mediumgreen1"),  QStringLiteral("#285b12")},
        {QStringLiteral("mediumgreen2"),  QStringLiteral("#203812")},
        {QStringLiteral("darkgreen"),     QStringLiteral("#0c160c")},
        {QStringLiteral("lightblue"),     QStringLiteral("#be69bf")},
        {QStringLiteral("mediumblue"),    QStringLiteral("#532e53")},
        {QStringLiteral("darkblue1"),     QStringLiteral("#492849")},
        {QStringLiteral("darkblue2"),     QStringLiteral("#3e223e")},
        {QStringLiteral("lightred1"),     QStringLiteral("#d174d3")},
        {QStringLiteral("lightred2"),     QStringLiteral("#cf73d1")},
        {QStringLiteral("lightred3"),     QStringLiteral("#945295")},
        {QStringLiteral("lightred4"),     QStringLiteral("#ae60af")},
        {QStringLiteral("mediumred1"),    QStringLiteral("#7b447c")},
        {QStringLiteral("mediumred2"),    QStringLiteral("#583058")},
        {QStringLiteral("darkred1"),      QStringLiteral("#3b203b")},
        {QStringLiteral("darkred2"),      QStringLiteral("#2a172a")}
    };
    for (const auto &p : defs) {
        m_order << p.first;
        m_defaults.insert(p.first, QColor(p.second));
    }
    m_colors = m_defaults;
    load();

    // Debounce disk writes so dragging a slider doesn't hammer QSettings.
    m_saveTimer = new QTimer(this);
    m_saveTimer->setSingleShot(true);
    m_saveTimer->setInterval(400);
    connect(m_saveTimer, &QTimer::timeout, this, [this]() { save(); });
}

QString LoteiPalette::hex(const QString &name) const
{
    return m_colors.value(name).value<QColor>().name(QColor::HexRgb);
}

void LoteiPalette::setColor(const QString &name, const QColor &c)
{
    if (!m_colors.contains(name) || !c.isValid()) { return; }
    if (m_colors.value(name).value<QColor>() == c) { return; }
    m_colors[name] = c;
    m_saveTimer->start();   // debounced; live recolor without per-drag disk writes
    emit changed();
}

void LoteiPalette::reset()
{
    m_colors = m_defaults;
    save();
    emit changed();
}

void LoteiPalette::load()
{
    const QString json = QSettings().value(QStringLiteral("lotei/palette")).toString();
    if (json.isEmpty()) { return; }
    const QJsonObject obj = QJsonDocument::fromJson(json.toUtf8()).object();
    for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
        if (m_colors.contains(it.key())) {
            const QColor c(it.value().toString());
            if (c.isValid()) { m_colors[it.key()] = c; }
        }
    }
}

void LoteiPalette::save() const
{
    QJsonObject obj;
    for (auto it = m_colors.constBegin(); it != m_colors.constEnd(); ++it) {
        obj.insert(it.key(), it.value().value<QColor>().name(QColor::HexRgb));
    }
    QSettings().setValue(QStringLiteral("lotei/palette"),
                         QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact)));
}

// ============================ FirmwareStore ============================

// Short display label for a channel id.
static QString fwChannelLabel(const QString &id)
{
    if (id == QLatin1String("development"))       { return QStringLiteral("dev"); }
    if (id == QLatin1String("release-candidate"))  { return QStringLiteral("rc"); }
    return id;   // "release", "dev"
}

FirmwareStore::FirmwareStore(QObject *parent)
    : QObject(parent)
{
    m_net.setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);

    const QString rel = QStringLiteral("release");
    const QStringList git = { QStringLiteral("release"), QStringLiteral("dev") };

    // fields: name, kind, locator, blurb, channels, wantChannel, latest, tgzUrl, status, raw
    m_sources = {
        { QStringLiteral("Official"),    Kind::DirJson,
          QStringLiteral("https://update.flipperzero.one/firmware/directory.json"),
          QStringLiteral("Stock Flipper Devices firmware."),      {},  rel, {}, {}, {}, {}, {} },
        { QStringLiteral("Momentum"),    Kind::DirJson,
          QStringLiteral("https://up.momentum-fw.dev/firmware/directory.json"),
          QStringLiteral("Feature-rich community firmware."),     {},  rel, {}, {}, {}, {}, {} },
        { QStringLiteral("Unleashed"),   Kind::GitHub,
          QStringLiteral("DarkFlippers/unleashed-firmware"),
          QStringLiteral("Popular unlocked community firmware."), git, rel, {}, {}, {}, {}, {} },
        { QStringLiteral("RogueMaster"), Kind::GitHub,
          QStringLiteral("RogueMaster/flipperzero-firmware-wPlugins"),
          QStringLiteral("Everything, plus the kitchen sink."),   git, rel, {}, {}, {}, {}, {} },
        // ARF ships only dev-tagged releases, so give it a single "dev" channel.
        { QStringLiteral("ARF"),         Kind::GitHub,
          QStringLiteral("D4C1-Labs/Flipper-ARF"),
          QStringLiteral("Automotive research: car keyfobs / Sub-GHz. Niche."),
          { QStringLiteral("dev") }, QStringLiteral("dev"), {}, {}, {}, {}, {} },
        // Xero publishes versioned releases (flipper-z-f7-update-local.tgz).
        { QStringLiteral("Xero"),        Kind::GitHub,
          QStringLiteral("noproto/xero-firmware"),
          QStringLiteral("Lean official-based community firmware."),
          { QStringLiteral("release") }, QStringLiteral("release"), {}, {}, {}, {}, {} },
    };

    // Restore each firmware's remembered channel choice.
    QSettings st;
    for (Source &s : m_sources) {
        const QString saved = st.value(QStringLiteral("firmware/ch/") + s.name).toString();
        if (!saved.isEmpty()) { s.wantChannel = saved; }
    }
    // Start from what was learned last run, so the UI is correct before -- and
    // regardless of -- what the network says this time.
    for (int i = 0; i < m_sources.size(); ++i) { loadDerived(i); }
}

void FirmwareStore::setOpen(bool value)
{
    if (value == m_open) { return; }
    m_open = value;
    emit openChanged();
    if (m_open) { refreshIfStale(); }   // freshen only if the cache aged out
}

void FirmwareStore::setBusy(bool value)
{
    if (value == m_busy) { return; }
    m_busy = value;
    emit busyChanged();
}

void FirmwareStore::setDeviceVersion(const QString &v)
{
    if (m_deviceVersion == v) { return; }
    m_deviceVersion = v;
    emit changed();

    // The main screen needs to know whether an update exists before the user
    // ever opens the store panel, so the first time a device reports in, go
    // fetch. Without this the button would have nothing to compare against.
    if (!m_deviceVersion.trimmed().isEmpty()) {
        const int i = installedIndex();
        loteiLog(QStringLiteral("firmware: device reports %1 -> %2")
                 .arg(m_deviceVersion.trimmed(),
                      i >= 0 ? m_sources.at(i).name : QStringLiteral("no matching source")));
    }

    if (!m_deviceVersion.trimmed().isEmpty() && !m_fetchedOnce) {
        m_fetchedOnce = true;
        refreshIfStale();
    }
}

// Counts a lookup in and, on the last one, works out what to tell the user.
// A source only counts as an update if it is a build the device is not already
// running -- otherwise "updates found" would fire for every source in the list.
void FirmwareStore::noteLookupDone(int index)
{
    if (m_pending <= 0) { return; }
    --m_pending;
    if (m_pending > 0) { return; }

    int reachable = 0;
    for (const Source &src : m_sources) {
        if (src.status == QLatin1String("ready")) { ++reachable; }
    }

    // An "update" means a newer build of the firmware THIS Flipper is running.
    // Counting every source whose version differs was meaningless: with six
    // firmwares listed, five always differ, so it reported "5 new releases
    // found" while there was nothing to update -- they were other people's
    // firmwares, not this one's.
    m_foundUpdates = updateAvailable() ? 1 : 0;

    if (reachable == 0) {
        m_checkSummary = QStringLiteral("Couldn't reach any source");
    } else if (m_foundUpdates > 0) {
        // Naming the build is the point: the same version then shows up in the
        // store and the main screen offers the update, so the message and the
        // rest of the app agree.
        m_checkSummary = QStringLiteral("%1 %2 available")
                         .arg(installedName(), installedLatest());
    } else if (!installedReady()) {
        m_checkSummary = QStringLiteral("Couldn't check your firmware");
    } else {
        // Nothing newer than what is installed. Say it plainly -- a count of
        // other firmwares' releases read as "it found things and did nothing".
        m_checkSummary = QStringLiteral("Everything is up to date");
    }
    loteiLog(QStringLiteral("firmware: %1").arg(m_checkSummary));
    Q_UNUSED(index)
}

QStringList FirmwareStore::installedChannels() const
{
    const int i = installedIndex();
    if (i < 0) { return QStringList(); }
    const Source &s = m_sources.at(i);
    // Same rule as the store panel: no picker when every channel resolves to
    // the same build.
    if (distinctChannelCount(s) <= 1) { return QStringList{ currentChannelId(s) }; }
    return s.channels;
}

QVariantList FirmwareStore::installedChannelModel() const
{
    QVariantList out;
    for (const QString &id : installedChannels()) {
        QVariantMap m;
        m.insert(QStringLiteral("name"), id);
        out.append(m);
    }
    return out;
}

QString FirmwareStore::installedChannel() const
{
    const int i = installedIndex();
    return (i >= 0) ? currentChannelId(m_sources.at(i)) : QString();
}

void FirmwareStore::setInstalledChannel(const QString &id)
{
    const int i = installedIndex();
    if (i < 0 || !m_sources.at(i).channels.contains(id)) { return; }
    if (m_sources.at(i).wantChannel == id) { return; }
    m_sources[i].wantChannel = id;
    QSettings().setValue(QStringLiteral("firmware/ch/") + m_sources.at(i).name, id);
    if (!m_sources.at(i).raw.isEmpty() || loadDerived(i)) { deriveFromCache(i); }
    else                                                  { fetchOne(i); }

    // Worth stating outright: for a GitHub source "dev" is the newest release
    // and "release" is the newest non-prerelease. When the newest release is
    // not flagged as a prerelease those are the same object, so switching
    // channel legitimately lands on the same build and the main button stays
    // on "Up to date". This line is what makes that visible instead of looking
    // like the switch did nothing.
    loteiLog(QStringLiteral("firmware: %1 channel -> %2, resolves to %3")
             .arg(m_sources.at(i).name, id,
                  m_sources.at(i).latest.isEmpty() ? QStringLiteral("nothing yet")
                                                   : m_sources.at(i).latest));
    emit changed();
}

// Reinstall means "the build already on the device, from the source it came
// from" -- not the official channel, which is what the stock action would do
// and which on a fork would silently replace it with a different firmware.
// The device only reports a version string, never which channel it came from.
// Without remembering it, "you are on release, dev is available" is unanswerable
// -- so the channel is recorded whenever this app is the one doing the flashing.
void FirmwareStore::rememberFlashedChannel(int index)
{
    if (index < 0 || index >= m_sources.size()) { return; }
    const Source &s = m_sources.at(index);
    QSettings().setValue(QStringLiteral("firmware/flashedCh/") + s.name, currentChannelId(s));
    QSettings().setValue(QStringLiteral("firmware/flashedVer/") + s.name, s.latest);
}

QString FirmwareStore::installedFromChannel() const
{
    const int i = installedIndex();
    if (i < 0) { return QString(); }
    QSettings st;
    const QString ver = st.value(QStringLiteral("firmware/flashedVer/") + m_sources.at(i).name).toString();
    // Only trust the record if it describes the build actually running; the
    // user may have flashed from somewhere else since.
    if (ver.isEmpty() || ver.compare(m_deviceVersion.trimmed(), Qt::CaseInsensitive) != 0) { return QString(); }
    return st.value(QStringLiteral("firmware/flashedCh/") + m_sources.at(i).name).toString();
}

bool FirmwareStore::channelSwitchPending() const
{
    const QString from = installedFromChannel();
    if (from.isEmpty() || !installedReady()) { return false; }
    return from.compare(installedChannel(), Qt::CaseInsensitive) != 0;
}

void FirmwareStore::reinstallInstalled()
{
    const int i = installedIndex();
    if (i < 0 || !installedReady()) { return; }
    install(i);
}

QString FirmwareStore::installedDate() const
{
    const int i = installedIndex();
    return (i >= 0) ? m_sources.at(i).date : QString();
}

QString FirmwareStore::selectedName() const
{
    return hasSelection() ? m_sources.at(m_selected).name : QString();
}

QString FirmwareStore::selectedVersion() const
{
    return hasSelection() ? m_sources.at(m_selected).latest : QString();
}

QString FirmwareStore::selectedDate() const
{
    return hasSelection() ? m_sources.at(m_selected).date : QString();
}

void FirmwareStore::select(int index)
{
    if (index < 0 || index >= m_sources.size()) { return; }
    // Picking the staged row again unstages it, so there is a way back without
    // hunting for a separate cancel control.
    m_selected = (m_selected == index) ? -1 : index;
    emit changed();
}

void FirmwareStore::clearSelection()
{
    if (m_selected < 0) { return; }
    m_selected = -1;
    emit changed();
}

void FirmwareStore::installSelected()
{
    if (!hasSelection()) { return; }
    install(m_selected);
}

QString FirmwareStore::installedName() const
{
    const int i = installedIndex();
    return (i >= 0) ? m_sources.at(i).name : QString();
}

QString FirmwareStore::installedLatest() const
{
    const int i = installedIndex();
    return (i >= 0) ? m_sources.at(i).latest : QString();
}

bool FirmwareStore::installedReady() const
{
    const int i = installedIndex();
    return (i >= 0) && m_sources.at(i).status == QLatin1String("ready")
           && !m_sources.at(i).tgzUrl.isEmpty();
}

// Same equality-not-ordering reasoning as in sources(): the store only ever
// lists the newest build, so "differs from what is running" is what can be
// claimed honestly.
bool FirmwareStore::updateAvailable() const
{
    if (!installedReady()) { return false; }
    // A different build, or the same build from a different channel -- both are
    // something to offer. The second case only fires when this app flashed the
    // current build and therefore knows which channel it came from.
    if (installedLatest().compare(m_deviceVersion.trimmed(), Qt::CaseInsensitive) != 0) { return true; }
    return channelSwitchPending();
}

// Each firmware stamps its version with a recognisable shape, so the running
// build can be traced back to the source it came from without asking the device
// anything extra. Unknown shapes fall through to Official, which is where a
// plain "1.4.3" or a bare commit hash comes from.
int FirmwareStore::installedIndex() const
{
    const QString v = m_deviceVersion.trimmed();
    if (v.isEmpty()) { return -1; }

    QString want;
    if (v.startsWith(QLatin1String("mntm"), Qt::CaseInsensitive))        { want = QStringLiteral("Momentum"); }
    else if (v.startsWith(QLatin1String("unlshd"), Qt::CaseInsensitive)) { want = QStringLiteral("Unleashed"); }
    else if (v.startsWith(QLatin1String("RM"), Qt::CaseSensitive))       { want = QStringLiteral("RogueMaster"); }
    else if (v.contains(QLatin1String("arf"), Qt::CaseInsensitive))      { want = QStringLiteral("ARF"); }
    else                                                                 { want = QStringLiteral("Official"); }

    for (int i = 0; i < m_sources.size(); ++i) {
        if (m_sources.at(i).name.compare(want, Qt::CaseInsensitive) == 0) { return i; }
    }
    return -1;
}

// What a given channel of this source would resolve to, without touching the
// source's own state. Mirrors the picking rules in deriveFromCache().
QString FirmwareStore::channelVersion(const Source &s, const QString &ch) const
{
    if (s.raw.isEmpty()) { return QString(); }

    if (s.kind == Kind::DirJson) {
        const QJsonArray channels = QJsonDocument::fromJson(s.raw).object()
                                    .value(QStringLiteral("channels")).toArray();
        for (const QJsonValue &cv : channels) {
            const QJsonObject c = cv.toObject();
            if (c.value(QStringLiteral("id")).toString() != ch) { continue; }
            const QJsonArray versions = c.value(QStringLiteral("versions")).toArray();
            if (versions.isEmpty()) { return QString(); }
            return versions.first().toObject().value(QStringLiteral("version")).toString();
        }
        return QString();
    }

    const QJsonArray rels = QJsonDocument::fromJson(s.raw).array();
    if (rels.isEmpty()) { return QString(); }
    if (ch == QLatin1String("dev")) {
        for (const QJsonValue &rv : rels) {
            const QJsonObject r = rv.toObject();
            if (r.value(QStringLiteral("prerelease")).toBool()) {
                return r.value(QStringLiteral("tag_name")).toString();
            }
        }
        return rels.first().toObject().value(QStringLiteral("tag_name")).toString();
    }
    for (const QJsonValue &rv : rels) {
        const QJsonObject r = rv.toObject();
        if (!r.value(QStringLiteral("prerelease")).toBool()) {
            return r.value(QStringLiteral("tag_name")).toString();
        }
    }
    return rels.first().toObject().value(QStringLiteral("tag_name")).toString();
}

// A channel picker is only worth showing when the channels actually lead
// somewhere different. Unleashed and RogueMaster list release and dev but
// publish no prereleases, so both land on the same build -- a dropdown there
// invites a choice that changes nothing. Decided from the data rather than a
// hand-kept list, so it follows whatever those repos do next.
int FirmwareStore::distinctChannelCount(const Source &s) const
{
    if (s.channels.size() <= 1) { return s.channels.size(); }
    // Nothing fetched this session: keep the picker so a switch can go fetch.
    if (s.raw.isEmpty()) { return s.channels.size(); }

    QStringList seen;
    for (const QString &ch : s.channels) {
        const QString v = channelVersion(s, ch);
        if (!v.isEmpty() && !seen.contains(v, Qt::CaseInsensitive)) { seen << v; }
    }
    return (seen.size() > 1) ? s.channels.size() : 1;
}

QVariantList FirmwareStore::sources() const
{
    const int running = installedIndex();
    QVariantList out;
    for (int i = 0; i < m_sources.size(); ++i) {
        const Source &s = m_sources.at(i);
        QVariantMap m;
        m.insert(QStringLiteral("name"), s.name);
        m.insert(QStringLiteral("blurb"), s.blurb);
        m.insert(QStringLiteral("latest"), s.latest);
        m.insert(QStringLiteral("status"), s.status);
        m.insert(QStringLiteral("ready"), s.status == QLatin1String("ready") && !s.tgzUrl.isEmpty());
        m.insert(QStringLiteral("channel"), fwChannelLabel(currentChannelId(s)));
        m.insert(QStringLiteral("channelCount"), distinctChannelCount(s));

        // Same firmware family as the one running?
        const bool installed = (i == running);
        // Deliberately an equality test, not an ordering one. Version strings
        // across these forks have no comparable format ("unlshd-080e" vs
        // "unlshd-089", "RM0722-1811-ff9f4feb"), so claiming which is newer
        // would be guesswork. The store only ever lists the latest build, so
        // "differs from what is running" is the honest reading of "update".
        const bool sameBuild = installed && !s.latest.isEmpty()
                               && s.latest.compare(m_deviceVersion.trimmed(), Qt::CaseInsensitive) == 0;
        m.insert(QStringLiteral("date"), s.date);
        m.insert(QStringLiteral("installed"), installed);
        m.insert(QStringLiteral("upToDate"), sameBuild);
        m.insert(QStringLiteral("selected"), i == m_selected);
        // This panel picks; it never flashes. The only button that starts an
        // install is the one on the main screen, so a row can be staged
        // ("IMPORT"), already staged ("IMPORTED"), or the build already running.
        m.insert(QStringLiteral("action"), sameBuild      ? QStringLiteral("INSTALLED")
                                         : (i == m_selected) ? QStringLiteral("IMPORTED")
                                                             : QStringLiteral("IMPORT"));
        out.append(m);
    }
    return out;
}

void FirmwareStore::refresh()
{
    loteiLog(QStringLiteral("firmware: checking %1 sources").arg(m_sources.size()));
    m_lastFetch = QDateTime::currentDateTime();
    m_pending = m_sources.size();
    m_foundUpdates = 0;
    m_checkSummary.clear();
    for (int i = 0; i < m_sources.size(); ++i) {
        // Only a source with nothing cached shows "checking". Clearing the
        // version of a source we already know turns a slow or rate-limited
        // reply into a row that reads "unavailable" for no reason.
        if (m_sources[i].raw.isEmpty()) {
            m_sources[i].status = QStringLiteral("checking");
            m_sources[i].latest.clear();
            m_sources[i].tgzUrl.clear();
            m_sources[i].date.clear();
        }
    }
    emit changed();
    for (int i = 0; i < m_sources.size(); ++i) { fetchOne(i); }
}

// Opening the panel shouldn't cost network requests when the answer is minutes
// old. "check for updates" still calls refresh() directly and always goes out.
void FirmwareStore::refreshIfStale()
{
    const bool haveAll = [this]() {
        for (const Source &s : m_sources) { if (s.raw.isEmpty()) { return false; } }
        return true;
    }();
    if (haveAll && m_lastFetch.isValid() && m_lastFetch.secsTo(QDateTime::currentDateTime()) < 600) {
        loteiLog(QStringLiteral("firmware: versions are %1s old, not re-checking")
                 .arg(m_lastFetch.secsTo(QDateTime::currentDateTime())));
        return;
    }
    refresh();
}

QString FirmwareStore::currentChannelId(const Source &s) const
{
    if (s.channels.contains(s.wantChannel)) { return s.wantChannel; }
    return s.channels.isEmpty() ? s.wantChannel : s.channels.first();
}

void FirmwareStore::fetchOne(int index)
{
    if (index < 0 || index >= m_sources.size()) { return; }
    const Source src = m_sources.at(index);

    // GitHub: pull the whole release list (newest-first) so release/dev come from one fetch.
    QUrl url = (src.kind == Kind::DirJson)
             ? QUrl(src.locator)
             : QUrl(QStringLiteral("https://api.github.com/repos/%1/releases?per_page=30").arg(src.locator));

    QNetworkRequest req(url);
    req.setRawHeader("User-Agent", "Hyper-Zero-UI");
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

    QNetworkReply *reply = m_net.get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply, index]() {
        reply->deleteLater();
        if (index < 0 || index >= m_sources.size()) { return; }
        Source &s = m_sources[index];

        if (reply->error() != QNetworkReply::NoError) {
            // Say what actually went wrong. GitHub answers 403 once the
            // unauthenticated 60-per-hour budget is spent, and four of these
            // sources share it -- without this line "unavailable" looks like a
            // broken source rather than a spent quota.
            const int http = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            const QString why = (http == 403 || http == 429)
                                ? QStringLiteral("rate limited by GitHub (HTTP %1) -- the hourly quota is shared by every firmware source").arg(http)
                                : (http > 0 ? QStringLiteral("HTTP %1").arg(http) : reply->errorString());
            loteiLog(QStringLiteral("firmware: %1 lookup failed -- %2").arg(s.name, why));

            // Whatever was fetched last time is still perfectly good, so fall
            // back to it instead of blanking the row to "unavailable".
            if (!s.raw.isEmpty()) {
                deriveFromCache(index);
                loteiLog(QStringLiteral("firmware: %1 kept this session's cached %2").arg(s.name, s.latest));
            } else if (loadDerived(index)) {
                loteiLog(QStringLiteral("firmware: %1 restored saved %2").arg(s.name, s.latest));
            } else {
                s.status = QStringLiteral("error");
            }
            noteLookupDone(index);   // after the fallback, for the same reason
            emit changed();
            return;
        }
        s.raw = reply->readAll();

        // Discover the channel list from the directory.json, but keep only the
        // canonical channels -- Momentum also lists dozens of per-PR preview
        // channels (long ids like "pr294:feat/...") that we don't want to cycle.
        if (s.kind == Kind::DirJson) {
            const QJsonArray channels = QJsonDocument::fromJson(s.raw).object()
                                        .value(QStringLiteral("channels")).toArray();
            auto hasChannel = [&channels](const QString &id) {
                for (const QJsonValue &cv : channels) {
                    if (cv.toObject().value(QStringLiteral("id")).toString() == id) { return true; }
                }
                return false;
            };
            static const QStringList canonical = {
                QStringLiteral("release"), QStringLiteral("release-candidate"), QStringLiteral("development") };
            QStringList ids;
            for (const QString &c : canonical) { if (hasChannel(c)) { ids << c; } }
            s.channels = ids;
        }

        deriveFromCache(index);
        if (s.status == QLatin1String("ready")) {
            loteiLog(QStringLiteral("firmware: %1 [%2] -> %3%4")
                     .arg(s.name, currentChannelId(s), s.latest,
                          s.date.isEmpty() ? QString() : QStringLiteral(" (%1)").arg(s.date)));
        } else {
            loteiLog(QStringLiteral("firmware: %1 [%2] replied, but no usable build was found in it")
                     .arg(s.name, currentChannelId(s)));
        }
        // Counted last, on purpose. This is what closes the verdict when the
        // final lookup lands, and it reads every source's resolved state -- so
        // running it before deriveFromCache() judged the last source on data it
        // had not parsed yet, and reported "couldn't check your firmware" one
        // line before printing that same firmware's version.
        noteLookupDone(index);
        emit changed();
    });
}

void FirmwareStore::deriveFromCache(int index)
{
    if (index < 0 || index >= m_sources.size()) { return; }
    Source &s = m_sources[index];
    const QString ch = currentChannelId(s);
    s.latest.clear();
    s.tgzUrl.clear();
    s.date.clear();
    if (s.raw.isEmpty()) {
        // No payload this session (fresh start, or the fetch was refused).
        // Whatever was saved for this channel still describes a real build.
        if (!loadDerived(index)) { s.status = QStringLiteral("error"); }
        return;
    }

    if (s.kind == Kind::DirJson) {
        const QJsonArray channels = QJsonDocument::fromJson(s.raw).object()
                                    .value(QStringLiteral("channels")).toArray();
        for (const QJsonValue &cv : channels) {
            const QJsonObject c = cv.toObject();
            if (c.value(QStringLiteral("id")).toString() != ch) { continue; }
            const QJsonArray versions = c.value(QStringLiteral("versions")).toArray();
            if (versions.isEmpty()) { break; }
            const QJsonObject v0 = versions.first().toObject();
            s.latest = v0.value(QStringLiteral("version")).toString();
            // directory.json carries a unix timestamp; some mirrors write an
            // ISO string instead, so accept either rather than showing nothing.
            const QJsonValue ts = v0.value(QStringLiteral("timestamp"));
            if (ts.isDouble()) {
                s.date = QDateTime::fromSecsSinceEpoch(qint64(ts.toDouble())).toString(QStringLiteral("yyyy-MM-dd"));
            } else {
                const QDateTime dt = QDateTime::fromString(v0.value(QStringLiteral("date")).toString(), Qt::ISODate);
                if (dt.isValid()) { s.date = dt.toString(QStringLiteral("yyyy-MM-dd")); }
            }
            for (const QJsonValue &fv : v0.value(QStringLiteral("files")).toArray()) {
                const QJsonObject f = fv.toObject();
                if (f.value(QStringLiteral("target")).toString() == QLatin1String("f7") &&
                    f.value(QStringLiteral("type")).toString() == QLatin1String("update_tgz")) {
                    s.tgzUrl = f.value(QStringLiteral("url")).toString();
                    break;
                }
            }
            break;
        }
    } else {   // GitHub: "dev" = newest prerelease, "release" = newest stable
        const QJsonArray rels = QJsonDocument::fromJson(s.raw).array();
        QJsonObject chosen;
        if (ch == QLatin1String("dev")) {
            // Was "the newest release, whatever it is" -- which on any repo that
            // doesn't publish prereleases is the exact same object the release
            // channel picks. The two channels then resolve to one build and
            // switching between them looks like it does nothing. Prefer a real
            // prerelease; fall back only when the repo has none.
            for (const QJsonValue &rv : rels) {
                const QJsonObject r = rv.toObject();
                if (r.value(QStringLiteral("prerelease")).toBool()) { chosen = r; break; }
            }
            if (chosen.isEmpty() && !rels.isEmpty()) { chosen = rels.first().toObject(); }
        } else {
            for (const QJsonValue &rv : rels) {
                const QJsonObject r = rv.toObject();
                if (!r.value(QStringLiteral("prerelease")).toBool()) { chosen = r; break; }
            }
            if (chosen.isEmpty() && !rels.isEmpty()) { chosen = rels.first().toObject(); }
        }
        if (!chosen.isEmpty()) {
            s.latest = chosen.value(QStringLiteral("tag_name")).toString();
            const QDateTime dt = QDateTime::fromString(
                chosen.value(QStringLiteral("published_at")).toString(), Qt::ISODate);
            if (dt.isValid()) { s.date = dt.toString(QStringLiteral("yyyy-MM-dd")); }
            QString bestUrl, bestName, anyUrl, anyName;
            for (const QJsonValue &av : chosen.value(QStringLiteral("assets")).toArray()) {
                const QJsonObject a = av.toObject();
                const QString name = a.value(QStringLiteral("name")).toString();
                if (!name.endsWith(QLatin1String(".tgz"))) { continue; }
                const QString dl = a.value(QStringLiteral("browser_download_url")).toString();
                if (anyName.isEmpty() || name.size() < anyName.size()) { anyName = name; anyUrl = dl; }
                if (name.contains(QLatin1String("f7")) && name.contains(QLatin1String("update"))) {
                    if (bestName.isEmpty() || name.size() < bestName.size()) { bestName = name; bestUrl = dl; }
                }
            }
            s.tgzUrl = bestUrl.isEmpty() ? anyUrl : bestUrl;
        }
    }

    s.status = (!s.latest.isEmpty() && !s.tgzUrl.isEmpty()) ? QStringLiteral("ready")
                                                            : QStringLiteral("error");
    if (s.status == QLatin1String("ready")) { saveDerived(index); }
}

// The raw payloads only ever lived in memory, so every restart had to hit the
// network again -- and four of the five sources share GitHub's 60-per-hour
// unauthenticated budget. One rate-limited start was enough to leave the whole
// panel reading "unavailable" and the main button "No data". Persisting the
// handful of derived fields (not the payload, which runs to hundreds of KB)
// means a restart shows the right thing immediately and the network is only
// ever an improvement.
void FirmwareStore::saveDerived(int index) const
{
    if (index < 0 || index >= m_sources.size()) { return; }
    const Source &s = m_sources.at(index);
    const QString key = QStringLiteral("firmware/cache/%1/%2/").arg(s.name, currentChannelId(s));
    QSettings st;
    st.setValue(key + QStringLiteral("latest"), s.latest);
    st.setValue(key + QStringLiteral("tgz"), s.tgzUrl);
    st.setValue(key + QStringLiteral("date"), s.date);
}

bool FirmwareStore::loadDerived(int index)
{
    if (index < 0 || index >= m_sources.size()) { return false; }
    Source &s = m_sources[index];
    const QString key = QStringLiteral("firmware/cache/%1/%2/").arg(s.name, currentChannelId(s));
    QSettings st;
    const QString latest = st.value(key + QStringLiteral("latest")).toString();
    const QString tgz    = st.value(key + QStringLiteral("tgz")).toString();
    if (latest.isEmpty() || tgz.isEmpty()) { return false; }
    s.latest = latest;
    s.tgzUrl = tgz;
    s.date   = st.value(key + QStringLiteral("date")).toString();
    s.status = QStringLiteral("ready");
    return true;
}

void FirmwareStore::cycleChannel(int index)
{
    if (index < 0 || index >= m_sources.size()) { return; }
    Source &s = m_sources[index];
    if (s.channels.size() < 2) { return; }
    int i = s.channels.indexOf(currentChannelId(s));
    i = (i + 1) % s.channels.size();
    s.wantChannel = s.channels.at(i);
    QSettings().setValue(QStringLiteral("firmware/ch/") + s.name, s.wantChannel);
    deriveFromCache(index);   // every channel is already cached -> instant, no re-fetch
    emit changed();
}

void FirmwareStore::install(int index)
{
    if (index < 0 || index >= m_sources.size()) { return; }
    const Source src = m_sources.at(index);

    // Everything below reports through failed()/progress(), which are wired to
    // the store panel -- and that panel is closed when this runs from Reinstall
    // in the tools tab. Without these lines the whole download is invisible:
    // no progress, and a failure that never reaches the user.
    if (src.status != QLatin1String("ready") || src.tgzUrl.isEmpty()) {
        loteiLog(QStringLiteral("firmware: install %1 refused -- no downloadable build").arg(src.name));
        emit failed(index, QStringLiteral("No downloadable build found -- try re-checking."));
        return;
    }
    if (m_busy) {
        loteiLog(QStringLiteral("firmware: install %1 refused -- a download is already running").arg(src.name));
        emit failed(index, QStringLiteral("A download is already in progress."));
        return;
    }

    QString dir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    if (dir.isEmpty()) { dir = QDir::tempPath(); }
    dir += QStringLiteral("/firmware");
    QDir().mkpath(dir);

    const QUrl url(src.tgzUrl);
    QString fileName = url.fileName();
    if (fileName.isEmpty() || !fileName.endsWith(QLatin1String(".tgz"))) {
        fileName = QStringLiteral("flipper-z-f7-update.tgz");
    }
    const QString outPath = dir + QStringLiteral("/") + fileName;

    QNetworkRequest req(url);
    req.setRawHeader("User-Agent", "Hyper-Zero-UI");
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

    rememberFlashedChannel(index);
    setBusy(true);
    loteiLog(QStringLiteral("firmware: downloading %1 %2 from %3")
             .arg(src.name, src.latest, src.tgzUrl));
    emit progress(index, 0.0, QStringLiteral("Downloading %1…").arg(src.latest));

    QNetworkReply *reply = m_net.get(req);
    connect(reply, &QNetworkReply::downloadProgress, this, [this, index](qint64 rec, qint64 total) {
        const qreal frac = (total > 0) ? (qreal)rec / (qreal)total : 0.0;
        emit progress(index, frac, QStringLiteral("Downloading…"));
    });
    connect(reply, &QNetworkReply::finished, this, [this, reply, index, outPath]() {
        reply->deleteLater();
        setBusy(false);
        if (reply->error() != QNetworkReply::NoError) {
            loteiLog(QStringLiteral("firmware: download FAILED -- %1").arg(reply->errorString()));
            emit failed(index, QStringLiteral("Download failed: %1").arg(reply->errorString()));
            return;
        }
        QFile f(outPath);
        if (!f.open(QIODevice::WriteOnly)) {
            loteiLog(QStringLiteral("firmware: couldn't write %1").arg(outPath));
            emit failed(index, QStringLiteral("Couldn't save the download to disk."));
            return;
        }
        const QByteArray body = reply->readAll();
        f.write(body);
        f.close();
        loteiLog(QStringLiteral("firmware: downloaded %1 bytes -> %2").arg(body.size()).arg(outPath));
        emit progress(index, 1.0, QStringLiteral("Ready -- flashing…"));
        emit readyToInstall(QUrl::fromLocalFile(outPath).toString());
    });
}

// ===================== FlipperCli: in-app Flipper text CLI =====================

// ---- shared helpers (command tables, path routing, help layout) -------------
namespace {

// Every shortcut this CLI adds on top of the firmware, with the one-line
// description "help <command>" prints. Order here is the order they're listed.
struct CliCmd { const char *name; const char *help; };
const CliCmd kCliCommands[] = {
    { "cat",      "Prints a file's contents to the screen." },
    { "cd",       "Changes the current folder on the Flipper." },
    { "clear",    "Clears the CLI terminal screen view." },
    { "colors",   "Colours folders, prompt and log lines the way ls --color does: colors on | off." },
    { "cp",       "Copies a file or folder (-r), including between this computer and the Flipper. Wildcards (*.sub) work as a source. Every transfer is MD5-verified." },
    { "df",       "Shows free and used space on the storage." },
    { "du",       "Adds up how much space a folder uses, broken down by what is inside it." },
    { "echo",     "Echoes bytes back; with a redirect it writes to a file on the Flipper instead: echo hello > /ext/notes.txt (>> appends)." },
    { "edit",     "Opens a Flipper file for editing on this computer." },
    { "find",     "Finds files under a folder by name, e.g. find *.sub /ext/subghz." },
    { "grep",     "Prints the lines of a file that contain some text: grep nonce /ext/nfc/card.nfc." },
    { "head",     "Prints the first lines of a file: head [-n 20] <file>." },
    { "history",  "Lists the commands typed this session." },
    { "ls",       "Lists the files and folders in a directory." },
    { "md5",      "Prints a file's MD5 hash." },
    { "mkdir",    "Creates a folder." },
    { "mv",       "Moves or renames a file or folder." },
    { "open",     "Launches an app on the Flipper, e.g. open NFC." },
    { "pwd",      "Prints the current folder." },
    { "reboot",   "Restarts the Flipper." },
    { "rm",       "Deletes a file, or a folder and everything in it (-r). Wildcards (*.sub) work too." },
    { "shutdown", "Powers the Flipper off." },
    { "stat",     "Shows the size and type of a file or folder." },
    { "tree",     "Lists everything under a folder, recursively." },
    { "tail",     "Prints the last lines of a file: tail [-n 20] <file>." },
    { "tgz",      "Packs a local folder into a .tgz, the same way Backup does: tgz <folder> [archive.tgz]." },
    { "touch",    "Creates an empty file on the Flipper." },
    { "wc",       "Counts the lines, words and bytes in a file." },
    { "wget",     "Downloads a URL on this computer and saves it straight onto the Flipper: wget <url> [destination]. The Flipper has no network of its own." },
    { "sed",      "Find-and-replace inside a file: sed s/old/new/g <file>. Overwrites the file with the result." },
    { "diff",     "Shows what differs between two files, line by line: diff <a> <b>." },
    { "file",     "Identifies what a file is from its contents: file <path>." },
    { "locate",   "Searches the whole SD card for a name: locate <text>. (find rooted at /ext.)" },
    { "ping",     "Pings a host FROM THIS COMPUTER (the Flipper has no network): ping <host>." },
    { "ip",       "Shows THIS COMPUTER's network interfaces (the Flipper has none)." },
    { "uname",    "Shows THIS COMPUTER's system info. For the Flipper's own, use device_info." },
    { "pm3",      "Runs a Proxmark3 command via the pm3 client on THIS computer (Proxmark must be plugged into the Mac): pm3 hf search, pm3 lf search, etc." },
    { "verbose",  "Shows or hides the wire-level log of everything a command runs: verbose on | off." },
};

// Second spellings people reach for out of habit. "help <alias>" answers with
// the command it points at rather than repeating the description.
struct CliAlias { const char *name; const char *target; };
const CliAlias kCliAliases[] = {
    { "?",        "help" },      { "buzz",     "vibro" },
    { "cls",      "clear" },     { "del",      "rm" },
    { "diskfree", "df" },        { "dir",      "ls" },
    { "halt",     "shutdown" },  { "hostname", "device_info" },
    { "ll",       "ls" },        { "md",       "mkdir" },
    { "md5sum",   "md5" },       { "mount",    "df" },
    { "poweroff", "shutdown" },  { "pull",     "cp" },
    { "push",     "cp" },        { "read",     "cat" },
    { "restart",  "reboot" },    { "vibrate",  "vibro" },
    { "whoami",   "device_info" },   { "curl",     "wget" },
    { "fetch",    "wget" },
};

QStringList cliOurNames()
{
    QStringList out;
    for (const CliCmd &c : kCliCommands) { out += QLatin1String(c.name); }
    return out;
}

QString cliHelpFor(const QString &name)
{
    for (const CliCmd &c : kCliCommands) {
        if (name == QLatin1String(c.name)) { return QLatin1String(c.help); }
    }
    return QString();
}

// One-line descriptions for the firmware's own commands, from the official CLI
// reference (docs.flipper.net/zero/development/cli), so "help <command>" answers
// for everything on the list instead of going quiet.
const CliCmd kFwCommands[] = {
    { "!",                 "Alias for <info device>." },
    { "bt",                "Bluetooth test app -- reads the BLE HCI version." },
    { "buzzer",            "Plays a frequency or a musical note on the piezo speaker." },
    { "crypto",            "Encrypts and decrypts text using keys in the secure enclave." },
    { "date",              "Shows or sets the date and time." },
    { "device_info",       "Alias for <info device> (obsolete)." },
    { "echo",              "Echoes back whatever bytes it receives." },
    { "exit",              "Leaves the CLI shell -- useful inside a secondary shell." },
    { "factory_reset",     "Resets the device to factory settings; the microSD is kept." },
    { "free",              "Shows heap memory allocator information." },
    { "free_blocks",       "Shows free heap blocks and their sizes, for fragmentation." },
    { "gpio",              "Sets pin mode and reads or writes GPIO pin state." },
    { "help",              "Lists the available commands." },
    { "i2c",               "Scans the I2C bus for devices." },
    { "ikey",              "Reads, emulates and writes iButton keys." },
    { "info",              "Shows detailed device and power system information." },
    { "input",             "Shows button presses and injects input events." },
    { "ir",                "Reads and sends infrared signals." },
    { "js",                "Runs a JavaScript file and prints its console output." },
    { "led",               "Sets the status LED colour and the display backlight." },
    { "loader",            "Lists, opens and closes applications." },
    { "log",               "Streams the system log; Ctrl-C stops it." },
    { "neofetch",          "Prints system info, neofetch style." },
    { "nfc",               "Opens the NFC shell to read and emulate cards." },
    { "onewire",           "Scans the 1-Wire bus for devices." },
    { "power",             "Powers off, reboots, and switches GPIO power rails." },
    { "reload_ext_cmds",   "Reloads the external commands stored on the microSD." },
    { "rfid",              "Reads, writes and emulates low-frequency RFID cards." },
    { "start_rpc_session", "Switches the CLI into protobuf RPC mode." },
    { "storage",           "Filesystem commands under /int and /ext." },
    { "subghz",            "Sub-GHz tools: transmit, receive, decode and chat." },
    { "sysctl",            "Configures system settings such as debug and heap tracking." },
    { "top",               "Lists running threads in real time; Ctrl-C quits." },
    { "update",            "Installs updates and backs up or restores internal storage." },
    { "uptime",            "Shows the time since the last reboot." },
    { "vibro",             "Turns the vibration motor on (1) or off (0)." },
};

QString cliFwHelpFor(const QString &name)
{
    for (const CliCmd &c : kFwCommands) {
        if (name == QLatin1String(c.name)) { return QLatin1String(c.help); }
    }
    return QString();
}

QString cliAliasTarget(const QString &name)
{
    for (const CliAlias &a : kCliAliases) {
        if (name == QLatin1String(a.name)) { return QLatin1String(a.target); }
    }
    return QString();
}

// Every name the CLI answers to -- ours, the habit spellings, and the
// firmware's own -- for Tab completion of the first word.
QStringList cliAllCommandNames()
{
    QStringList out;
    for (const CliCmd &c : kCliCommands)   { out += QLatin1String(c.name); }
    for (const CliAlias &a : kCliAliases)  { out += QLatin1String(a.name); }
    for (const CliCmd &c : kFwCommands)    { out += QLatin1String(c.name); }
    out.removeAll(QStringLiteral("!"));
    out.removeAll(QStringLiteral("?"));
    out.removeDuplicates();
    std::sort(out.begin(), out.end(), [](const QString &a, const QString &b) {
        return a.compare(b, Qt::CaseInsensitive) < 0;
    });
    return out;
}

// The Flipper's own filesystem is /ext (SD), /int (internal) and /any.
bool cliIsDevicePath(const QString &p)
{
    return p.startsWith(QLatin1String("/ext"), Qt::CaseInsensitive)
        || p.startsWith(QLatin1String("/int"), Qt::CaseInsensitive)
        || p.startsWith(QLatin1String("/any"), Qt::CaseInsensitive);
}

// A storage root, not a folder inside one. "storage stat /ext" answers with the
// volume ("Storage, label: ...") instead of "Directory", which is why a plain
// "cd .." out of /ext/nfc used to be rejected as a missing folder.
bool cliIsStorageRoot(const QString &p)
{
    QString s = p;
    while (s.size() > 1 && s.endsWith(QLatin1Char('/'))) { s.chop(1); }
    return s.compare(QLatin1String("/ext"), Qt::CaseInsensitive) == 0
        || s.compare(QLatin1String("/int"), Qt::CaseInsensitive) == 0
        || s.compare(QLatin1String("/any"), Qt::CaseInsensitive) == 0;
}

// One rule, so a single "cp" can serve both machines: "~..." and absolute paths
// outside /ext, /int, /any live on this computer. Everything else -- including
// every bare relative name -- belongs to the Flipper, because that's whose shell
// the prompt is.
bool cliIsHostPath(const QString &p)
{
    if (p.startsWith(QLatin1Char('~')))  { return true; }
    if (p.startsWith(QLatin1Char('/')))  { return !cliIsDevicePath(p); }
    return false;
}

// Resolve a Flipper-side argument against the current folder, so "cd nfc",
// "ls ..", "cat card.nfc" all behave the way a Linux shell would.
QString cliResolvePath(const QString &cwd, const QString &arg)
{
    if (arg.isEmpty() || arg == QLatin1String(".")) { return cwd; }
    if (arg == QLatin1String("~"))                  { return QStringLiteral("/ext"); }
    QString s = arg;
    if (s.startsWith(QLatin1String("~/")))  { s = QStringLiteral("/ext") + s.mid(1); }
    if (!s.startsWith(QLatin1Char('/')))    { s = cwd + QLatin1Char('/') + s; }
    s = QDir::cleanPath(s);
    return s.isEmpty() ? QStringLiteral("/") : s;
}

// The same Linux-style shortcuts the panel offers, for the one-shot path the
// assistant uses. send() does this inline, but that version is bound to panel
// state -- current folder, capture buffers, transfer chains -- and the assistant
// has no panel. So this is the subset that is genuinely one command in, one
// command out, with relative paths resolved against /ext. Multi-step commands
// (cp to/from this computer, find, rm -r, edit) stay panel-only, and anything
// unrecognised passes through untouched so raw firmware commands still work.
QString cliOneShotTranslate(const QString &cmd)
{
    const QStringList parts = cmd.trimmed().split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
    if (parts.isEmpty()) { return cmd; }

    QString verb = parts.first().toLower();
    QStringList a = parts.mid(1);
    a.removeAll(QStringLiteral("-r")); a.removeAll(QStringLiteral("-R"));
    a.removeAll(QStringLiteral("-l")); a.removeAll(QStringLiteral("-a"));
    a.removeAll(QStringLiteral("-la")); a.removeAll(QStringLiteral("-al"));
    a.removeAll(QStringLiteral("-h")); a.removeAll(QStringLiteral("-f"));

    const QString canon = cliAliasTarget(verb);
    if (!canon.isEmpty()) { verb = canon; }

    const QString root = QStringLiteral("/ext");
    auto here = [&root](const QString &arg) { return cliResolvePath(root, arg); };
    const QString p0 = a.value(0);
    const QString p1 = a.value(1);

    if (verb == QLatin1String("ls"))    { return QStringLiteral("storage list ") + here(p0); }
    if (verb == QLatin1String("tree"))  { return QStringLiteral("storage tree ") + here(p0); }
    if (verb == QLatin1String("df"))    { return QStringLiteral("storage info ") + here(p0); }
    if (verb == QLatin1String("cat"))   { return p0.isEmpty() ? cmd : QStringLiteral("storage read ")   + here(p0); }
    if (verb == QLatin1String("stat"))  { return p0.isEmpty() ? cmd : QStringLiteral("storage stat ")   + here(p0); }
    if (verb == QLatin1String("md5"))   { return p0.isEmpty() ? cmd : QStringLiteral("storage md5 ")    + here(p0); }
    if (verb == QLatin1String("mkdir")) { return p0.isEmpty() ? cmd : QStringLiteral("storage mkdir ")  + here(p0); }
    if (verb == QLatin1String("rm"))    { return p0.isEmpty() ? cmd : QStringLiteral("storage remove ") + here(p0); }
    if (verb == QLatin1String("mv"))    {
        return p1.isEmpty() ? cmd
                            : QStringLiteral("storage rename ") + here(p0) + QLatin1Char(' ') + here(p1);
    }
    if (verb == QLatin1String("reboot"))   { return QStringLiteral("power reboot"); }
    if (verb == QLatin1String("shutdown")) { return QStringLiteral("power off"); }
    if (verb == QLatin1String("vibro") && a.isEmpty()) { return QStringLiteral("vibro 1"); }
    if (verb == QLatin1String("open") && !a.isEmpty()) {
        return QStringLiteral("loader open ") + a.join(QLatin1Char(' '));
    }

    // An alias that folds onto a plain firmware command (whoami -> device_info)
    // still has to go out under its canonical name.
    if (!canon.isEmpty()) {
        QString out = verb;
        if (!a.isEmpty()) { out += QLatin1Char(' ') + a.join(QLatin1Char(' ')); }
        return out;
    }
    return cmd.trimmed();
}

QString cliExpandHostPath(const QString &p)
{
    QString s = p;
    if (s == QLatin1String("~"))           { return QDir::homePath(); }
    if (s.startsWith(QLatin1String("~/"))) { s = QDir::homePath() + s.mid(1); }
    if (!s.startsWith(QLatin1Char('/')))   { s = QDir::homePath() + QLatin1Char('/') + s; }
    return QDir::cleanPath(s);
}

bool cliLooksLikeDir(const QString &dst)
{
    if (dst.endsWith(QLatin1Char('/'))) { return true; }
    return !dst.section(QLatin1Char('/'), -1).contains(QLatin1Char('.'));
}

QString cliJoinDest(const QString &dst, const QString &srcName, bool dstIsDir)
{
    QString d = dst;
    while (d.size() > 1 && d.endsWith(QLatin1Char('/'))) { d.chop(1); }
    return dstIsDir ? (d + QLatin1Char('/') + srcName) : d;
}

// One line of "storage tree": possibly tab-indented, then "[D] /abs/path" or
// "[F] /abs/path 1234b". Same row shape as "storage list" (see
// cliFormatListing below) except tree's path column is already absolute.
struct CliTreeEntry { QString path; bool isDir; qint64 size; };

QList<CliTreeEntry> cliParseTree(const QString &raw)
{
    static const QRegularExpression rowRe(QStringLiteral("\\[([DF])\\]\\s+(\\S+)(?:\\s+(\\d+)b)?"));
    QList<CliTreeEntry> out;
    const QStringList lines = raw.split(QLatin1Char('\n'));
    for (const QString &line : lines) {
        const auto m = rowRe.match(line);
        if (!m.hasMatch()) { continue; }
        CliTreeEntry e;
        e.isDir = (m.captured(1) == QLatin1String("D"));
        e.path  = m.captured(2);
        e.size  = m.captured(3).isEmpty() ? -1 : m.captured(3).toLongLong();
        out += e;
    }
    return out;
}

// Pulls the first 32-hex-char token out of a "storage md5" reply.
QString cliExtractMd5(const QString &raw)
{
    static const QRegularExpression re(QStringLiteral("\\b[0-9a-fA-F]{32}\\b"));
    const auto m = re.match(raw);
    return m.hasMatch() ? m.captured(0).toLower() : QString();
}

// Two labelled sections, one shared column width. The firmware's own layout is
// discarded entirely so neither half can drift out of line with the other.
QString cliFormatHelp(const QString &raw, const QString &promptText)
{
    // Stock firmware prints "Commands available:", Unleashed/Momentum print
    // "Available commands:". Only the first was matched, so on a fork this
    // whole function bailed out on line one and the listing was passed through
    // untouched -- which is why cd, ls, grep and the rest were missing from it.
    static const QRegularExpression header(
        QStringLiteral("(?:commands available|available commands)\\s*:"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch hm = header.match(raw);
    if (!hm.hasMatch()) { return raw; }
    const int hdr = hm.capturedStart();

    int listStart = raw.indexOf(QLatin1Char('\n'), hdr);
    listStart = (listStart < 0) ? hm.capturedEnd() : (listStart + 1);

    static const QRegularExpression ws(QStringLiteral("\\s+"));
    QStringList stock;
    int listEnd = listStart;
    int pos = listStart;
    while (pos < raw.size()) {
        const int eol = raw.indexOf(QLatin1Char('\n'), pos);
        const int lineEnd = (eol < 0) ? raw.size() : eol;
        const QString line = raw.mid(pos, lineEnd - pos).trimmed();
        // Stop at the prompt. It is no longer ">: " -- we rewrite it -- and if
        // the parser runs past it, "Nikita@qflipper", "~" and "%" get sorted
        // into the command list and the prompt itself is eaten.
        if (line.isEmpty() || line.startsWith(QLatin1String(">:"))) { break; }
        if (!promptText.isEmpty() && line.startsWith(promptText)) { break; }
        // The firmware follows the grid with two sentences of prose ("If you
        // added a new external command...", "Find out more: <url>"). Without
        // this they were split on whitespace and sorted into the command list.
        // A command row is nothing but bare names.
        static const QRegularExpression nameRe(QStringLiteral("^[A-Za-z0-9_!?.-]{1,24}$"));
        const QStringList tokens = line.split(ws, Qt::SkipEmptyParts);
        bool allNames = !tokens.isEmpty();
        for (const QString &t : tokens) {
            if (!nameRe.match(t).hasMatch()) { allNames = false; break; }
        }
        if (!allNames) { break; }
        stock += tokens;
        if (eol < 0) { listEnd = raw.size(); break; }
        pos = eol + 1;
        listEnd = pos;
    }
    if (stock.isEmpty()) { return raw; }

    QStringList ours = cliOurNames();
    for (const QString &n : ours) { stock.removeAll(n); }
    stock.removeDuplicates();
    std::sort(stock.begin(), stock.end(), [](const QString &a, const QString &b) {
        return a.compare(b, Qt::CaseInsensitive) < 0;
    });

    int colW = 0;
    for (const QString &n : ours)  { colW = qMax(colW, n.size()); }
    for (const QString &n : stock) { colW = qMax(colW, n.size()); }
    colW += 4;

    auto grid = [colW](const QStringList &names) {
        QString out;
        const int rows = (names.size() + 1) / 2;
        for (int r = 0; r < rows; ++r) {
            QString row = names.at(r);
            const int right = r + rows;
            if (right < names.size()) {
                while (row.size() < colW) { row += QLatin1Char(' '); }
                row += names.at(right);
            }
            out += row + QLatin1Char('\n');
        }
        return out;
    };

    QString block;
    block += QStringLiteral("  (help <command> for details)\n");
    block += QStringLiteral("------ New Commands ------\n");
    block += grid(ours);
    block += QStringLiteral("\n------ Original Commands ------\n");
    block += grid(stock);

    QString out = raw;
    out.replace(listStart, listEnd - listStart, block);

    // Drop the firmware's two trailing sentences. "reload_ext_cmds" is still in
    // the listing above for anyone who needs it, and the docs link is already a
    // line of the banner on connect -- repeating both after every `help` is
    // noise in a panel this size.
    static const QRegularExpression trailer(
        QStringLiteral("(?m)^[ \\t]*(?:If you added a new external command.*|Find out more:.*)$\\n?"));
    out.remove(trailer);
    // A colour reset that arrived split across two reads leaves its tail behind
    // as literal text.
    out.remove(QRegularExpression(QStringLiteral("(?m)^\\[[0-9;]*m$\\n?")));
    out.replace(QRegularExpression(QStringLiteral("\\[[0-9;]*m")), QString());
    return out;
}


// Reformat "storage list" into something closer to ls: folders first with a
// trailing slash, files after them, sizes right-aligned and human readable. No
// type column -- the slash already says which is which, and a bare "-" is only
// half of the "ls -l" convention it borrows from.
QString cliFormatListing(const QString &raw)
{
    static const QRegularExpression rowRe(
        QStringLiteral("^\\[([DF])\\]\\s+(.*?)(?:\\s+(\\d+)b)?$"));

    const QStringList lines = raw.split(QLatin1Char('\n'));
    QStringList dirs, files, sizes, passthrough;
    QString head, tail;
    bool sawRow = false;

    for (const QString &line : lines) {
        const QString t = line.trimmed();
        const auto m = rowRe.match(t);
        if (m.hasMatch()) {
            sawRow = true;
            if (m.captured(1) == QLatin1String("D")) {
                dirs += m.captured(2);
            } else {
                files += m.captured(2);
                sizes += m.captured(3);
            }
        } else if (!sawRow) {
            head += line + QLatin1Char('\n');
        } else {
            tail += line + QLatin1Char('\n');
        }
    }
    if (!sawRow) { return raw; }

    // Keep each file glued to its size while sorting.
    QList<QPair<QString, QString>> fileRows;
    for (int i = 0; i < files.size(); ++i) { fileRows.append({ files.at(i), sizes.value(i) }); }
    auto byName = [](const QString &a, const QString &b) { return a.compare(b, Qt::CaseInsensitive) < 0; };
    std::sort(dirs.begin(), dirs.end(), byName);
    std::sort(fileRows.begin(), fileRows.end(),
              [&byName](const QPair<QString, QString> &a, const QPair<QString, QString> &b) {
                  return byName(a.first, b.first);
              });

    auto human = [](const QString &bytes) {
        if (bytes.isEmpty()) { return QString(); }
        const qint64 n = bytes.toLongLong();
        if (n < 1024)             { return QStringLiteral("%1 B").arg(n); }
        if (n < 1024LL * 1024)    { return QStringLiteral("%1 KB").arg(n / 1024.0, 0, 'f', 1); }
        if (n < 1024LL * 1024 * 1024) { return QStringLiteral("%1 MB").arg(n / (1024.0 * 1024), 0, 'f', 1); }
        return QStringLiteral("%1 GB").arg(n / (1024.0 * 1024 * 1024), 0, 'f', 1);
    };

    int nameW = 0;
    for (const QString &d : dirs) { nameW = qMax(nameW, d.size() + 1); }   // +1 for the trailing slash
    for (const auto &f : fileRows) { nameW = qMax(nameW, f.first.size()); }
    nameW += 2;

    int sizeW = 0;
    for (const auto &f : fileRows) { sizeW = qMax(sizeW, human(f.second).size()); }

    QString out = head;
    for (const QString &d : dirs) {
        out += d + QLatin1Char('/') + QLatin1Char('\n');
    }
    for (const auto &f : fileRows) {
        QString row = f.first;
        const QString sz = human(f.second);
        if (!sz.isEmpty()) {
            while (row.size() < nameW) { row += QLatin1Char(' '); }
            QString pad = sz;
            while (pad.size() < sizeW) { pad.prepend(QLatin1Char(' ')); }
            row += pad;
        }
        out += row + QLatin1Char('\n');
    }
    out += tail;
    // split() handed us the trailing prompt as its own element and the loop
    // above put a newline after it, which the raw text never had. That stray
    // newline is what dropped the caret onto the line below the prompt.
    if (!raw.endsWith(QLatin1Char('\n')) && out.endsWith(QLatin1Char('\n'))) { out.chop(1); }
    return out;
}

// ---- prompt ----------------------------------------------------------------
// "Nikita@qflipper ~/nfc % " -- the folder shows in the prompt, the way a shell
// does, so cd doesn't need to announce itself.
QString cliPromptFor(const QString &devName, const QString &cwd)
{
    QString where = cwd;
    if (where == QLatin1String("/ext"))            { where = QStringLiteral("~"); }
    else if (where.startsWith(QLatin1String("/ext/"))) { where = QStringLiteral("~") + where.mid(4); }
    const QString who = devName.isEmpty() ? QStringLiteral("flipper") : devName;
    // A bare "%" is safe here: QString::arg only consumes % followed by a digit.
    return QStringLiteral("%1@qflipper %2 % ").arg(who, where);
}

// ---- file transfer, as pure steps so the protocol can be exercised offline ---
struct CliXferStep {
    bool done = false;
    bool failed = false;
    QByteArray toWrite;   // raw bytes to push at the port
    QByteArray body;      // downloaded contents (download only)
    QString message;      // line to show the user
};

// storage write_chunk <path> <n> answers "Ready", then swallows exactly n bytes.
CliXferStep cliUploadFeed(QByteArray &raw, const QByteArray &chunk,
                          const QByteArray &payload, const QString &label)
{
    CliXferStep r;
    raw += chunk;
    if (raw.contains("Ready")) {
        r.done = true;
        r.toWrite = payload;
        r.message = QStringLiteral("[ sent %1 bytes -> %2 ]").arg(payload.size()).arg(label);
        raw.clear();
    } else if (raw.contains("error") || raw.size() > 4096) {
        r.done = true;
        r.failed = true;
        r.message = QStringLiteral("[ upload refused: %1 ]").arg(QString::fromUtf8(raw).trimmed());
        raw.clear();
    }
    return r;
}

// storage read <path> prints "Size: n" and then the raw contents.
CliXferStep cliDownloadFeed(QByteArray &raw, qint64 &size, const QByteArray &chunk,
                            const QString &hostDst)
{
    CliXferStep r;
    raw += chunk;
    if (size < 0) {
        const int c = raw.indexOf("Size:");
        if (c < 0) {
            if (raw.contains("error") || raw.size() > 4096) {
                r.done = true;
                r.failed = true;
                r.message = QStringLiteral("[ download failed: %1 ]").arg(QString::fromUtf8(raw).trimmed());
                raw.clear();
            }
            return r;
        }
        const int nl = raw.indexOf('\n', c);
        if (nl < 0) { return r; }
        size = raw.mid(c + 5, nl - c - 5).trimmed().toLongLong();
        raw = raw.mid(nl + 1);
    }
    if (raw.size() < size) { return r; }
    r.done = true;
    r.body = raw.left(int(size));
    r.message = QStringLiteral("[ saved %1 bytes -> %2 ]").arg(r.body.size()).arg(hostDst);
    raw.clear();
    return r;
}

}   // namespace

FlipperCli::FlipperCli(QObject *parent)
    : QObject(parent)
{
}

void FlipperCli::setOpen(bool value)
{
    if (m_open == value) { return; }

    // The CLI and the RPC session share one serial port, so connecting here
    // tears down whatever RPC work is in flight. A backup is minutes of
    // transfers; letting a click destroy it -- and only finding out from a
    // "session was stopped" line in the log -- is not a fair trade.
    if (value && g_transferRunning) {
        setStatus(QStringLiteral("A backup is running -- the CLI would cut its connection."));
        emit openChanged();      // let the UI snap the toggle back
        return;
    }

    m_open = value;
    emit openChanged();

    if (m_open) { connectCli(); }
    else        { disconnectCli(); }
}

void FlipperCli::connectCli()
{
    clearOutput();

    if (!m_appBackend) { setStatus(QStringLiteral("Backend unavailable.")); return; }

    auto *reg = m_appBackend->deviceRegistry();
    auto *dev = reg ? reg->currentDevice() : nullptr;
    if (!dev) {
        setStatus(QStringLiteral("Connect a Flipper over USB first."));
        return;
    }

    const auto &info = dev->deviceState()->deviceInfo();
    if (info.isBle || info.portInfo.isNull()) {
        setStatus(QStringLiteral("CLI is USB-only for now (this device is wireless)."));
        return;
    }
    const QSerialPortInfo portInfo = info.portInfo;
    // Name for the prompt: the device's own name, or the one baked into the
    // serial port (…usbmodemflip_Nikita1) if the field comes back empty.
    QString devName = info.name;
    if (devName.isEmpty()) {
        const QString pn = portInfo.portName();
        const int f = pn.indexOf(QLatin1String("flip_"));
        if (f >= 0) {
            devName = pn.mid(f + 5);
            while (!devName.isEmpty() && devName.back().isDigit()) { devName.chop(1); }
        }
    }

    // Hand the serial line off from RPC to us: releasePort() stops the RPC
    // session, which closes the COM port and drops the Flipper back to its CLI.
    setStatus(QStringLiteral("Pausing qFlipper's session and opening the CLI…"));
    // (device session pauses silently while the CLI is open)
    m_appBackend->releasePort();

    // Give the RPC teardown a moment to actually free the port, then take it over.
    QTimer::singleShot(700, this, [this, portInfo, devName]() {
        if (!m_open) { return; }   // user closed the CLI again before we got here

        m_port = new QSerialPort(portInfo, this);
        m_port->setBaudRate(230400);
        m_port->setDataBits(QSerialPort::Data8);
        m_port->setParity(QSerialPort::NoParity);
        m_port->setStopBits(QSerialPort::OneStop);
        m_port->setFlowControl(QSerialPort::NoFlowControl);

        if (!m_port->open(QIODevice::ReadWrite)) {
            appendOutput(QStringLiteral("[ couldn't open %1: %2 ]\n").arg(portInfo.portName(), m_port->errorString()));
            setStatus(QStringLiteral("Couldn't open the port -- close and retry."));
            m_port->deleteLater();
            m_port = nullptr;
            return;
        }

        connect(m_port, &QSerialPort::readyRead, this, &FlipperCli::onReadyRead);
        m_cwd = QStringLiteral("/ext");
        m_devName = devName;
        emit promptChanged();
        m_cdPrev = m_cwd;
        m_cdPending.clear();
        m_cdRaw.clear();
        m_xfer = Xfer::None;
        m_rawCb = nullptr;
        m_xferChain = nullptr;
        m_capture = Capture::None;
        m_captureBuf.clear();
        m_echoPending.clear();
        m_escTail.clear();
        setActive(true);
        setStatus(QStringLiteral("CLI live -- type a command (try 'help')."));

        // The firmware prints its banner and a prompt by itself the moment the
        // CDC port is opened. Writing "\r\n" here unconditionally asked it for a
        // second one, which is the duplicate "name@qflipper ~ %" line sitting
        // above the caret on every connect. Nudge only if it stayed silent --
        // which does happen when the device was already parked at a prompt and
        // has nothing new to announce.
        m_sawDeviceBytes = false;
        QTimer::singleShot(700, this, [this]() {
            if (m_port && m_active && !m_sawDeviceBytes) { m_port->write("\r\n"); }
        });
    });
}

void FlipperCli::disconnectCli()
{
    resetTransientState();
    if (m_port) {
        m_port->close();
        m_port->deleteLater();
        m_port = nullptr;
    }
    setActive(false);
    setStatus(QString());

    // Hand the line back to qFlipper's normal RPC session.
    if (m_appBackend) { m_appBackend->reacquirePort(); }
}

// ---- one-shot CLI run for the assistant (isolated from the interactive panel) --
void FlipperCli::runOneShot(const QString &cmd, std::function<void(bool, QString)> done)
{
    if (m_open || m_active) { done(false, QStringLiteral("The CLI panel is open -- close it first.")); return; }
    if (m_runBusy)          { done(false, QStringLiteral("A CLI command is already running.")); return; }
    if (!m_appBackend)      { done(false, QStringLiteral("Backend unavailable.")); return; }

    auto *reg = m_appBackend->deviceRegistry();
    auto *dev = reg ? reg->currentDevice() : nullptr;
    if (!dev) { done(false, QStringLiteral("Connect a Flipper over USB first.")); return; }
    const auto &info = dev->deviceState()->deviceInfo();
    if (info.isBle || info.portInfo.isNull()) { done(false, QStringLiteral("CLI is USB-only.")); return; }
    const QSerialPortInfo portInfo = info.portInfo;

    // The assistant is told it can type "ls /ext/nfc"; make that true. Raw
    // firmware commands are returned unchanged, so both spellings work.
    const QString wire = cliOneShotTranslate(cmd);

    m_runBusy = true;
    m_runBuf.clear();
    m_runDone = std::move(done);

    // Idle timer: once output stops arriving for a beat, the command is done.
    if (!m_runIdle) {
        m_runIdle = new QTimer(this);
        m_runIdle->setSingleShot(true);
        m_runIdle->setInterval(700);
        connect(m_runIdle, &QTimer::timeout, this, [this]() { finishOneShot(true, m_runBuf); });
    }
    // Hard guard: never hang forever.
    if (!m_runGuard) {
        m_runGuard = new QTimer(this);
        m_runGuard->setSingleShot(true);
        m_runGuard->setInterval(6000);
        connect(m_runGuard, &QTimer::timeout, this, [this]() { finishOneShot(true, m_runBuf); });
    }

    // Release RPC, wait for the port to free, then take it over briefly.
    m_appBackend->releasePort();
    QTimer::singleShot(700, this, [this, portInfo, wire]() {
        if (!m_runBusy) { return; }
        m_runPort = new QSerialPort(portInfo, this);
        m_runPort->setBaudRate(230400);
        m_runPort->setDataBits(QSerialPort::Data8);
        m_runPort->setParity(QSerialPort::NoParity);
        m_runPort->setStopBits(QSerialPort::OneStop);
        m_runPort->setFlowControl(QSerialPort::NoFlowControl);
        if (!m_runPort->open(QIODevice::ReadWrite)) {
            const QString err = m_runPort->errorString();
            m_runPort->deleteLater(); m_runPort = nullptr;
            finishOneShot(false, QStringLiteral("Couldn't open the port: %1").arg(err));
            return;
        }
        connect(m_runPort, &QSerialPort::readyRead, this, [this]() {
            if (!m_runPort) { return; }
            QString chunk = QString::fromUtf8(m_runPort->readAll());
            static const QRegularExpression ansi(QStringLiteral("\x1B\\[[0-9;?]*[A-Za-z]"));
            chunk.remove(ansi);
            chunk.remove(QLatin1Char('\r'));
            static const QRegularExpression ctrl(QStringLiteral("[\\x00-\\x08\\x0B\\x0C\\x0E-\\x1F\\x7F]"));
            chunk.remove(ctrl);
            m_runBuf += chunk;
            if (m_runIdle) { m_runIdle->start(); }   // reset idle countdown
        });
        m_runGuard->start();
        m_runPort->write(wire.toUtf8());
        m_runPort->write("\r\n");
        m_runIdle->start();
    });
}

void FlipperCli::finishOneShot(bool ok, const QString &out)
{
    if (!m_runBusy) { return; }
    if (m_runIdle)  { m_runIdle->stop(); }
    if (m_runGuard) { m_runGuard->stop(); }
    if (m_runPort) {
        m_runPort->close();
        m_runPort->deleteLater();
        m_runPort = nullptr;
    }
    m_runBusy = false;

    // Tidy the captured text: drop the echoed command line and the trailing prompt.
    QString text = out;
    text.remove(QRegularExpression(QStringLiteral("(^|\\n)>: *")));   // prompt lines
    text = text.trimmed();

    auto cb = m_runDone;
    m_runDone = nullptr;
    if (m_appBackend) { m_appBackend->reacquirePort(); }   // hand the line back to RPC
    if (cb) { cb(ok, text); }
}

void FlipperCli::send(const QString &cmd)
{
    m_lastTyped = cmd.trimmed();
    if (!m_lastTyped.isEmpty() && m_history.value(m_history.size() - 1) != m_lastTyped) {
        m_history += m_lastTyped;
        if (m_history.size() > 200) { m_history.removeFirst(); }
    }

    // "clear" / "cls" clears the on-screen CLI view (the Flipper firmware has no
    // clear command) instead of being sent to the device.
    const QString t = cmd.trimmed().toLower();
    if (t == QLatin1String("clear") || t == QLatin1String("cls")) {
        clearOutput();
        appendOutput(prompt());
        return;
    }
    // "verbose [on|off]" is ours too -- it decides how much of the plumbing
    // below shows up on screen, so it never reaches the device.
    {
        const QStringList vp = cmd.trimmed().split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
        if (!vp.isEmpty() && vp.first().toLower() == QLatin1String("verbose")) {
            const QString arg = vp.value(1).toLower();
            if (arg.isEmpty())                                                     { setVerbose(!m_verbose); }
            else if (arg == QLatin1String("on")  || arg == QLatin1String("1"))     { setVerbose(true); }
            else if (arg == QLatin1String("off") || arg == QLatin1String("0"))     { setVerbose(false); }
            else {
                appendOutput(cmd.trimmed() + QStringLiteral("\n[ usage: verbose on | off ]\n") + prompt());
                return;
            }
            appendOutput(cmd.trimmed()
                         + (m_verbose ? QStringLiteral("\n[ verbose on -- logging every step ]\n")
                                      : QStringLiteral("\n[ verbose off -- results only ]\n"))
                         + prompt());
            return;
        }
    }

    // "tgz <folder> [archive]" runs the backup's packing step on its own. It
    // exists because testing that step used to mean waiting for 1400+ files to
    // copy first -- now the two can be exercised separately.
    {
        const QStringList tp = cmd.trimmed().split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
        if (!tp.isEmpty() && tp.first().toLower() == QLatin1String("tgz")) {
            if (tp.size() < 2) {
                appendOutput(cmd.trimmed()
                             + QStringLiteral("\n[ usage: tgz <folder> [archive.tgz] ]\n")
                             + prompt());
                return;
            }
            auto expand = [](QString p) {
                if (p.startsWith(QLatin1Char('~'))) { p.replace(0, 1, QDir::homePath()); }
                return QDir::cleanPath(p);
            };
            const QString src = expand(tp.at(1));
            const QString out = tp.size() > 2 ? expand(tp.at(2))
                                              : QFileInfo(src).absolutePath() + QStringLiteral("/bkp.tgz");

            int packed = 0, skipped = 0;
            QString err;
            const bool ok = loteiWriteTgz(src, out, &packed, &skipped, &err);

            QString msg = cmd.trimmed() + QLatin1Char('\n');
            if (ok) {
                msg += QStringLiteral("[ %1 -> %2 ]\n[ %3 entr(y/ies)%4, %5 KB ]\n")
                       .arg(src, out).arg(packed)
                       .arg(skipped ? QStringLiteral(", %1 skipped").arg(skipped) : QString())
                       .arg(QFileInfo(out).size() / 1024);
            } else {
                msg += QStringLiteral("Storage error: %1\n").arg(err);
            }
            appendOutput(msg + prompt());
            return;
        }
    }

    // "colors [on|off]" is ours as well, and never reaches the device.
    {
        const QStringList cp = cmd.trimmed().split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
        if (!cp.isEmpty() && cp.first().toLower() == QLatin1String("colors")) {
            const QString arg = cp.value(1).toLower();
            if (arg.isEmpty())                                                 { setColored(!m_colored); }
            else if (arg == QLatin1String("on")  || arg == QLatin1String("1")) { setColored(true); }
            else if (arg == QLatin1String("off") || arg == QLatin1String("0")) { setColored(false); }
            else {
                appendOutput(cmd.trimmed() + QStringLiteral("\n[ usage: colors on | off ]\n") + prompt());
                return;
            }
            appendOutput(cmd.trimmed()
                         + (m_colored ? QStringLiteral("\n[ colors on ]\n")
                                      : QStringLiteral("\n[ colors off ]\n"))
                         + prompt());
            return;
        }
    }

    if (!m_port || !m_active) { return; }

    // Record what the user just did to the device. Placed after the local-only
    // commands above so "clear" and "colors" don't land in the log.
    {
        const QString verb = m_lastTyped.section(QRegularExpression(QStringLiteral("\\s+")), 0, 0).toLower();
        const QString canon = cliAliasTarget(verb);
        if (cliCommandMutates(canon.isEmpty() ? verb : canon)) {
            cliLog(m_lastTyped);
        }
    }

    // One conversation at a time. Every mode owns the serial line exclusively,
    // so a command typed mid-operation used to interleave its reply with the
    // one already in flight.
    if (busy()) {
        appendOutput(m_lastTyped + QStringLiteral("\n[ still working -- Ctrl-C to cancel ]\n") + prompt());
        return;
    }

    // Arm the help collector the moment we ask for it. Sniffing the reply for
    // "Commands available:" isn't enough -- that string can land split across
    // two serial chunks and the listing then slips through unformatted.
    {
        const QString h = cmd.trimmed().toLower();
        if (h == QLatin1String("help") || h == QLatin1String("?")) {
            m_capture = Capture::Help;
            m_captureBuf.clear();
            armGuard();
        }
    }

    // The terminal types straight into the view, so anything we answer locally
    // never gets echoed back by the firmware -- echo the typed line ourselves.
    auto echo = [](const QString &c) { return c.trimmed() + QLatin1Char('\n'); };

    // "echo <text> > <file>" and ">>" write to the Flipper's storage. The
    // firmware's own echo only parrots bytes back down the wire, so the redirect
    // form is entirely ours -- and it is handled here, before the tokeniser,
    // because splitting on whitespace would lose the spacing inside the text.
    {
        static const QRegularExpression redir(QStringLiteral("^echo\\s+(.*?)\\s*(>>|>)\\s*(\\S+)\\s*$"));
        const QRegularExpressionMatch m = redir.match(cmd.trimmed());
        if (m.hasMatch()) {
            QString text = m.captured(1);
            if (text.size() >= 2
                && ((text.startsWith(QLatin1Char('"'))  && text.endsWith(QLatin1Char('"')))
                 || (text.startsWith(QLatin1Char('\'')) && text.endsWith(QLatin1Char('\''))))) {
                text = text.mid(1, text.size() - 2);
            }
            appendOutput(echo(cmd));
            writeTextToDevice(text + QLatin1Char('\n'),
                              cliResolvePath(m_cwd, m.captured(3)),
                              m.captured(2) == QLatin1String(">>"));
            return;
        }
    }

    // ---- Linux-style shortcuts over the firmware's own commands ----
    // "help <name>" -> one short line for ours, or what an alias points at.
    {
        const QStringList hp = cmd.trimmed().split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
        if (hp.size() == 2 && (hp[0].toLower() == QLatin1String("help") || hp[0] == QLatin1String("?"))) {
            const QString c = hp[1].toLower();
            const QString alias = cliAliasTarget(c);
            if (!alias.isEmpty()) {
                appendOutput(echo(cmd) + QStringLiteral("Alias for <%1>\n").arg(alias) + prompt());
                return;
            }
            QString h = cliHelpFor(c);
            if (h.isEmpty()) { h = cliFwHelpFor(c); }
            if (!h.isEmpty()) {
                appendOutput(echo(cmd) + h + QStringLiteral("\n") + prompt());
                return;
            }
            // still unknown -> let the firmware answer (e.g. "led ?" style)
        }
    }

    QStringList parts = cmd.trimmed().split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
    if (!parts.isEmpty()) {
        QString verb = parts.first().toLower();
        QStringList a = parts.mid(1);
        // "-r" actually changes behaviour for cp/rm, so read it before the
        // generic flag-stripping below throws it away.
        // "-r" and "-f" change behaviour (rm), and the combined "-rf"/"-fr"
        // spelling arrives as one token -- so detect all of that before the
        // generic flag-stripping below discards it.
        const bool flagRecursive = a.contains(QStringLiteral("-r")) || a.contains(QStringLiteral("-R"))
                                 || a.contains(QStringLiteral("-rf")) || a.contains(QStringLiteral("-fr"))
                                 || a.contains(QStringLiteral("-rF")) || a.contains(QStringLiteral("-Rf"));
        const bool flagForce     = a.contains(QStringLiteral("-f")) || a.contains(QStringLiteral("-rf"))
                                 || a.contains(QStringLiteral("-fr")) || a.contains(QStringLiteral("-rF"))
                                 || a.contains(QStringLiteral("-Rf"));
        // strip common flags (we accept them for familiarity)
        a.removeAll(QStringLiteral("-r")); a.removeAll(QStringLiteral("-R"));
        a.removeAll(QStringLiteral("-rf")); a.removeAll(QStringLiteral("-fr"));
        a.removeAll(QStringLiteral("-rF")); a.removeAll(QStringLiteral("-Rf"));
        a.removeAll(QStringLiteral("-l")); a.removeAll(QStringLiteral("-a"));
        a.removeAll(QStringLiteral("-la")); a.removeAll(QStringLiteral("-al"));
        a.removeAll(QStringLiteral("-h")); a.removeAll(QStringLiteral("-f"));

        // Fold the habit spellings onto their canonical command up front, so the
        // dispatch below only ever deals with one name per behaviour.
        const QString canon = cliAliasTarget(verb);
        if (!canon.isEmpty()) { verb = canon; }

        auto usage = [this, &echo, cmd](const QString &u) { appendOutput(echo(cmd) + QStringLiteral("[ usage: %1 ]\n").arg(u) + prompt()); };
        auto here  = [this](const QString &arg) { return cliResolvePath(m_cwd, arg); };
        const QString p0 = a.value(0);
        const QString p1 = a.value(1);
        QString fw;   // the real firmware command to run

        if (verb == QLatin1String("cd")) {
            QString target;
            if (p0.isEmpty() || p0 == QLatin1String("~"))  { target = QStringLiteral("/ext"); }
            else if (p0 == QLatin1String("-"))             { target = m_cdPrev; }
            else                                           { target = here(p0); }
            if (target == m_cwd) { appendOutput(echo(cmd) + prompt()); return; }
            // "/" has no stat at all, and a storage root (/ext, /int, /any)
            // stats as a volume rather than a directory -- neither can be
            // confirmed the usual way, and both are always valid. This is what
            // made "cd .." out of /ext/<anything> report "no such folder".
            if (target == QLatin1String("/") || cliIsStorageRoot(target)) {
                const QString typed = echo(cmd);
                m_cdPrev = m_cwd;
                m_cwd = target;
                emit promptChanged();
                appendOutput(typed + prompt());   // prompt() must read the new folder
                return;
            }
            // Confirm it's really a folder before moving -- a silent bad cd would
            // quietly break every relative path typed after it.
            appendOutput(echo(cmd));
            m_cdPending = target;
            m_cdRaw.clear();
            writeLine(QStringLiteral("storage stat ") + target, false);
            armGuard();
            return;
        }
        if (verb == QLatin1String("pwd")) {
            appendOutput(echo(cmd) + m_cwd + QLatin1Char('\n') + prompt());
            return;
        }

        if (verb == QLatin1String("ls")) {
            fw = QStringLiteral("storage list ") + here(p0);
        } else if (verb == QLatin1String("tree")) {
            fw = QStringLiteral("storage tree ") + here(p0);
        } else if (verb == QLatin1String("cat")) {
            if (p0.isEmpty()) { usage(QStringLiteral("cat <file>")); return; }
            fw = QStringLiteral("storage read ") + here(p0);
        } else if (verb == QLatin1String("rm")) {
            if (p0.isEmpty()) { usage(QStringLiteral("rm [-r] <path>")); return; }
            const QString target = here(p0);
            if (target.contains(QLatin1Char('*')) || target.contains(QLatin1Char('?'))) {
                appendOutput(echo(cmd));
                expandDeviceGlob(target, [this](const QStringList &matches) {
                    if (matches.isEmpty()) { appendOutput(QStringLiteral("[ no matches ]\n") + prompt()); return; }
                    runRemoveQueue(matches);
                });
                return;
            }
            if (flagRecursive) {
                appendOutput(echo(cmd));
                // -f means don't stop to confirm. startRemoveTree already just
                // does it; the force flag exists so "rm -rf" reads normally and
                // so a future confirm-prompt for "rm -r" wouldn't apply here.
                Q_UNUSED(flagForce);
                startRemoveTree(target);
                return;
            }
            fw = QStringLiteral("storage remove ") + target;
        } else if (verb == QLatin1String("mkdir")) {
            if (p0.isEmpty()) { usage(QStringLiteral("mkdir <path>")); return; }
            fw = QStringLiteral("storage mkdir ") + here(p0);
        } else if (verb == QLatin1String("cp")) {
            if (p1.isEmpty()) { usage(QStringLiteral("cp [-r] <source> <destination>   -- works computer <-> Flipper, wildcards ok")); return; }
            const bool srcHost = cliIsHostPath(p0);
            const bool dstHost = cliIsHostPath(p1);
            if (!srcHost && (p0.contains(QLatin1Char('*')) || p0.contains(QLatin1Char('?')))) {
                const QString devGlob = here(p0);
                appendOutput(echo(cmd));
                const QString dstResolved = dstHost ? cliExpandHostPath(p1) : here(p1);
                if (dstHost) { QDir().mkpath(dstResolved); }
                expandDeviceGlob(devGlob, [this, dstResolved, dstHost](const QStringList &matches) {
                    if (matches.isEmpty()) { appendOutput(QStringLiteral("[ no matches ]\n") + prompt()); return; }
                    runCopyQueue(matches, dstResolved, dstHost);
                });
                return;
            }
            if (!srcHost && !dstHost) {
                fw = QStringLiteral("storage copy ") + here(p0) + QLatin1Char(' ') + here(p1);
            } else if (srcHost && !dstHost) {
                appendOutput(echo(cmd));
                const QString hostSrc = cliExpandHostPath(p0);
                if (flagRecursive || QFileInfo(hostSrc).isDir()) { startCopyUpTree(hostSrc, here(p1)); }
                else { uploadToFlipper(hostSrc, here(p1)); }
                return;
            } else if (!srcHost && dstHost) {
                appendOutput(echo(cmd));
                const QString devSrc = here(p0);
                const QString hostDst = cliExpandHostPath(p1);
                if (flagRecursive) { startCopyDownTree(devSrc, hostDst); }
                else { downloadFromFlipper(devSrc, hostDst); }
                return;
            } else {
                appendOutput(echo(cmd) + QStringLiteral("[ both paths are on this computer -- nothing for the Flipper to do ]\n") + prompt());
                return;
            }
        } else if (verb == QLatin1String("mv")) {
            if (p1.isEmpty()) { usage(QStringLiteral("mv <source> <destination>")); return; }
            fw = QStringLiteral("storage rename ") + here(p0) + QLatin1Char(' ') + here(p1);
        } else if (verb == QLatin1String("stat")) {
            if (p0.isEmpty()) { usage(QStringLiteral("stat <path>")); return; }
            fw = QStringLiteral("storage stat ") + here(p0);
        } else if (verb == QLatin1String("md5")) {
            if (p0.isEmpty()) { usage(QStringLiteral("md5 <file>")); return; }
            fw = QStringLiteral("storage md5 ") + here(p0);
        } else if (verb == QLatin1String("df")) {
            fw = QStringLiteral("storage info ") + here(p0);
        } else if (verb == QLatin1String("find")) {
            if (p0.isEmpty()) { usage(QStringLiteral("find <pattern> [path]   e.g. find *.sub /ext/subghz")); return; }
            appendOutput(echo(cmd));
            startFind(p1.isEmpty() ? m_cwd : here(p1), p0);
            return;
        } else if (verb == QLatin1String("grep")) {
            if (p1.isEmpty()) { usage(QStringLiteral("grep <text> <file>")); return; }
            appendOutput(echo(cmd));
            startGrep(p0, here(p1));
            return;
        } else if (verb == QLatin1String("head") || verb == QLatin1String("tail")) {
            // "-n" survives the generic flag strip above precisely because it
            // takes a value.
            int n = 10;
            QString file = p0;
            if (p0 == QLatin1String("-n")) {
                bool okN = false;
                const int v = p1.toInt(&okN);
                if (okN) { n = v; }
                file = a.value(2);
            }
            if (file.isEmpty()) { usage(QStringLiteral("%1 [-n lines] <file>").arg(verb)); return; }
            appendOutput(echo(cmd));
            startHeadTail(here(file), n, verb == QLatin1String("head"));
            return;
        } else if (verb == QLatin1String("wc")) {
            if (p0.isEmpty()) { usage(QStringLiteral("wc <file>")); return; }
            appendOutput(echo(cmd));
            startWc(here(p0));
            return;
        } else if (verb == QLatin1String("du")) {
            appendOutput(echo(cmd));
            startDu(p0.isEmpty() ? m_cwd : here(p0));
            return;
        } else if (verb == QLatin1String("touch")) {
            if (p0.isEmpty()) { usage(QStringLiteral("touch <file>")); return; }
            appendOutput(echo(cmd));
            writeTextToDevice(QString(), here(p0), false);
            return;
        } else if (verb == QLatin1String("wget")) {
            if (p0.isEmpty()) { usage(QStringLiteral("wget <url> [destination]   e.g. wget https://.../app.fap /ext/apps/Tools/")); return; }
            appendOutput(echo(cmd));
            // No destination -> keep the URL's own filename, drop it here.
            QString name = QUrl(p0).fileName();
            if (name.isEmpty()) { name = QStringLiteral("download.bin"); }
            // Either way we hand startWget a complete file path, so the upload
            // never has to guess whether the last segment is a folder.
            QString dst;
            if (p1.isEmpty()) {
                dst = cliResolvePath(m_cwd, name);
            } else if (p1.endsWith(QLatin1Char('/'))) {
                QString dir = cliResolvePath(m_cwd, p1);
                while (dir.size() > 1 && dir.endsWith(QLatin1Char('/'))) { dir.chop(1); }
                dst = dir + QLatin1Char('/') + name;
            } else {
                dst = here(p1);
            }
            startWget(p0, dst, true);
            return;
        } else if (verb == QLatin1String("sed")) {
            if (p1.isEmpty()) { usage(QStringLiteral("sed s/old/new/[g] <file>")); return; }
            appendOutput(echo(cmd));
            startSed(p0, here(p1));
            return;
        } else if (verb == QLatin1String("diff")) {
            if (p1.isEmpty()) { usage(QStringLiteral("diff <fileA> <fileB>")); return; }
            appendOutput(echo(cmd));
            startDiff(here(p0), here(p1));
            return;
        } else if (verb == QLatin1String("file")) {
            if (p0.isEmpty()) { usage(QStringLiteral("file <path>")); return; }
            appendOutput(echo(cmd));
            startFileType(here(p0));
            return;
        } else if (verb == QLatin1String("locate")) {
            if (p0.isEmpty()) { usage(QStringLiteral("locate <name>")); return; }
            appendOutput(echo(cmd));
            startLocate(p0);
            return;
        // ---- host-side system / network tools (run on THIS computer) --------
        } else if (verb == QLatin1String("pm3") || verb == QLatin1String("proxmark3")
                || verb == QLatin1String("proxmark")) {
            // The Proxmark3 is a SEPARATE device on this computer's USB, driven by
            // its own client. It is not something the Flipper runs -- so this is
            // the pm3 client executing on this Mac, output piped back here.
            //
            // pm3 is normally interactive (it opens its own "[usb] pm3 -->"
            // prompt); a one-command-per-line terminal can't drive that, so we
            // use its batch form: `pm3 -c "<command>"` runs one command and exits.
            // "pm3 hf search" here becomes `pm3 -c "hf search"` on the Mac.
            if (a.isEmpty()) {
                appendOutput(echo(cmd) + QStringLiteral(
                    "[ usage: pm3 <proxmark command>   e.g. pm3 hf search, pm3 lf search, pm3 hf mf autopwn.\n"
                    "  Runs the pm3 client on THIS computer -- the Proxmark3 must be plugged in here. ]\n") + prompt());
                return;
            }
            appendOutput(echo(cmd));
            const QString pmCmd = a.join(QLatin1Char(' '));
            // -c runs one command and exits. pm3 auto-detects the port; if the
            // user needs a specific one they can pass it: pm3 -p /dev/... (we
            // pass their args through after -c's command untouched isn't
            // possible with -c, so a bare command is the supported path).
            runHostProgram(QStringLiteral("pm3 -c \"%1\"").arg(pmCmd),
                           QStringLiteral("pm3"),
                           {QStringLiteral("-c"), pmCmd},
                           60000);   // Proxmark ops (autopwn, dictionary attacks) run long
            return;
        } else if (verb == QLatin1String("ping")) {
            if (p0.isEmpty()) { usage(QStringLiteral("ping <host>")); return; }
            appendOutput(echo(cmd));
            // -c 4 on Unix; the Flipper has no network so this is your Mac pinging.
            runHostProgram(QStringLiteral("ping %1").arg(p0), QStringLiteral("ping"),
                           {QStringLiteral("-c"), QStringLiteral("4"), p0}, 12000);
            return;
        } else if (verb == QLatin1String("ip") || verb == QLatin1String("ifconfig")) {
            appendOutput(echo(cmd));
            // The Flipper has no IP interfaces; show this computer's, which is
            // the only networking in the picture. `ip addr` on Linux, `ifconfig`
            // on macOS (no `ip` there by default).
            if (!QStandardPaths::findExecutable(QStringLiteral("ip")).isEmpty()) {
                runHostProgram(QStringLiteral("ip addr"), QStringLiteral("ip"), {QStringLiteral("addr")});
            } else {
                runHostProgram(QStringLiteral("ifconfig"), QStringLiteral("ifconfig"), {});
            }
            return;
        } else if (verb == QLatin1String("uname")) {
            // Linux prints the kernel here. The Flipper equivalent is its own
            // firmware/hardware identity -- read from the CLI, not invented.
            appendOutput(echo(cmd) + QStringLiteral(
                "[ the Flipper has no kernel; its firmware/hardware identity is 'device_info' "
                "or 'neofetch'. Showing this computer's uname: ]\n"));
            runHostProgram(QStringLiteral("uname -a"), QStringLiteral("uname"), {QStringLiteral("-a")});
            return;
        } else if (verb == QLatin1String("history")) {
            appendOutput(echo(cmd));
            if (m_history.isEmpty()) {
                appendOutput(QStringLiteral("[ nothing yet this session ]\n") + prompt());
            } else {
                QStringList rows;
                for (int i = 0; i < m_history.size(); ++i) {
                    rows += QStringLiteral("%1  %2").arg(i + 1, 4).arg(m_history.at(i));
                }
                appendOutput(rows.join(QLatin1Char('\n')) + QLatin1Char('\n') + prompt());
            }
            return;
        } else if (verb == QLatin1String("edit")) {
            if (p0.isEmpty()) { usage(QStringLiteral("edit <file>")); return; }
            appendOutput(echo(cmd));
            startEdit(here(p0));
            return;
        } else if (verb == QLatin1String("reboot")) {
            fw = QStringLiteral("power reboot");
        } else if (verb == QLatin1String("shutdown")) {
            fw = QStringLiteral("power off");
        } else if (verb == QLatin1String("open")) {
            // Join the rest: app names have spaces ("Bad USB", "Sub-GHz").
            if (a.isEmpty()) { usage(QStringLiteral("open <app>   e.g. open NFC")); return; }
            fw = QStringLiteral("loader open ") + a.join(QLatin1Char(' '));
        } else if (verb == QLatin1String("vibro") && a.isEmpty()) {
            fw = QStringLiteral("vibro 1");   // bare "vibro" isn't valid on its own
        }

        // An alias that folds onto a plain firmware command (whoami -> device_info)
        // still has to go out under its canonical name, not what was typed.
        if (fw.isEmpty() && !canon.isEmpty()) {
            fw = verb;
            if (!a.isEmpty()) { fw += QLatin1Char(' ') + a.join(QLatin1Char(' ')); }
        }

        if (!fw.isEmpty()) {
            // The user typed "ls"; the wire carries "storage list /ext". Show what
            // was typed and swallow the firmware's echo of the translation -- the
            // alias plumbing shouldn't be visible.
            if (fw != cmd.trimmed()) {
                appendOutput(echo(cmd));
                m_echoPending = fw + QLatin1Char('\n');
            }
            if (verb == QLatin1String("ls")) {
                m_capture = Capture::Listing;
                m_captureBuf.clear();
                armGuard();
            }
            writeLine(fw, false);
            return;
        }
    }

    writeLine(cmd.trimmed());
}

void FlipperCli::setVerbose(bool value)
{
    if (m_verbose == value) { return; }
    m_verbose = value;
    emit verboseChanged();
}

void FlipperCli::setColored(bool value)
{
    if (m_colored == value) { return; }
    m_colored = value;
    emit coloredChanged();
}

// ---- verbose log -----------------------------------------------------------
// A single "cp -r" or "rm -r" is dozens of firmware commands, and every one of
// them used to happen behind a summary line. With verbose on, each command and
// each reply lands in the terminal as it happens, indented so it reads as
// machinery rather than as output.
void FlipperCli::trace(const QString &what)
{
    if (!m_verbose || m_quiet) { return; }
    const QString s = what.trimmed();
    if (s.isEmpty() || s == m_lastTyped) { return; }
    appendOutput(QStringLiteral("  · ") + s + QLatin1Char('\n'));
}

void FlipperCli::traceReply(const QString &raw)
{
    if (!m_verbose || m_quiet) { return; }
    QString s = raw;
    static const QRegularExpression ansi(QStringLiteral("\x1B\\[[0-9;?]*[A-Za-z]"));
    s.remove(ansi);
    s.remove(QLatin1Char('\r'));
    static const QRegularExpression ctrl(QStringLiteral("[\\x00-\\x08\\x0B\\x0C\\x0E-\\x1F\\x7F]"));
    s.remove(ctrl);
    // A tree of a big folder can be tens of thousands of characters; the log is
    // there to follow along, not to replay the whole payload.
    const int total = s.size();
    if (total > 2000) { s = s.left(2000) + QStringLiteral("\n… (%1 more characters)").arg(total - 2000); }

    QString out;
    for (const QString &line : s.split(QLatin1Char('\n'))) {
        const QString l = line.trimmed();
        if (l.isEmpty() || l.startsWith(QLatin1String(">:"))) { continue; }
        if (l == m_lastTyped) { continue; }
        out += QStringLiteral("    \u00b7 ") + l + QLatin1Char('\n');
    }
    if (!out.isEmpty()) { appendOutput(out); }
}

bool FlipperCli::busy() const
{
    return m_xfer != Xfer::None || !m_cdPending.isEmpty() || m_capture != Capture::None;
}

void FlipperCli::armGuard()
{
    if (!m_opGuard) {
        m_opGuard = new QTimer(this);
        m_opGuard->setSingleShot(true);
        m_opGuard->setInterval(8000);
        connect(m_opGuard, &QTimer::timeout, this, &FlipperCli::onOpTimeout);
    }
    m_opGuard->start();
}

void FlipperCli::disarmGuard()
{
    if (m_opGuard) { m_opGuard->stop(); }
}

// Everything that is only true for the duration of one operation. Ctrl-C, the
// watchdog and disconnect all need exactly this set cleared, and keeping three
// copies of the list in sync is how one of them ends up missing a field.
void FlipperCli::resetTransientState()
{
    disarmGuard();
    m_cdPending.clear();
    m_cdRaw.clear();
    m_xfer = Xfer::None;
    m_xferRaw.clear();
    m_xferPayload.clear();
    m_xferSize = -1;
    m_rawCb = nullptr;
    m_xferChain = nullptr;
    if (m_captureFlush) { m_captureFlush->stop(); }
    m_capture = Capture::None;
    m_captureBuf.clear();
    m_echoPending.clear();
    m_escTail.clear();
    m_quiet = false;
}

void FlipperCli::onOpTimeout()
{
    const Xfer what = m_xfer;
    auto rawCb      = m_rawCb;
    auto chain      = m_xferChain;

    // Take the continuations before the reset so the reset can't drop them.
    m_rawCb = nullptr;
    m_xferChain = nullptr;
    // A half-arrived listing is still worth showing.
    if (m_capture != Capture::None) { flushCapture(); }
    resetTransientState();

    // Whoever was waiting has to be told, or a cp -r / rm -r queue simply stops
    // halfway with nothing on screen. The sentinel is worded as a storage error
    // because that is already how every continuation tests for failure.
    static const QString kNoReply = QStringLiteral("Storage error: no reply from the device");

    if (what == Xfer::Download || what == Xfer::UploadReady) {
        m_xferChain = chain;   // finishXfer advances the batch if there is one
        finishXfer(false, QStringLiteral("[ no reply from the Flipper -- transfer aborted ]"));
        return;
    }
    if (rawCb) {
        // This continuation may be one step of a batch that still owns the
        // chain, so hand it back before running it -- otherwise the queue is
        // orphaned and stalls silently, which is the failure this whole
        // watchdog exists to prevent.
        m_xferChain = chain;
        rawCb(kNoReply);
        return;
    }
    if (chain) { chain(false); return; }
    cliLogFail(QStringLiteral("%1 -- no reply from the Flipper").arg(m_lastTyped));
    appendOutput(QStringLiteral("[ no reply from the Flipper -- giving up ]\n") + prompt());
}

void FlipperCli::writeLine(const QString &cmd, bool logIt)
{
    if (!m_port) { return; }
    if (logIt) { trace(cmd); }
    m_port->write(cmd.toUtf8());
    m_port->write("\r\n");
}

// ---- clipboard -------------------------------------------------------------
QString FlipperCli::clipboardText() const
{
    QClipboard *cb = QGuiApplication::clipboard();
    return cb ? cb->text() : QString();
}

void FlipperCli::copyToClipboard(const QString &text) const
{
    QClipboard *cb = QGuiApplication::clipboard();
    if (cb) { cb->setText(text); }
}

// ---- Tab completion --------------------------------------------------------
// The first word completes against every command name this CLI answers to;
// anything after it completes against a real directory listing -- the Flipper's
// if the path lives there, this computer's for "~/..." and other host paths.
void FlipperCli::complete(const QString &line)
{
    // Anything mid-flight owns the port; completing now would inject a listing
    // into the middle of a transfer.
    if (!m_port || !m_active || busy()) {
        emit completion(line);
        return;
    }

    int start = line.size();
    while (start > 0 && !line.at(start - 1).isSpace()) { --start; }
    const QString token = line.mid(start);
    const QString head  = line.left(start);

    if (head.trimmed().isEmpty()) {
        QStringList hits;
        for (const QString &n : cliAllCommandNames()) {
            if (n.startsWith(token, Qt::CaseInsensitive)) { hits += n; }
        }
        applyCompletion(head, token, hits, QList<bool>(), line);
        return;
    }

    const int slash = token.lastIndexOf(QLatin1Char('/'));
    const QString dirPart = (slash >= 0) ? token.left(slash + 1) : QString();   // keeps its '/'
    const QString base    = (slash >= 0) ? token.mid(slash + 1) : token;

    // Host side: plain QDir, no round trip.
    if (cliIsHostPath(dirPart.isEmpty() ? token : dirPart)) {
        const QString absDir = cliExpandHostPath(dirPart.isEmpty() ? QStringLiteral("~/") : dirPart);
        QStringList hits;
        QList<bool> isDir;
        const QFileInfoList entries = QDir(absDir).entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot,
                                                                 QDir::Name | QDir::DirsFirst);
        for (const QFileInfo &fi : entries) {
            if (!fi.fileName().startsWith(base, Qt::CaseInsensitive)) { continue; }
            hits  += dirPart + fi.fileName();
            isDir += fi.isDir();
        }
        applyCompletion(head, token, hits, isDir, line);
        return;
    }

    // Tab's own lookup is machinery the user never asked for -- it stays out of
    // the verbose log even when everything else goes in.
    m_quiet = true;
    const QString absDir = cliResolvePath(m_cwd, dirPart.isEmpty() ? QStringLiteral(".") : dirPart);
    sendRaw(QStringLiteral("storage list ") + absDir,
            [this, head, token, dirPart, base, line](const QString &raw) {
        m_quiet = false;
        static const QRegularExpression rowRe(QStringLiteral("^\\[([DF])\\]\\s+(.*?)(?:\\s+(\\d+)b)?$"));
        QStringList hits;
        QList<bool> isDir;
        for (const QString &l : raw.split(QLatin1Char('\n'))) {
            const auto m = rowRe.match(l.trimmed());
            if (!m.hasMatch()) { continue; }
            const QString name = m.captured(2);
            if (!name.startsWith(base, Qt::CaseInsensitive)) { continue; }
            hits  += dirPart + name;
            isDir += (m.captured(1) == QLatin1String("D"));
        }
        applyCompletion(head, token, hits, isDir, line);
    });
}

void FlipperCli::applyCompletion(const QString &head, const QString &token,
                                 const QStringList &hits, const QList<bool> &isDir,
                                 const QString &original)
{
    if (hits.isEmpty()) { emit completion(original); return; }

    if (hits.size() == 1) {
        const bool dir = isDir.value(0, false);
        emit completion(head + hits.first() + (dir ? QStringLiteral("/") : QStringLiteral(" ")));
        return;
    }

    // Several candidates: fill in as far as they all agree, then show the list
    // the way a shell does -- reprinting the line underneath so the caret keeps
    // its place.
    QString common = hits.first();
    for (const QString &h : hits) {
        int i = 0;
        while (i < common.size() && i < h.size() && common.at(i) == h.at(i)) { ++i; }
        common.truncate(i);
    }
    if (common.size() <= token.size()) { common = token; }

    QStringList shown;
    for (int i = 0; i < hits.size(); ++i) {
        const QString name = hits.at(i).section(QLatin1Char('/'), -1);
        shown += isDir.value(i, false) ? (name + QLatin1Char('/')) : name;
    }
    int colW = 0;
    for (const QString &s : shown) { colW = qMax(colW, s.size()); }
    colW += 2;
    const int cols = qMax(1, 76 / qMax(1, colW));
    QString grid;
    for (int i = 0; i < shown.size(); ++i) {
        QString cell = shown.at(i);
        const bool last = ((i % cols) == cols - 1) || (i == shown.size() - 1);
        if (!last) { while (cell.size() < colW) { cell += QLatin1Char(' '); } }
        grid += cell;
        if (last) { grid += QLatin1Char('\n'); }
    }

    appendOutput(original + QLatin1Char('\n') + grid + prompt());
    emit completion(head + common);
}

void FlipperCli::interrupt()
{
    if (!m_port || !m_active) { return; }
    m_port->write("\x03");   // Ctrl-C

    // Drop every half-finished capture. Without this, a Ctrl-C landing in the
    // middle of a help listing or a file transfer leaves the panel buffering
    // forever and nothing reaches the screen again.
    resetTransientState();

    appendOutput(QStringLiteral("^C\n") + prompt());
}

void FlipperCli::onReadyRead()
{
    if (!m_port) { return; }
    const QByteArray chunk = m_port->readAll();
    if (!chunk.isEmpty()) { m_sawDeviceBytes = true; }

    // Bytes arriving mean the device is still talking, so the no-reply
    // watchdog only ever fires on real silence.
    if (m_opGuard && m_opGuard->isActive()) { m_opGuard->start(); }

    // ---- raw transfer phases (must not go through the text sanitiser) -------
    if (!m_cdPending.isEmpty()) {
        m_cdRaw += chunk;
        const QString s = QString::fromUtf8(m_cdRaw);
        if (!s.contains(QLatin1String(">:")) && m_cdRaw.size() < 4096) { return; }
        // "Directory" for a folder; a storage root answers with the volume line
        // instead, and that is just as valid a place to stand.
        if (s.contains(QLatin1String("Directory"))
            || s.contains(QLatin1String("Storage, label"))
            || s.contains(QLatin1String("Total space"))) {
            m_cdPrev = m_cwd;
            m_cwd = m_cdPending;
            emit promptChanged();
            setStatus(QStringLiteral("CLI live -- %1").arg(m_cwd));
            appendOutput(prompt());
        } else if (s.contains(QLatin1String("File"))) {
            appendOutput(QStringLiteral("[ not a folder: %1 ]\n").arg(m_cdPending) + prompt());
        } else {
            appendOutput(QStringLiteral("[ no such folder: %1 ]\n").arg(m_cdPending) + prompt());
        }
        m_cdPending.clear();
        m_cdRaw.clear();
        disarmGuard();
        return;
    }

    // Generic one-shot command (sendRaw): buffer until the prompt marker, then
    // hand the raw text to whoever asked for it. Everything that isn't a
    // payload transfer -- md5 checks, tree/list scans, mkdir, remove -- goes
    // through here.
    if (m_xfer == Xfer::Raw) {
        m_xferRaw += chunk;
        const QString s = QString::fromUtf8(m_xferRaw);
        if (!s.contains(QLatin1String(">:")) && m_xferRaw.size() < 300000) { return; }
        m_xfer = Xfer::None;
        m_xferRaw.clear();
        disarmGuard();
        auto cb = m_rawCb;
        m_rawCb = nullptr;
        if (cb) { cb(s); }
        return;
    }

    if (m_xfer == Xfer::UploadReady) {
        const CliXferStep st = cliUploadFeed(m_xferRaw, chunk, m_xferPayload, m_xferLabel);
        if (!st.done) { return; }
        disarmGuard();
        if (st.failed) {
            m_xfer = Xfer::None;
            m_xferPayload.clear();
            finishXfer(false, st.message);
            return;
        }
        if (!st.toWrite.isEmpty()) { m_port->write(st.toWrite); }
        m_xfer = Xfer::None;
        // The payload is now in flight to the firmware. write_chunk gives no
        // per-byte ack, so "storage md5" against a local hash is the only real
        // proof the bytes weren't clipped or corrupted on the wire.
        const QByteArray payload = m_xferPayload;
        m_xferPayload.clear();
        const QString devPath = m_xferDevPath;
        const QString baseMsg = st.message;
        QTimer::singleShot(150, this, [this, payload, devPath, baseMsg]() {
            sendRaw(QStringLiteral("storage md5 ") + devPath, [this, payload, baseMsg](const QString &raw) {
                const QString got = cliExtractMd5(raw);
                const QString want = QString::fromLatin1(QCryptographicHash::hash(payload, QCryptographicHash::Md5).toHex());
                const bool ok = !got.isEmpty() && got == want;
                finishXfer(ok, ok ? baseMsg + QStringLiteral(" [md5 ok]")
                                   : baseMsg + QStringLiteral(" [md5 MISMATCH -- transfer is suspect]"));
            });
        });
        return;
    }

    if (m_xfer == Xfer::Download) {
        const CliXferStep st = cliDownloadFeed(m_xferRaw, m_xferSize, chunk, m_xferHostDst);
        if (!st.done) { return; }
        disarmGuard();
        m_xfer = Xfer::None;
        if (st.failed) { finishXfer(false, st.message); return; }

        const QByteArray body = st.body;
        const QString devPath = m_xferDevPath;
        const QString hostDst = m_xferHostDst;
        sendRaw(QStringLiteral("storage md5 ") + devPath, [this, body, hostDst](const QString &raw) {
            const QString got = cliExtractMd5(raw);
            const QString want = QString::fromLatin1(QCryptographicHash::hash(body, QCryptographicHash::Md5).toHex());
            if (got.isEmpty() || got != want) {
                finishXfer(false, QStringLiteral("[ %1 -- md5 MISMATCH, file NOT saved ]").arg(hostDst));
                return;
            }
            QFile out(hostDst);
            if (!out.open(QIODevice::WriteOnly)) {
                finishXfer(false, QStringLiteral("[ can't write %1: %2 ]").arg(hostDst, out.errorString()));
                return;
            }
            out.write(body);
            out.close();
            finishXfer(true, QStringLiteral("[ saved %1 bytes -> %2 [md5 ok] ]").arg(body.size()).arg(hostDst));
        });
        return;
    }

    QString text = QString::fromUtf8(chunk);

    // An escape sequence can be split across two serial reads. When that
    // happened the regex below saw only "\x1B[0" (no final byte) and left it
    // alone, the control-character sweep then ate the lone ESC, and the "[0m"
    // remainder was printed as text -- the stray "[0m" after the help listing.
    // Hold an unterminated tail back and glue it onto the next chunk instead.
    static const QRegularExpression ansiWhole(QStringLiteral("\x1B\\[[0-9;?]*[A-Za-z]"));
    if (!m_escTail.isEmpty()) { text.prepend(m_escTail); m_escTail.clear(); }
    {
        const int e = text.lastIndexOf(QChar(0x1B));
        if (e >= 0) {
            const QString tail = text.mid(e);
            // Bounded: a sequence this long is not one, and holding bytes back
            // forever would stall the view.
            if (tail.size() < 16 && !ansiWhole.match(tail).hasMatch()) {
                m_escTail = tail;
                text.chop(tail.size());
            }
        }
    }

    // Strip ANSI escape sequences (colours, cursor moves) for a clean text view.
    static const QRegularExpression ansi(QStringLiteral("\x1B\\[[0-9;?]*[A-Za-z]"));
    text.remove(ansi);
    text.remove(QLatin1Char('\r'));
    // Strip stray C0/C1 control characters -- this is the little square the
    // firmware emits next to the prompt.
    static const QRegularExpression ctrl(QStringLiteral("[\\x00-\\x08\\x0B\\x0C\\x0E-\\x1F\\x7F]"));
    text.remove(ctrl);
    // Eat the echo of a translated command, character by character so a chunk
    // boundary in the middle of it doesn't leak the translation onto the screen.
    if (!m_echoPending.isEmpty()) {
        int i = 0;
        while (i < text.size() && !m_echoPending.isEmpty()) {
            const QChar c = text.at(i);
            if (c == m_echoPending.at(0)) { m_echoPending.remove(0, 1); ++i; }
            else if (c == QLatin1Char('\n') && m_echoPending.at(0) == QLatin1Char('\n')) { ++i; }
            else { m_echoPending.clear(); break; }   // not our echo -- leave it alone
        }
        text = text.mid(i);
        if (text.isEmpty()) { return; }
    }

    // Swap the firmware's bare ">: " for our shell-style prompt.
    text.replace(QLatin1String(">: "), prompt());
    if (text.endsWith(QLatin1String(">:"))) { text.chop(2); text += prompt(); }

    // The firmware streams its help listing across several serial chunks, so it
    // can't be reformatted chunk by chunk -- that's what left our shortcuts on a
    // different grid. Hold the whole listing back until the prompt returns, then
    // lay the entire thing out at once.
    // Both spellings: stock firmware says "Commands available:", Unleashed and
    // Momentum say "Available commands:". Matching only the first one meant the
    // whole help reformatter never ran on a fork -- which is why the listing
    // came through raw, with none of our own commands in it.
    static const QRegularExpression helpHdr(
        QStringLiteral("(?:commands available|available commands)\\s*:"),
        QRegularExpression::CaseInsensitiveOption);
    if (m_capture == Capture::None && helpHdr.match(text).hasMatch()) {
        m_capture = Capture::Help;
    }
    if (m_capture != Capture::None) {
        m_captureBuf += text;
        const QString p = prompt();
        const bool done = m_captureBuf.contains(p)
                          || m_captureBuf.contains(QLatin1String("Storage error"))
                          || m_captureBuf.size() > 16384;
        if (done) {
            flushCapture();
        } else {
            if (!m_captureFlush) {
                m_captureFlush = new QTimer(this);
                m_captureFlush->setSingleShot(true);
                connect(m_captureFlush, &QTimer::timeout, this, &FlipperCli::flushCapture);
            }
            m_captureFlush->start(500);   // safety net if the prompt never lands
        }
        return;
    }

    // "Storage error: file/dir not exist" and friends come back as plain text
    // on the serial line, so this is the only place they can be caught. The
    // guard stops a reply split across chunks from logging twice.
    if (text.contains(QLatin1String("Storage error"))) {
        const QString err = text.section(QLatin1String("Storage error"), 1, 1)
                                .section(QLatin1Char('\n'), 0, 0).trimmed();
        const QString line = QStringLiteral("%1 -- Storage error%2").arg(m_lastTyped, err);
        if (line != m_lastLoggedError) {
            m_lastLoggedError = line;
            cliLogFail(line);
        }
    }

    appendOutput(text);
}

QString FlipperCli::prompt() const
{
    return cliPromptFor(m_devName, m_cwd);
}

void FlipperCli::flushCapture()
{
    if (m_capture == Capture::None) { return; }
    if (m_captureFlush) { m_captureFlush->stop(); }
    disarmGuard();
    const Capture what = m_capture;
    m_capture = Capture::None;
    if (m_captureBuf.isEmpty()) { return; }

    const QString buf = m_captureBuf;
    m_captureBuf.clear();
    appendOutput(what == Capture::Help ? cliFormatHelp(buf, prompt().trimmed())
                                      : cliFormatListing(buf));
}

// ---- host <-> Flipper file transfer over the plain CLI ----------------------
void FlipperCli::uploadToFlipper(const QString &hostPath, const QString &devPath, bool exactDest)
{
    // Every bail-out below goes through finishXfer rather than printing and
    // returning. A cp -r sets m_xferChain before calling in, so a plain return
    // left the queue waiting on a continuation that never came: the remaining
    // files silently never copied and no summary was ever printed.
    if (!m_port || !m_active) {
        finishXfer(false, QStringLiteral("[ the CLI link is not open ]"));
        return;
    }

    const QFileInfo fi(hostPath);
    if (!fi.exists() || !fi.isFile()) {
        finishXfer(false, QStringLiteral("[ no such file on this computer: %1 ]").arg(hostPath));
        return;
    }
    QFile f(hostPath);
    if (!f.open(QIODevice::ReadOnly)) {
        finishXfer(false, QStringLiteral("[ can't read %1: %2 ]").arg(hostPath, f.errorString()));
        return;
    }
    const QByteArray data = f.readAll();
    f.close();

    // exactDest is set by the commands that build their payload in a temp file
    // (echo, touch, wget): there the host filename is a throwaway, and a
    // destination like /ext/notes has no dot in its last segment, so the
    // usual "looks like a folder" guess would bury the temp name inside it.
    const QString dst = exactDest ? devPath
                                  : cliJoinDest(devPath, fi.fileName(), cliLooksLikeDir(devPath));

    // The firmware reads write_chunk's payload in a fixed 1 KB loop rather than
    // malloc'ing the whole thing, so this cap is just a sane one-shot size for
    // the CLI link, not a memory ceiling. Anything past it still belongs to the
    // RPC file tools, which report progress.
    static const int kMaxChunk = 1 * 1024 * 1024;   // 1 MiB
    if (data.size() > kMaxChunk) {
        finishXfer(false, QStringLiteral("[ %1 is %2 bytes -- CLI uploads cap at %3. Close the CLI and use the file tools. ]")
                          .arg(fi.fileName()).arg(data.size()).arg(kMaxChunk));
        return;
    }

    m_xferDevPath = dst;
    appendOutput(QStringLiteral("[ uploading %1 (%2 bytes) -> %3 ]\n").arg(fi.fileName()).arg(data.size()).arg(dst));
    // storage write_chunk APPENDS to an existing file rather than replacing it
    // (FSOM_OPEN_APPEND), so a repeat "cp" would otherwise leave the file with
    // old and new content concatenated. Clear any previous copy first -- "not
    // found" here is expected and harmless.
    sendRaw(QStringLiteral("storage remove ") + dst, [this, data, dst](const QString &) {
        if (!m_port || !m_active) {
            finishXfer(false, QStringLiteral("[ the CLI link dropped mid-upload ]"));
            return;
        }
        m_xfer = Xfer::UploadReady;
        m_xferRaw.clear();
        m_xferPayload = data;
        m_xferLabel = dst;
        // CR only. With "\r\n" the firmware takes the CR as Enter and then reads
        // the stray LF as the chunk's first byte -- that's the blank line that
        // showed up at the top of uploaded files.
        const QString wc = QStringLiteral("storage write_chunk %1 %2").arg(dst).arg(data.size());
        trace(wc);
        m_port->write(wc.toUtf8());
        m_port->write("\r");
        armGuard();
    });
}

void FlipperCli::downloadFromFlipper(const QString &devPath, const QString &hostPath)
{
    if (!m_port || !m_active) { return; }

    const QString name = devPath.section(QLatin1Char('/'), -1);
    QString dst = hostPath;
    if (dst.endsWith(QLatin1Char('/')) || QFileInfo(dst).isDir()) {
        dst = cliJoinDest(dst, name, true);
    }
    QDir().mkpath(QFileInfo(dst).absolutePath());

    m_xfer = Xfer::Download;
    m_xferRaw.clear();
    m_xferSize = -1;
    m_xferHostDst = dst;
    m_xferLabel = dst;
    m_xferDevPath = devPath;
    appendOutput(QStringLiteral("[ downloading %1 -> %2 ]\n").arg(devPath, dst));
    writeLine(QStringLiteral("storage read %1").arg(devPath));
    armGuard();
}

// Prints the result of one transfer -- unless a batch op is chained behind it,
// in which case the batch driver decides when the prompt finally comes back.
void FlipperCli::finishXfer(bool ok, const QString &message)
{
    if (!message.isEmpty()) {
        if (ok) { cliLog(message); } else { cliLogFail(message); }
    }
    if (m_xferChain) {
        auto cb = m_xferChain;
        m_xferChain = nullptr;
        appendOutput(message + QLatin1Char('\n'));
        cb(ok);
        return;
    }
    appendOutput(message + QLatin1Char('\n') + prompt());
}

// Generic one-shot command on the interactive port -- see the Xfer::Raw branch
// in onReadyRead. Refuses if a transfer or another raw command is already in
// flight rather than silently clobbering it.
void FlipperCli::sendRaw(const QString &cmd, std::function<void(const QString &)> onDone)
{
    if (!m_port || !m_active || busy()) {
        // Not an empty string: continuations test for failure with
        // contains("error"), so "" used to be read as a clean success.
        if (onDone) { onDone(QStringLiteral("Storage error: the CLI is busy")); }
        return;
    }
    m_xfer = Xfer::Raw;
    m_xferRaw.clear();
    m_rawCb = [this, onDone](const QString &raw) {
        traceReply(raw);
        if (onDone) { onDone(raw); }
    };
    writeLine(cmd);
    armGuard();
}

// mkdir -p: the firmware's own mkdir isn't recursive, so walk the path one
// segment at a time. "already exists" errors are expected and ignored.
void FlipperCli::ensureDeviceDir(const QString &path, std::function<void()> done)
{
    if (path.isEmpty() || path == QLatin1String("/ext") || path == QLatin1String("/int")) {
        if (done) { done(); }
        return;
    }
    auto queue = std::make_shared<QStringList>();
    QString acc;
    const QStringList segs = path.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    for (const QString &s : segs) {
        acc += QLatin1Char('/') + s;
        *queue += acc;
    }
    auto step = std::make_shared<std::function<void()>>();
    *step = [this, queue, done, step]() {
        if (queue->isEmpty()) { if (done) { done(); } return; }
        const QString dir = queue->takeFirst();
        sendRaw(QStringLiteral("storage mkdir ") + dir, [step](const QString &) { (*step)(); });
    };
    (*step)();
}

// cp -r, computer -> Flipper: walk the local tree with QDirIterator (cheap,
// synchronous, local disk), mirror the folder structure on the SD card, and
// upload each file through the same verified single-file path used everywhere
// else -- chained one at a time via m_xferChain so the port is never shared.
void FlipperCli::startCopyUpTree(const QString &hostRoot, const QString &devRoot)
{
    const QFileInfo fi(hostRoot);
    if (!fi.exists()) {
        appendOutput(QStringLiteral("[ no such file or folder on this computer: %1 ]\n").arg(hostRoot) + prompt());
        return;
    }
    QStringList files;
    if (fi.isDir()) {
        QDirIterator it(hostRoot, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) { files += it.next(); }
    } else {
        files += fi.absoluteFilePath();
    }
    if (files.isEmpty()) {
        appendOutput(QStringLiteral("[ %1 has no files to copy ]\n").arg(hostRoot) + prompt());
        return;
    }
    std::sort(files.begin(), files.end());
    const QString base = fi.isDir() ? QDir(hostRoot).absolutePath() : QFileInfo(hostRoot).absolutePath();
    appendOutput(QStringLiteral("[ copying %1 file(s) -> %2 ]\n").arg(files.size()).arg(devRoot));

    auto queue = std::make_shared<QStringList>(files);
    auto ok = std::make_shared<int>(0);
    auto fail = std::make_shared<int>(0);
    auto step = std::make_shared<std::function<void()>>();
    *step = [this, queue, ok, fail, base, devRoot, step]() {
        if (queue->isEmpty()) {
            appendOutput(QStringLiteral("[ done -- %1 ok, %2 failed ]\n").arg(*ok).arg(*fail) + prompt());
            return;
        }
        const QString hostFile = queue->takeFirst();
        const QString rel = QDir(base).relativeFilePath(hostFile);
        const int slash = rel.lastIndexOf(QLatin1Char('/'));
        const QString devDir = (slash >= 0) ? (devRoot + QLatin1Char('/') + rel.left(slash)) : devRoot;
        m_xferChain = [ok, fail, step](bool success) {
            if (success) { ++(*ok); } else { ++(*fail); }
            (*step)();
        };
        ensureDeviceDir(devDir, [this, hostFile, devDir]() {
            uploadToFlipper(hostFile, devDir + QLatin1Char('/'));
        });
    };
    (*step)();
}

// cp -r, Flipper -> computer: "storage tree" already does the recursion, so
// just enumerate, mkpath the mirror on this side, and download each file.
void FlipperCli::startCopyDownTree(const QString &devRoot, const QString &hostRoot)
{
    appendOutput(QStringLiteral("[ scanning %1... ]\n").arg(devRoot));
    sendRaw(QStringLiteral("storage tree ") + devRoot, [this, devRoot, hostRoot](const QString &raw) {
        if (raw.contains(QLatin1String("Storage error"))) {
            appendOutput(QStringLiteral("[ can't read %1 on the Flipper ]\n").arg(devRoot) + prompt());
            return;
        }
        QStringList files;
        for (const auto &e : cliParseTree(raw)) { if (!e.isDir) { files += e.path; } }
        if (files.isEmpty()) {
            appendOutput(QStringLiteral("[ %1 has no files to copy ]\n").arg(devRoot) + prompt());
            return;
        }
        QDir().mkpath(hostRoot);
        appendOutput(QStringLiteral("[ copying %1 file(s) -> %2 ]\n").arg(files.size()).arg(hostRoot));

        auto queue = std::make_shared<QStringList>(files);
        auto ok = std::make_shared<int>(0);
        auto fail = std::make_shared<int>(0);
        auto step = std::make_shared<std::function<void()>>();
        *step = [this, queue, ok, fail, devRoot, hostRoot, step]() {
            if (queue->isEmpty()) {
                appendOutput(QStringLiteral("[ done -- %1 ok, %2 failed ]\n").arg(*ok).arg(*fail) + prompt());
                return;
            }
            const QString devFile = queue->takeFirst();
            QString rel = devFile.startsWith(devRoot) ? devFile.mid(devRoot.size()) : devFile.section(QLatin1Char('/'), -1);
            while (rel.startsWith(QLatin1Char('/'))) { rel.remove(0, 1); }
            const QString hostFile = hostRoot + QLatin1Char('/') + rel;
            QDir().mkpath(QFileInfo(hostFile).absolutePath());
            m_xferChain = [ok, fail, step](bool success) {
                if (success) { ++(*ok); } else { ++(*fail); }
                (*step)();
            };
            downloadFromFlipper(devFile, hostFile);
        };
        (*step)();
    });
}

// Deletes "path" and, if it's a folder, everything under it. "storage tree"
// tells us what's there; a plain "storage remove" on the same path is tried
// first because it also covers the common cases (a file, or an already-empty
// folder) without a scan. Children are removed before parents: files first,
// then directories in reverse listing order (tree lists a folder before its
// contents, so reversing guarantees every child is gone before its parent).
void FlipperCli::removeTreeCore(const QString &path, std::function<void(bool)> done)
{
    sendRaw(QStringLiteral("storage tree ") + path, [this, path, done](const QString &raw) {
        if (raw.contains(QLatin1String("Storage error"))) {
            sendRaw(QStringLiteral("storage remove ") + path, [done](const QString &raw2) {
                if (done) { done(!raw2.contains(QLatin1String("error"), Qt::CaseInsensitive)); }
            });
            return;
        }
        const auto entries = cliParseTree(raw);
        QStringList targets;
        for (const auto &e : entries) { if (!e.isDir) { targets += e.path; } }
        QStringList dirs;
        for (const auto &e : entries) { if (e.isDir) { dirs += e.path; } }
        std::reverse(dirs.begin(), dirs.end());
        targets += dirs;
        targets += path;   // the root itself, last

        auto queue = std::make_shared<QStringList>(targets);
        auto allOk = std::make_shared<bool>(true);
        auto step = std::make_shared<std::function<void()>>();
        *step = [this, queue, allOk, done, step]() {
            if (queue->isEmpty()) { if (done) { done(*allOk); } return; }
            const QString t = queue->takeFirst();
            sendRaw(QStringLiteral("storage remove ") + t, [allOk, step](const QString &raw3) {
                if (raw3.contains(QLatin1String("error"), Qt::CaseInsensitive)) { *allOk = false; }
                (*step)();
            });
        };
        (*step)();
    });
}

void FlipperCli::startRemoveTree(const QString &path)
{
    appendOutput(QStringLiteral("[ removing %1... ]\n").arg(path));
    removeTreeCore(path, [this, path](bool ok) {
        appendOutput((ok ? QStringLiteral("[ removed %1 ]\n").arg(path)
                          : QStringLiteral("[ some items under %1 could not be removed ]\n").arg(path))
                     + prompt());
    });
}

// Same as startRemoveTree but for a set of matches from a wildcard, with one
// summary at the end instead of one prompt per file.
void FlipperCli::runRemoveQueue(const QStringList &targets)
{
    auto queue = std::make_shared<QStringList>(targets);
    auto ok = std::make_shared<int>(0);
    auto fail = std::make_shared<int>(0);
    auto step = std::make_shared<std::function<void()>>();
    *step = [this, queue, ok, fail, step]() {
        if (queue->isEmpty()) {
            appendOutput(QStringLiteral("[ done -- %1 removed, %2 failed ]\n").arg(*ok).arg(*fail) + prompt());
            return;
        }
        const QString t = queue->takeFirst();
        removeTreeCore(t, [ok, fail, step](bool success) {
            if (success) { ++(*ok); } else { ++(*fail); }
            (*step)();
        });
    };
    (*step)();
}

// Expands "*.sub" style patterns against one directory's listing (not
// recursive -- that's what "find" is for). Matches on the bare filename.
void FlipperCli::expandDeviceGlob(const QString &pattern, std::function<void(const QStringList &)> done)
{
    const int slash = pattern.lastIndexOf(QLatin1Char('/'));
    const QString dir  = (slash > 0) ? pattern.left(slash) : QStringLiteral("/ext");
    const QString glob = (slash >= 0) ? pattern.mid(slash + 1) : pattern;
    static const QRegularExpression rowRe(QStringLiteral("^\\[([DF])\\]\\s+(.*?)(?:\\s+(\\d+)b)?$"));
    const QRegularExpression rx(QRegularExpression::wildcardToRegularExpression(glob),
                                QRegularExpression::CaseInsensitiveOption);
    sendRaw(QStringLiteral("storage list ") + dir, [dir, rx, done](const QString &raw) {
        QStringList out;
        for (const QString &line : raw.split(QLatin1Char('\n'))) {
            const auto m = rowRe.match(line.trimmed());
            if (!m.hasMatch()) { continue; }
            const QString name = m.captured(2);
            if (rx.match(name).hasMatch()) { out += dir + QLatin1Char('/') + name; }
        }
        if (done) { done(out); }
    });
}

// Copies a set of wildcard matches (device paths) to one destination -- either
// this computer (dstHost) or another spot on the Flipper.
void FlipperCli::runCopyQueue(const QStringList &devMatches, const QString &dst, bool dstHost)
{
    auto queue = std::make_shared<QStringList>(devMatches);
    auto ok = std::make_shared<int>(0);
    auto fail = std::make_shared<int>(0);
    auto step = std::make_shared<std::function<void()>>();
    *step = [this, queue, ok, fail, dst, dstHost, step]() {
        if (queue->isEmpty()) {
            appendOutput(QStringLiteral("[ done -- %1 ok, %2 failed ]\n").arg(*ok).arg(*fail) + prompt());
            return;
        }
        const QString devFile = queue->takeFirst();
        auto next = [ok, fail, step](bool success) {
            if (success) { ++(*ok); } else { ++(*fail); }
            (*step)();
        };
        if (dstHost) {
            m_xferChain = next;
            downloadFromFlipper(devFile, dst + QLatin1Char('/'));
        } else {
            const QString name = devFile.section(QLatin1Char('/'), -1);
            sendRaw(QStringLiteral("storage copy ") + devFile + QLatin1Char(' ') + dst + QLatin1Char('/') + name,
                    [next](const QString &raw) { next(!raw.contains(QLatin1String("error"), Qt::CaseInsensitive)); });
        }
    };
    (*step)();
}

// find: "storage tree" the root, filter by wildcard against basename (falling
// back to a match against the full path) and print every hit.
void FlipperCli::startFind(const QString &root, const QString &pattern)
{
    const QRegularExpression rx(QRegularExpression::wildcardToRegularExpression(pattern),
                                QRegularExpression::CaseInsensitiveOption);
    sendRaw(QStringLiteral("storage tree ") + root, [this, rx](const QString &raw) {
        if (raw.contains(QLatin1String("Storage error"))) {
            appendOutput(QStringLiteral("[ can't read that path ]\n") + prompt());
            return;
        }
        QStringList hits;
        for (const auto &e : cliParseTree(raw)) {
            const QString name = e.path.section(QLatin1Char('/'), -1);
            if (rx.match(name).hasMatch() || rx.match(e.path).hasMatch()) { hits += e.path; }
        }
        appendOutput((hits.isEmpty() ? QStringLiteral("[ no matches ]\n") : hits.join(QLatin1Char('\n')) + QLatin1Char('\n'))
                     + prompt());
    });
}

// edit: read the file's text back like "cat" does, but hand it to
// editRequested() instead of just printing it, so a host-side editor panel can
// pop it open. Also echoes the content in the terminal so the command is still
// useful with nothing connected to that signal.
void FlipperCli::startEdit(const QString &path)
{
    sendRaw(QStringLiteral("storage read ") + path, [this, path](const QString &raw) {
        if (raw.contains(QLatin1String("Storage error"))) {
            appendOutput(QStringLiteral("[ no such file: %1 ]\n").arg(path) + prompt());
            return;
        }
        QString body = raw;
        const int c = raw.indexOf(QLatin1String("Size:"));
        if (c >= 0) {
            const int nl = raw.indexOf(QLatin1Char('\n'), c);
            body = (nl >= 0) ? raw.mid(nl + 1) : QString();
        }
        const int p = body.lastIndexOf(QLatin1String(">:"));
        if (p >= 0) { body = body.left(p); }
        body = body.trimmed();
        // Hand the file to the editor panel in the UI. We deliberately DON'T
        // also dump the body into the terminal any more -- that made edit look
        // like a glorified cat and buried the point. A short line confirms what
        // opened; the panel shows and edits the actual contents.
        emit editRequested(path, body);
        appendOutput(QStringLiteral("[ editing %1 -- opening editor ]\n").arg(path) + prompt());
    });
}

// ---- host-side text utilities over "storage read" --------------------------
//
// The Flipper cannot run grep, head, tail or wc itself, and never will: the
// firmware lives on an STM32WB55 with 256 KB of RAM and a fixed command table.
// So the file comes over the wire once and this computer does the work -- which
// is the same trade the rest of this panel already makes for find and cp -r.
void FlipperCli::readDeviceText(const QString &path, std::function<void(bool, const QString &)> done)
{
    sendRaw(QStringLiteral("storage read ") + path, [path, done](const QString &raw) {
        if (raw.contains(QLatin1String("Storage error"))) { done(false, QString()); return; }
        // "storage read" prints a "Size: N" header before the body and the
        // prompt after it; neither belongs to the file.
        QString body = raw;
        const int c = raw.indexOf(QLatin1String("Size:"));
        if (c >= 0) {
            const int nl = raw.indexOf(QLatin1Char('\n'), c);
            body = (nl >= 0) ? raw.mid(nl + 1) : QString();
        }
        const int p = body.lastIndexOf(QLatin1String(">:"));
        if (p >= 0) { body = body.left(p); }
        done(true, body);
    });
}

void FlipperCli::startGrep(const QString &pattern, const QString &path)
{
    readDeviceText(path, [this, pattern, path](bool ok, const QString &body) {
        if (!ok) { appendOutput(QStringLiteral("[ no such file: %1 ]\n").arg(path) + prompt()); return; }
        // Plain substring, case-insensitive -- the same thing `grep -i` does for
        // the overwhelming majority of what gets typed at a Flipper. A regex
        // pattern would need escaping rules the rest of this shell doesn't have.
        QStringList hits;
        const QStringList lines = body.split(QLatin1Char('\n'));
        for (int i = 0; i < lines.size(); ++i) {
            if (lines.at(i).contains(pattern, Qt::CaseInsensitive)) {
                hits += QStringLiteral("%1: %2").arg(i + 1, 4).arg(lines.at(i).trimmed());
            }
        }
        appendOutput((hits.isEmpty() ? QStringLiteral("[ no matches ]\n")
                                     : hits.join(QLatin1Char('\n')) + QLatin1Char('\n'))
                     + prompt());
    });
}

void FlipperCli::startHeadTail(const QString &path, int n, bool head)
{
    readDeviceText(path, [this, path, n, head](bool ok, const QString &body) {
        if (!ok) { appendOutput(QStringLiteral("[ no such file: %1 ]\n").arg(path) + prompt()); return; }
        QStringList lines = body.split(QLatin1Char('\n'));
        while (!lines.isEmpty() && lines.last().trimmed().isEmpty()) { lines.removeLast(); }
        const int take = qBound(1, n, lines.size());
        const QStringList slice = head ? lines.mid(0, take) : lines.mid(lines.size() - take);
        appendOutput(slice.join(QLatin1Char('\n')) + QLatin1Char('\n') + prompt());
    });
}

void FlipperCli::startWc(const QString &path)
{
    readDeviceText(path, [this, path](bool ok, const QString &body) {
        if (!ok) { appendOutput(QStringLiteral("[ no such file: %1 ]\n").arg(path) + prompt()); return; }
        const QStringList lines = body.split(QLatin1Char('\n'));
        int words = 0;
        for (const QString &l : lines) {
            words += l.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts).size();
        }
        appendOutput(QStringLiteral("%1 lines  %2 words  %3 bytes  %4\n")
                         .arg(lines.size()).arg(words).arg(body.toUtf8().size()).arg(path)
                     + prompt());
    });
}

// du: one "storage tree" walk, summed here. Prints the total plus a line per
// immediate child, which is the question actually being asked when an SD card
// is filling up.
void FlipperCli::startDu(const QString &path)
{
    sendRaw(QStringLiteral("storage tree ") + path, [this, path](const QString &raw) {
        if (raw.contains(QLatin1String("Storage error"))) {
            appendOutput(QStringLiteral("[ no such folder: %1 ]\n").arg(path) + prompt());
            return;
        }
        QString root = path;
        while (root.size() > 1 && root.endsWith(QLatin1Char('/'))) { root.chop(1); }

        qint64 total = 0;
        int files = 0;
        QMap<QString, qint64> perChild;
        for (const auto &e : cliParseTree(raw)) {
            if (e.isDir || e.size < 0) { continue; }
            total += e.size;
            ++files;
            QString rel = e.path;
            if (rel.startsWith(root)) { rel = rel.mid(root.size()); }
            while (rel.startsWith(QLatin1Char('/'))) { rel.remove(0, 1); }
            const int slash = rel.indexOf(QLatin1Char('/'));
            perChild[slash >= 0 ? rel.left(slash) : QStringLiteral(".")] += e.size;
        }

        auto human = [](qint64 b) {
            if (b >= 1024 * 1024) { return QStringLiteral("%1 MB").arg(b / 1048576.0, 0, 'f', 1); }
            if (b >= 1024)        { return QStringLiteral("%1 KB").arg(b / 1024.0, 0, 'f', 1); }
            return QStringLiteral("%1 B").arg(b);
        };

        QStringList out;
        for (auto it = perChild.constBegin(); it != perChild.constEnd(); ++it) {
            out += QStringLiteral("%1\t%2").arg(human(it.value()), it.key());
        }
        out += QStringLiteral("%1\t%2  (%3 files)").arg(human(total), root).arg(files);
        appendOutput(out.join(QLatin1Char('\n')) + QLatin1Char('\n') + prompt());
    });
}

// echo > / >> and touch. Both stage the bytes in a temp file and hand them to
// the normal upload path, so they get its remove-then-write_chunk sequence and
// its MD5 verification for free rather than reimplementing either.
// Save from the editor panel: write the edited text straight back to the same
// path, verified through the normal upload path, and signal the panel so it can
// show "saved" and close. Kept separate from writeTextToDevice so the editor
// isn't coupled to the terminal's prompt printing.
void FlipperCli::saveEditedFile(const QString &path, const QString &content)
{
    if (path.isEmpty()) { emit editSaveError(path, QStringLiteral("No path.")); return; }

    QTemporaryFile tmp(QDir::tempPath() + QStringLiteral("/lotei-edit-XXXXXX"));
    if (!tmp.open()) {
        emit editSaveError(path, QStringLiteral("Couldn't create a temp file on this computer."));
        return;
    }
    tmp.write(content.toUtf8());
    tmp.flush();

    // uploadToFlipper reads the temp file synchronously and reports into the
    // terminal; we additionally emit editSaved so the panel can react. A tiny
    // deferred emit keeps ordering predictable relative to the upload's own
    // terminal output.
    uploadToFlipper(tmp.fileName(), path, true);
    emit editSaved(path);
}

void FlipperCli::writeTextToDevice(const QString &text, const QString &devPath, bool append)
{
    if (append) {
        // ">>" has to read first: storage write_chunk appends, but the upload
        // deliberately removes the file beforehand so a repeated cp can't
        // concatenate. Merging here keeps that rule in one place.
        readDeviceText(devPath, [this, text, devPath](bool ok, const QString &body) {
            QString merged = ok ? body : QString();
            if (!merged.isEmpty() && !merged.endsWith(QLatin1Char('\n'))) { merged += QLatin1Char('\n'); }
            merged += text;
            writeTextToDevice(merged, devPath, false);
        });
        return;
    }

    QTemporaryFile tmp(QDir::tempPath() + QStringLiteral("/lotei-write-XXXXXX"));
    if (!tmp.open()) {
        appendOutput(QStringLiteral("[ can't create a temp file on this computer ]\n") + prompt());
        return;
    }
    tmp.write(text.toUtf8());
    tmp.flush();
    const QString hostPath = tmp.fileName();
    // uploadToFlipper reads the file synchronously, so the temp file going out
    // of scope here is safe.
    uploadToFlipper(hostPath, devPath, true);
}

// wget: this computer does the downloading -- the Flipper has no network stack
// at all -- and the result lands on the SD card. That is the honest version of
// "install something from the internet onto the Flipper": .fap apps, IR and
// Sub-GHz databases, wordlists, anything that is just a file.
// sed s/PATTERN/REPLACEMENT/[g] -- the only form worth having on a Flipper.
// Reads the file over the wire, edits on this computer, writes it back. Without
// a trailing target it prints the transformed text instead of saving (like sed
// to stdout); with one it overwrites (like sed -i), because on a device round-
// tripping a file just to NOT save it is rarely what anyone means.
void FlipperCli::startSed(const QString &expr, const QString &path)
{
    // Parse s/a/b/ or s/a/b/g. Any delimiter after the "s".
    if (expr.size() < 4 || !expr.startsWith(QLatin1Char('s'))) {
        appendOutput(QStringLiteral("[ only s/pattern/replacement/[g] is supported ]\n") + prompt());
        return;
    }
    const QChar delim = expr.at(1);
    const QStringList parts = expr.mid(2).split(delim);
    if (parts.size() < 2) {
        appendOutput(QStringLiteral("[ malformed sed expression ]\n") + prompt());
        return;
    }
    const QString pat = parts.value(0);
    const QString rep = parts.value(1);
    const bool global = parts.value(2).contains(QLatin1Char('g'));
    if (pat.isEmpty()) { appendOutput(QStringLiteral("[ empty pattern ]\n") + prompt()); return; }

    readDeviceText(path, [this, path, pat, rep, global](bool ok, const QString &body) {
        if (!ok) { appendOutput(QStringLiteral("[ no such file: %1 ]\n").arg(path) + prompt()); return; }
        QString out = body;
        const QRegularExpression re(pat);
        if (!re.isValid()) { appendOutput(QStringLiteral("[ bad pattern: %1 ]\n").arg(re.errorString()) + prompt()); return; }
        int count = 0;
        if (global) {
            const QString before = out;
            out.replace(re, rep);
            count = before.count(re);
        } else {
            const QRegularExpressionMatch m = re.match(out);
            if (m.hasMatch()) { out.replace(m.capturedStart(), m.capturedLength(), rep); count = 1; }
        }
        // Overwrite the file with the edited text; report how many it changed.
        writeTextToDevice(out, path, false);
        appendOutput(QStringLiteral("[ sed: %1 replacement%2 in %3 ]\n")
                         .arg(count).arg(count == 1 ? QString() : QStringLiteral("s"), path));
        // writeTextToDevice prints its own prompt on completion.
    });
}

// diff A B -- both files fetched, compared here. A plain line-level diff with
// +/- markers; enough to see what changed between two configs or scripts.
void FlipperCli::startDiff(const QString &pathA, const QString &pathB)
{
    readDeviceText(pathA, [this, pathA, pathB](bool okA, const QString &bodyA) {
        if (!okA) { appendOutput(QStringLiteral("[ no such file: %1 ]\n").arg(pathA) + prompt()); return; }
        readDeviceText(pathB, [this, pathA, pathB, bodyA](bool okB, const QString &bodyB) {
            if (!okB) { appendOutput(QStringLiteral("[ no such file: %1 ]\n").arg(pathB) + prompt()); return; }
            const QStringList la = bodyA.split(QLatin1Char('\n'));
            const QStringList lb = bodyB.split(QLatin1Char('\n'));
            // Simplest useful diff: walk both, mark lines that differ. Not an
            // LCS, but on a Flipper's small config files that reads fine and
            // keeps the code tiny.
            QStringList out;
            const int n = qMax(la.size(), lb.size());
            int diffs = 0;
            for (int i = 0; i < n; ++i) {
                const QString a = i < la.size() ? la.at(i) : QString();
                const QString b = i < lb.size() ? lb.at(i) : QString();
                if (a == b) { continue; }
                ++diffs;
                if (i < la.size()) { out += QStringLiteral("- %1").arg(a); }
                if (i < lb.size()) { out += QStringLiteral("+ %1").arg(b); }
            }
            if (diffs == 0) { appendOutput(QStringLiteral("[ files are identical ]\n") + prompt()); return; }
            appendOutput(out.join(QLatin1Char('\n')) + QLatin1Char('\n') + prompt());
        });
    });
}

// file -- identify a file by its leading bytes (magic numbers), the way Unix's
// file(1) does. The Flipper's own formats are text with a "Filetype:" header,
// so check that too -- it is more useful here than libmagic would be.
void FlipperCli::startFileType(const QString &path)
{
    readDeviceText(path, [this, path](bool ok, const QString &body) {
        if (!ok) { appendOutput(QStringLiteral("[ no such file: %1 ]\n").arg(path) + prompt()); return; }
        QString verdict;
        const QByteArray head = body.left(16).toUtf8();
        auto starts = [&](const char *sig) { return head.startsWith(sig); };
        if (body.startsWith(QLatin1String("Filetype:"))) {
            // Flipper's own asset format -- the second token names it.
            const QString first = body.section(QLatin1Char('\n'), 0, 0);
            verdict = QStringLiteral("Flipper asset (%1)").arg(first.section(QLatin1Char(':'), 1).trimmed());
        } else if (starts("\x89PNG"))            { verdict = QStringLiteral("PNG image"); }
        else if (starts("\xFF\xD8\xFF"))          { verdict = QStringLiteral("JPEG image"); }
        else if (starts("PK\x03\x04"))            { verdict = QStringLiteral("ZIP archive"); }
        else if (starts("\x1F\x8B"))              { verdict = QStringLiteral("gzip archive"); }
        else if (starts("%PDF"))                  { verdict = QStringLiteral("PDF document"); }
        else if (body.startsWith(QLatin1String("#!")))       { verdict = QStringLiteral("script (shebang)"); }
        else {
            bool binary = false;
            for (const QChar c : body.left(512)) {
                if (c.unicode() == 0 || (c.unicode() < 9)) { binary = true; break; }
            }
            verdict = binary ? QStringLiteral("binary data") : QStringLiteral("ASCII text");
        }
        appendOutput(QStringLiteral("%1: %2\n").arg(path, verdict) + prompt());
    });
}

// locate -- Linux locate reads a prebuilt index (updatedb); there is none here,
// so this is find rooted at /ext, i.e. "search the whole card". Same result,
// honest mechanism.
void FlipperCli::startLocate(const QString &pattern)
{
    QString pat = pattern;
    if (!pat.contains(QLatin1Char('*')) && !pat.contains(QLatin1Char('?'))) {
        pat = QLatin1Char('*') + pat + QLatin1Char('*');   // substring match, like real locate
    }
    startFind(QStringLiteral("/ext"), pat);
}

// Host programs. The Flipper has no OS, no processes and no network stack, so
// ping/ip/uname/time can't run ON it -- but they can run on THIS computer and
// have their output piped into the terminal, which is what the user actually
// wants to see. Every one of these prints where it ran so nobody mistakes it
// for the Flipper answering.
void FlipperCli::runHostProgram(const QString &label, const QString &program,
                                const QStringList &args, int timeoutMs)
{
    const QStringList extraDirs = {
        QStringLiteral("/sbin"), QStringLiteral("/usr/sbin"),
        QStringLiteral("/bin"), QStringLiteral("/usr/bin"),
        // Homebrew (Apple silicon + Intel) -- where pm3 and friends usually land.
        QStringLiteral("/opt/homebrew/bin"), QStringLiteral("/usr/local/bin")
    };
    QString exe = QStandardPaths::findExecutable(program);
    if (exe.isEmpty()) { exe = QStandardPaths::findExecutable(program, extraDirs); }
    // pm3 is sometimes installed under its long name.
    if (exe.isEmpty() && program == QLatin1String("pm3")) {
        exe = QStandardPaths::findExecutable(QStringLiteral("proxmark3"), extraDirs);
    }
    if (exe.isEmpty()) {
        appendOutput(QStringLiteral("[ %1 isn't installed on this computer (looked on PATH, "
                                    "Homebrew and the system dirs) ]\n").arg(program) + prompt());
        return;
    }
    appendOutput(QStringLiteral("[ %1 -- running on this computer, not the Flipper ]\n").arg(label));

    auto *proc = new QProcess(this);
    proc->setProcessChannelMode(QProcess::MergedChannels);
    auto *guard = new QTimer(this);
    guard->setSingleShot(true);
    guard->setInterval(timeoutMs);

    connect(guard, &QTimer::timeout, this, [proc]() {
        if (proc->state() != QProcess::NotRunning) { proc->kill(); }
    });
    connect(proc, &QProcess::readyReadStandardOutput, this, [this, proc]() {
        appendOutput(QString::fromUtf8(proc->readAll()));
    });
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            [this, proc, guard](int, QProcess::ExitStatus) {
        appendOutput(QString::fromUtf8(proc->readAll()));
        appendOutput(prompt());
        guard->deleteLater();
        proc->deleteLater();
    });
    proc->start(exe, args);
    guard->start();
}

void FlipperCli::startWget(const QString &url, const QString &devPath, bool exactDest)
{
    QUrl u(url);
    if (!u.isValid() || u.scheme().isEmpty()) { u = QUrl(QStringLiteral("https://") + url); }
    if (!u.isValid() || (u.scheme() != QLatin1String("http") && u.scheme() != QLatin1String("https"))) {
        appendOutput(QStringLiteral("[ not a http(s) URL: %1 ]\n").arg(url) + prompt());
        return;
    }

    appendOutput(QStringLiteral("[ downloading %1 ]\n").arg(u.toString()));
    QNetworkRequest req(u);
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("lotei-cli"));

    QNetworkReply *reply = m_dl.get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply, devPath, exactDest]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            appendOutput(QStringLiteral("[ download failed: %1 ]\n").arg(reply->errorString()) + prompt());
            return;
        }
        const QByteArray data = reply->readAll();
        if (data.isEmpty()) {
            appendOutput(QStringLiteral("[ the server sent an empty body ]\n") + prompt());
            return;
        }
        // Same 1 MiB ceiling the CLI upload path enforces -- say so here rather
        // than downloading first and refusing afterwards.
        if (data.size() > 1024 * 1024) {
            appendOutput(QStringLiteral("[ %1 bytes -- over the 1 MB CLI upload limit. Save it on this computer and use the file tools. ]\n")
                             .arg(data.size())
                         + prompt());
            return;
        }
        QTemporaryFile tmp(QDir::tempPath() + QStringLiteral("/lotei-wget-XXXXXX"));
        if (!tmp.open()) {
            appendOutput(QStringLiteral("[ can't create a temp file on this computer ]\n") + prompt());
            return;
        }
        tmp.write(data);
        tmp.flush();
        uploadToFlipper(tmp.fileName(), devPath, exactDest);
    });
}

void FlipperCli::clearOutput()
{
    if (m_output.isEmpty()) { return; }
    m_output.clear();
    emit outputChanged();
}

void FlipperCli::appendOutput(const QString &text)
{
    m_output += text;

    // No blank rows, ever. The firmware pads its replies with a spare newline
    // before the prompt and the reformatters add their own, which is what left
    // an empty line hanging above every prompt after a command like "ls".
    // Only the newly appended tail is scanned -- everything before it was
    // normalised on the way in, and re-running this over the whole buffer on
    // every serial chunk is real work during a streaming command like "top".
    static const QRegularExpression blankRows(QStringLiteral("\\n[ \\t]*(?:\\n[ \\t]*)+"));
    const int scanFrom = qMax(0, m_output.size() - text.size() - 64);
    QString tail = m_output.mid(scanFrom);
    tail.replace(blankRows, QStringLiteral("\n"));
    m_output.truncate(scanFrom);
    m_output += tail;
    while (m_output.startsWith(QLatin1Char('\n'))) { m_output.remove(0, 1); }

    // Collapse a prompt that lands straight after another one. The old version
    // matched three exact spellings of the gap between them (p+p, p+\n+p,
    // rtrim+\n+p), so a single stray space from the firmware was enough for a
    // duplicate to slip through. Two prompts with nothing but whitespace
    // between them means nothing was typed or printed in between, so the
    // earlier one is always the redundant one.
    const QString p = prompt();
    const QString rtrim = p.trimmed();
    if (!rtrim.isEmpty()) {
        for (;;) {
            int e = m_output.size();
            while (e > 0 && m_output.at(e - 1).isSpace()) { --e; }
            if (e < rtrim.size() || QStringView{m_output}.mid(e - rtrim.size(), rtrim.size()) != QStringView{rtrim}) { break; }
            const int lastStart = e - rtrim.size();

            int w = lastStart;
            while (w > 0 && m_output.at(w - 1).isSpace()) { --w; }
            if (w < rtrim.size() || QStringView{m_output}.mid(w - rtrim.size(), rtrim.size()) != QStringView{rtrim}) { break; }

            m_output.remove(w - rtrim.size(), lastStart - (w - rtrim.size()));
        }
    }
    // Whatever the user types next belongs on the prompt's own line, so the
    // prompt is never the second-to-last thing in the buffer.
    if (m_output.endsWith(p + QLatin1Char('\n')))      { m_output.chop(1); }
    else if (m_output.endsWith(rtrim + QLatin1Char('\n'))) { m_output.chop(1); }
    if (m_output.size() > 20000) {
        m_output = m_output.right(16000);
        // Drop the partial first line. A buffer starting mid-line can hand the
        // colouriser a fragment that matches a rule the whole line would not.
        const int nl = m_output.indexOf(QLatin1Char('\n'));
        if (nl >= 0) { m_output.remove(0, nl + 1); }
    }

    // Rate-limit the repaint. Streaming commands (top, log) push chunks far
    // faster than the view can lay out 16k characters, which is what makes the
    // panel feel locked up.
    if (!m_outputTick) {
        m_outputTick = new QTimer(this);
        m_outputTick->setSingleShot(true);
        m_outputTick->setInterval(60);
        connect(m_outputTick, &QTimer::timeout, this, &FlipperCli::outputChanged);
    }
    if (!m_outputTick->isActive()) { m_outputTick->start(); }
}

void FlipperCli::setActive(bool v)
{
    if (m_active != v) { m_active = v; emit activeChanged(); }
}

void FlipperCli::setStatus(const QString &s)
{
    if (m_status != s) { m_status = s; emit statusChanged(); }
}