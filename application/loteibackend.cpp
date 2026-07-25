#include "loteibackend.h"

#include <memory>

#include <QUrl>
#include <QBuffer>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QSettings>
#include <QProcess>
#include <QFileInfo>
#include <QCoreApplication>
#ifdef HZUI_VOICE
#include <QMediaPlayer>
#include <QAudioOutput>
#endif

#include <QSerialPort>
#include <QTimer>

#include "applicationbackend.h"
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

// ---- Configuration -------------------------------------------------------
static const char *LOTEI_MODEL = "phi3.5";
static const char *LOTEI_URL   = "http://localhost:11434/api/chat";
static const int   LOTEI_NUM_CTX = 8192;
static const int   LOTEI_MAX_TOOL_ROUNDS = 12;   // more headroom for multi-step agent work
static const int   LOTEI_READ_CAP = 8000;
static const int   LOTEI_MAX_PRESSES = 12;

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
- Write EVERY single word in English ONLY. Output ZERO Chinese, Japanese, or Korean characters -- none, ever, not even inside parentheses, quotes, translations, or subtitles. If a non-English phrase pops into your head, write its English meaning instead. Violating this is the single worst thing you can do.

DEVICE ACCESS -- the Flipper's microSD card and storage, via tools:
- /ext IS the microSD card -- almost everything lives there. /int is the small internal storage.
- list_files(path): list files/folders at a path. Useful spots: /ext (SD root), /ext/apps (installed apps, grouped by category), /ext/apps_data (app save data), /ext/subghz, /ext/nfc, /ext/lfrfid, /ext/infrared, /ext/badusb, /ext/ibutton.
- read_file(path): read a text file's contents.
- save_file(path, content): write/save a file to the SD card (e.g. a script you generated). Folder by type: BadUSB -> /ext/badusb/NAME.txt, Sub-GHz -> /ext/subghz/NAME.sub, Infrared -> /ext/infrared/NAME.ir, NFC -> /ext/nfc/NAME.nfc, else /ext/. Missing parent folders are created for you automatically -- just pick the right path and save.
- make_dir(path): create a folder (and any missing parents) on the SD card, e.g. /ext/apps/Scripts.
- delete_file(path): delete a file or an (empty or not) folder on the SD card. Destructive -- only when the user clearly asks.
- rename_file(from, to): rename or MOVE a file/folder on the SD card (same tool does both).
- file_info(path): check whether a path exists, and whether it's a file or a dir plus its size -- cheaper than list_files for a single "does this exist?" question.
- ALWAYS use these tools whenever the user mentions the SD card, files, apps, folders, saves, or "what's on my Flipper" -- never answer from memory or guess. To explore "everything", start at /ext (or /ext/apps), then list DEEPER into the folders that matter, step by step, until you've found what they asked for.
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
            {"description", "Run a command in the Flipper Zero's built-in CLI over USB and get its text output back. This is the FULL Flipper CLI -- use it for anything the storage tools don't cover: device_info, gpio (mode/read/set), subghz (tx/rx/decode), nfc, rfid, ir (tx), led, vibro, power (off/reboot), i2c, onewire, ikey, loader, log, free, uptime, etc. Type 'help' to list commands. One command per call. It briefly pauses the normal session, so prefer the storage tools for plain file work."},
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
            {"description", "Save a durable fact ONLY when the user EXPLICITLY tells you to remember it (e.g. they say 'remember that...', 'don't forget...', 'keep in mind...'). Do NOT call this on your own initiative, do NOT infer or invent facts, do NOT save nicknames, names, or preferences the user did not clearly state. If in doubt, do not call it."},
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
static bool messageNeedsTools(const QString &text)
{
    const QString t = text.toLower();

    // Strong signals: an explicit path, a file extension, or a Flipper subsystem
    // name almost always means "do something with this" -> tools on, verb or not.
    static const QStringList strongNouns = {
        QStringLiteral("/ext"), QStringLiteral("/int"), QStringLiteral(".txt"),
        QStringLiteral(".sub"), QStringLiteral(".nfc"), QStringLiteral(".ir"),
        QStringLiteral("badusb"), QStringLiteral("ducky"), QStringLiteral("subghz"),
        QStringLiteral("sub-ghz"),
        // CLI-driven hardware commands (run_cli)
        QStringLiteral("cli"), QStringLiteral("device_info"), QStringLiteral("gpio"),
        QStringLiteral("vibro"), QStringLiteral("vibra"), QStringLiteral("reboot"),
        QStringLiteral("reinicia"), QStringLiteral(" led "), QStringLiteral("i2c"),
        QStringLiteral("onewire"), QStringLiteral("uptime"), QStringLiteral("bateria"),
        QStringLiteral("battery")
    };
    for (const QString &s : strongNouns) {
        if (t.contains(s)) { return true; }
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
        // PT triggers (user speaks Portuguese too)
        QStringLiteral("salva"), QStringLiteral("cria"), QStringLiteral("criar"),
        QStringLiteral("faz"), QStringLiteral("escreve"), QStringLiteral("lista"),
        QStringLiteral("mostra"), QStringLiteral("abre"), QStringLiteral("apaga"),
        QStringLiteral("deleta"), QStringLiteral("renomeia"), QStringLiteral("aperta"),
        QStringLiteral("navega"), QStringLiteral("gera")
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
        QStringLiteral(".nfc"), QStringLiteral(".ir"), QStringLiteral("app"),
        QStringLiteral("arquivo"), QStringLiteral("pasta"), QStringLiteral("botao"),
        QStringLiteral("botão"), QStringLiteral("cartao")
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
                    {"content", "REM open TextEdit on macOS via Spotlight and type a note\nDELAY 1000\nGUI SPACE\nDELAY 500\nSTRING TextEdit\nENTER\nDELAY 1500\nGUI n\nDELAY 800\nSTRING oi tudo bem"}
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

void LoteiBackend::setAppBackend(ApplicationBackend *backend) { m_appBackend = backend; }

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
        // Self-heal: if the saved model isn't installed in Ollama anymore (e.g. an
        // old qwen the user removed), fall back to the default -- or the first model
        // available -- and persist it, so we never stay stuck on a deleted model.
        auto baseAvailable = [&found](const QString &m) {
            const QString base = m.section(QLatin1Char(':'), 0, 0);
            for (const QString &f : found) {
                if (f == m || f.section(QLatin1Char(':'), 0, 0) == base) { return true; }
            }
            return false;
        };
        if (!found.isEmpty() && !baseAvailable(m_model)) {
            const QString def = QString::fromUtf8(LOTEI_MODEL);
            m_model = baseAvailable(def) ? def : found.first();
            QSettings().setValue(QStringLiteral("lotei/model"), m_model);
            emit modelChanged();
        }
    });
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
// /ext/lotei/memoria.txt so it's portable with the device.
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
        sys += QStringLiteral("\n\nTHIS TURN IS CONVERSATION: you have NO tools available right now. "
                              "Reply in plain words only -- short and direct. Do NOT write any function "
                              "call, code, script or file path. Just answer.");
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
    QString name;
    static const QRegularExpression nameRe(QStringLiteral("(?m)^Name:\\s*(.+)$"));
    const QRegularExpressionMatch nm = nameRe.match(m_deviceContext);
    if (nm.hasMatch()) { name = nm.captured(1).trimmed(); }
    if (name.isEmpty()) { name = m_manualName; }
    if (!name.isEmpty()) {
        sys += QStringLiteral("\n\nYOUR NAME -- IMPORTANT: you are bonded to a Flipper Zero named "
            "\"%1\", so your name is %1 (NOT LOTEI -- that's just your underlying model line). "
            "Introduce yourself as %1 when greeting. But ALWAYS speak in the FIRST PERSON -- say "
            "\"I\", \"me\", \"my\", never talk about yourself in the third person. NEVER write things "
            "like \"%1 is on it\" or \"%1 garante\"; say \"I'm on it\", \"I've got it\". You ARE %1, "
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
        sys += QStringLiteral("\n\nWHAT YOU REMEMBER about this user (durable facts from past sessions -- "
            "use them without being asked; they're already true):\n") + m_memory;
    }

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
    m_turnNeedsTools = messageNeedsTools(userText);   // action -> tools; talk -> no tools
    m_history.append(QJsonObject{{"role", "user"}, {"content", userText}});
    setThinking(true);
    dispatchToOllama();
}

void LoteiBackend::dispatchToOllama()
{
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
    if (!m_noToolModels.contains(m_model) && m_turnNeedsTools) {
        body["tools"] = loteiTools(agentReady());
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
    QString text = stripNonEnglish(m_streamContent);
    if (text.isEmpty()) {
        text = QStringLiteral("...(LOTEI flicks his tail; nothing to say)");
    }
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
        return QStringLiteral("%1 needs more memory than you have free. Try a smaller brain — run "
                              "`ollama pull phi3.5`, then click my model name to switch. (Ollama said: %2)")
                .arg(model, raw);
    }
    // Model was never pulled.
    if (low.contains(QStringLiteral("not found"))) {
        return QStringLiteral("the model %1 isn't downloaded yet. Run `ollama pull %1` in a terminal, "
                              "then poke me again.").arg(model);
    }
    // Tools: we already retry tools-less once; if we still land here, say so plainly.
    if (low.contains(QStringLiteral("tool"))) {
        return QStringLiteral("%1 can't use my Flipper tools. Chat still works — for the device tools, "
                              "pick a tool-capable model like phi3.5. (Ollama said: %2)").arg(model, raw);
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
            msg = QStringLiteral("my brain (Ollama) isn't awake. Launch me with the LOTEI shortcut.");
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
        emit errorOccurred(QStringLiteral("...(LOTEI lost his train of thought)"));
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
            connect(op, &AbstractOperation::finished, this, [op, buf, path, done]() {
                QString result;
                if (op->isError()) {
                    result = QStringLiteral("{\"error\":\"%1\"}").arg(op->errorString());
                } else {
                    result = QStringLiteral("{\"saved\":\"%1\"}").arg(QString::fromUtf8(path));
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
// memory to the Flipper SD at /ext/lotei/memoria.txt when a device is around.
void LoteiBackend::rememberFact(const QString &fact)
{
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
        QStringLiteral("^(ok|okay|sure|yes|no|sim|nao|não|thanks|obrigad|hello|oi|hi|hey|done|pronto|"
                       "got it|entendi|beleza|blz|tudo bem|hmm+|lol|kk+|test(e|ing)?)\\b"),
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
        const QByteArray memPath = "/ext/lotei/memoria.txt";
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
    if (m_memory.trimmed().isEmpty()) { return 0; }
    QStringList lines = m_memory.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    const int before = lines.size();

    const QString m = match.trimmed();
    if (m.isEmpty() || m.compare(QLatin1String("all"), Qt::CaseInsensitive) == 0
        || m.compare(QLatin1String("tudo"), Qt::CaseInsensitive) == 0) {
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
        const QByteArray memPath = "/ext/lotei/memoria.txt";
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
            if (op->isError()) { emit scriptSaveError(op->errorString()); }
            else               { emit scriptSaved(path); }
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
    connect(op, &AbstractOperation::finished, this, [this, op, buf, path]() {
        if (op->isError()) { emit fileEditError(op->errorString()); }
        else               { emit fileSaved(path); }
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
          QStringLiteral("Stock Flipper Devices firmware."),      {},  rel, {}, {}, {}, {} },
        { QStringLiteral("Momentum"),    Kind::DirJson,
          QStringLiteral("https://up.momentum-fw.dev/firmware/directory.json"),
          QStringLiteral("Feature-rich community firmware."),     {},  rel, {}, {}, {}, {} },
        { QStringLiteral("Unleashed"),   Kind::GitHub,
          QStringLiteral("DarkFlippers/unleashed-firmware"),
          QStringLiteral("Popular unlocked community firmware."), git, rel, {}, {}, {}, {} },
        { QStringLiteral("RogueMaster"), Kind::GitHub,
          QStringLiteral("RogueMaster/flipperzero-firmware-wPlugins"),
          QStringLiteral("Everything, plus the kitchen sink."),   git, rel, {}, {}, {}, {} },
        // ARF ships only dev-tagged releases, so give it a single "dev" channel.
        { QStringLiteral("ARF"),         Kind::GitHub,
          QStringLiteral("D4C1-Labs/Flipper-ARF"),
          QStringLiteral("Automotive research: car keyfobs / Sub-GHz. Niche."),
          { QStringLiteral("dev") }, QStringLiteral("dev"), {}, {}, {}, {} },
        // Xero publishes versioned releases (flipper-z-f7-update-local.tgz).
        { QStringLiteral("Xero"),        Kind::GitHub,
          QStringLiteral("noproto/xero-firmware"),
          QStringLiteral("Lean official-based community firmware."),
          { QStringLiteral("release") }, QStringLiteral("release"), {}, {}, {}, {} },
    };

    // Restore each firmware's remembered channel choice.
    QSettings st;
    for (Source &s : m_sources) {
        const QString saved = st.value(QStringLiteral("firmware/ch/") + s.name).toString();
        if (!saved.isEmpty()) { s.wantChannel = saved; }
    }
}

void FirmwareStore::setOpen(bool value)
{
    if (value == m_open) { return; }
    m_open = value;
    emit openChanged();
    if (m_open) { refresh(); }   // freshen versions each time the panel opens
}

void FirmwareStore::setBusy(bool value)
{
    if (value == m_busy) { return; }
    m_busy = value;
    emit busyChanged();
}

QVariantList FirmwareStore::sources() const
{
    QVariantList out;
    for (const Source &s : m_sources) {
        QVariantMap m;
        m.insert(QStringLiteral("name"), s.name);
        m.insert(QStringLiteral("blurb"), s.blurb);
        m.insert(QStringLiteral("latest"), s.latest);
        m.insert(QStringLiteral("status"), s.status);
        m.insert(QStringLiteral("ready"), s.status == QLatin1String("ready") && !s.tgzUrl.isEmpty());
        m.insert(QStringLiteral("channel"), fwChannelLabel(currentChannelId(s)));
        m.insert(QStringLiteral("channelCount"), s.channels.size());
        out.append(m);
    }
    return out;
}

void FirmwareStore::refresh()
{
    for (int i = 0; i < m_sources.size(); ++i) {
        m_sources[i].status = QStringLiteral("checking");
        m_sources[i].latest.clear();
        m_sources[i].tgzUrl.clear();
    }
    emit changed();
    for (int i = 0; i < m_sources.size(); ++i) { fetchOne(i); }
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
            s.status = QStringLiteral("error");
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
    if (s.raw.isEmpty()) { s.status = QStringLiteral("error"); return; }

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
    } else {   // GitHub: "dev" = newest release, "release" = newest non-prerelease
        const QJsonArray rels = QJsonDocument::fromJson(s.raw).array();
        QJsonObject chosen;
        if (ch == QLatin1String("dev")) {
            if (!rels.isEmpty()) { chosen = rels.first().toObject(); }
        } else {
            for (const QJsonValue &rv : rels) {
                const QJsonObject r = rv.toObject();
                if (!r.value(QStringLiteral("prerelease")).toBool()) { chosen = r; break; }
            }
            if (chosen.isEmpty() && !rels.isEmpty()) { chosen = rels.first().toObject(); }
        }
        if (!chosen.isEmpty()) {
            s.latest = chosen.value(QStringLiteral("tag_name")).toString();
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

    if (src.status != QLatin1String("ready") || src.tgzUrl.isEmpty()) {
        emit failed(index, QStringLiteral("No downloadable build found -- try re-checking."));
        return;
    }
    if (m_busy) {
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

    setBusy(true);
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
            emit failed(index, QStringLiteral("Download failed: %1").arg(reply->errorString()));
            return;
        }
        QFile f(outPath);
        if (!f.open(QIODevice::WriteOnly)) {
            emit failed(index, QStringLiteral("Couldn't save the download to disk."));
            return;
        }
        f.write(reply->readAll());
        f.close();
        emit progress(index, 1.0, QStringLiteral("Ready -- flashing…"));
        emit readyToInstall(QUrl::fromLocalFile(outPath).toString());
    });
}

// ===================== FlipperCli: in-app Flipper text CLI =====================

FlipperCli::FlipperCli(QObject *parent)
    : QObject(parent)
{
}

void FlipperCli::setOpen(bool value)
{
    if (m_open == value) { return; }
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

    // Hand the serial line off from RPC to us: releasePort() stops the RPC
    // session, which closes the COM port and drops the Flipper back to its CLI.
    setStatus(QStringLiteral("Pausing qFlipper's session and opening the CLI…"));
    // (device session pauses silently while the CLI is open)
    m_appBackend->releasePort();

    // Give the RPC teardown a moment to actually free the port, then take it over.
    QTimer::singleShot(700, this, [this, portInfo]() {
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
        setActive(true);
        setStatus(QStringLiteral("CLI live -- type a command (try 'help')."));
        m_port->write("\r\n");   // nudge a fresh prompt
    });
}

void FlipperCli::disconnectCli()
{
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
    QTimer::singleShot(700, this, [this, portInfo, cmd]() {
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
        m_runPort->write(cmd.toUtf8());
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
    // "clear" / "cls" clears the on-screen CLI view (the Flipper firmware has no
    // clear command) instead of being sent to the device.
    const QString t = cmd.trimmed().toLower();
    if (t == QLatin1String("clear") || t == QLatin1String("cls")) {
        clearOutput();
        appendOutput(QStringLiteral(">: "));
        return;
    }
    if (!m_port || !m_active) { return; }
    m_port->write(cmd.toUtf8());
    m_port->write("\r\n");
}

void FlipperCli::interrupt()
{
    if (!m_port || !m_active) { return; }
    m_port->write("\x03");   // Ctrl-C
}

void FlipperCli::onReadyRead()
{
    if (!m_port) { return; }
    QString text = QString::fromUtf8(m_port->readAll());
    // Strip ANSI escape sequences (colours, cursor moves) for a clean text view.
    static const QRegularExpression ansi(QStringLiteral("\x1B\\[[0-9;?]*[A-Za-z]"));
    text.remove(ansi);
    text.remove(QLatin1Char('\r'));
    // Strip stray C0/C1 control characters -- this is the little square the
    // firmware emits next to the prompt.
    static const QRegularExpression ctrl(QStringLiteral("[\\x00-\\x08\\x0B\\x0C\\x0E-\\x1F\\x7F]"));
    text.remove(ctrl);
    // Add our client-side "clear" into the device's help listing so it shows up
    // among the available commands.
    if (text.contains(QLatin1String("Commands available:"))) {
        text.replace(QLatin1String("Commands available:"),
                     QStringLiteral("Commands available:\nclear"));
    }
    appendOutput(text);
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
    // Collapse a bare repeated prompt ("\n>: \n>: " -> one) so we don't show
    // two ">" lines stacked with nothing between them.
    static const QRegularExpression dupPrompt(QStringLiteral(">:[ \\t]*\\n(>: )"));
    m_output.replace(dupPrompt, QStringLiteral("\\1"));
    if (m_output.size() > 20000) { m_output = m_output.right(16000); }
    emit outputChanged();
}

void FlipperCli::setActive(bool v)
{
    if (m_active != v) { m_active = v; emit activeChanged(); }
}

void FlipperCli::setStatus(const QString &s)
{
    if (m_status != s) { m_status = s; emit statusChanged(); }
}