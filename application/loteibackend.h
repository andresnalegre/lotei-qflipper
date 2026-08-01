#pragma once

#include <functional>

#include <QObject>
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QColor>
#include <QDateTime>
#include <QVariant>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkAccessManager>
#ifdef HZUI_VOICE
#include <QTextToSpeech>
#include <QVoice>
#endif
#include <QTimer>

class ApplicationBackend;
class QNetworkReply;
class QProcess;
class QMediaPlayer;
class QAudioOutput;

// LOTEI - a local-AI (Ollama) chat assistant inside qFlipper, with tool access
// to live-query the connected Flipper Zero over qFlipper's RPC link.
class FlipperCli;   // defined below; LoteiBackend holds a pointer for run_cli

class LoteiBackend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool thinking READ thinking NOTIFY thinkingChanged)
    Q_PROPERTY(bool configured READ configured CONSTANT)
    Q_PROPERTY(bool hasAudio READ hasAudio CONSTANT)   // false on Linux (no QtMultimedia): hides voice + music
    Q_PROPERTY(bool muted READ muted WRITE setMuted NOTIFY mutedChanged)
    Q_PROPERTY(QString voiceName READ voiceName NOTIFY voiceChanged)
    Q_PROPERTY(QString modelName READ modelName NOTIFY modelChanged)
    Q_PROPERTY(qreal voiceVolume READ voiceVolume WRITE setVoiceVolume NOTIFY voiceVolumeChanged)
    Q_PROPERTY(qreal musicVolume READ musicVolume WRITE setMusicVolume NOTIFY musicVolumeChanged)
    Q_PROPERTY(bool setupComplete READ setupComplete NOTIFY setupCompleteChanged)
    Q_PROPERTY(bool ollamaOnline READ ollamaOnline NOTIFY modelChanged)
    Q_PROPERTY(QString manualName READ manualName WRITE setManualName NOTIFY manualNameChanged)
    Q_PROPERTY(bool agentEnabled READ agentEnabled WRITE setAgentEnabled NOTIFY agentChanged)
    Q_PROPERTY(QString agentDir READ agentDir WRITE setAgentDir NOTIFY agentChanged)

public:
    explicit LoteiBackend(QObject *parent = nullptr);

    // Gives LOTEI access to the connected device (for the inspection tools).
    void setAppBackend(ApplicationBackend *backend);
    void setCli(FlipperCli *cli);   // link the CLI for run_cli

    bool thinking() const;
    bool configured() const;
    bool hasAudio() const;   // true only where voice/music (QtMultimedia) is compiled in
    bool muted() const;
    void setMuted(bool value);
    QString voiceName() const;
    QString modelName() const;
    qreal voiceVolume() const;
    void setVoiceVolume(qreal value);
    qreal musicVolume() const;
    void setMusicVolume(qreal value);
    bool setupComplete() const;
    bool ollamaOnline() const;
    QString manualName() const;
    void setManualName(const QString &name);
    bool agentEnabled() const;                       // host-workspace (self-edit) tools on?
    QString agentDir() const;                        // workspace root LOTEI may touch
    void setAgentEnabled(bool on);
    void setAgentDir(const QString &dir);
    Q_INVOKABLE QString extractScript(const QString &text) const;   // pull code from a message
    Q_INVOKABLE void saveScriptToFlipper(const QString &folder,     // manual, model-free save
                                         const QString &filename,
                                         const QString &content);
    Q_INVOKABLE void openFileForEdit(const QString &path);          // read a file into the editor
    Q_INVOKABLE void clearHistory();                                // wipe the chat conversation
    Q_INVOKABLE void writeFile(const QString &path, const QString &content); // save edits back

    Q_INVOKABLE void send(const QString &userText, const QString &deviceContext);
    Q_INVOKABLE void reset();
    Q_INVOKABLE void cycleVoice();
    Q_INVOKABLE void cycleModel();                    // switch to the next installed Ollama model
    Q_INVOKABLE void setModel(const QString &model);  // pick a specific model
    Q_INVOKABLE QStringList availableModels() const;  // models installed in Ollama
    Q_INVOKABLE QString musicFolderUrl() const;   // <appdir>/Music as a file URL

    // ---- Model manager (gear icon) ----------------------------------------
    // Curated catalog of downloadable Ollama models, each tagged with whether
    // it's currently installed. QML renders this as the model-manager list.
    Q_INVOKABLE QVariantList modelCatalog() const;
    Q_INVOKABLE void installModel(const QString &name);    // `ollama pull <name>`
    Q_INVOKABLE void uninstallModel(const QString &name);  // `ollama rm <name>`
    // Kills whatever install/uninstall/Ollama-install is currently running.
    // Works for all three since they share m_modelOpProc.
    Q_INVOKABLE void cancelModelOp();

    // Whether the Ollama binary itself was found on this machine (independent
    // of whether the server is currently answering /api/tags).
    Q_PROPERTY(bool ollamaInstalled READ ollamaInstalled NOTIFY ollamaInstalledChanged)
    bool ollamaInstalled() const;
    Q_INVOKABLE void installOllama();       // platform install script/package manager
    Q_INVOKABLE void detectOllama();        // re-check whether the binary is on PATH

    // Progress of whatever model pull/remove/Ollama-install is currently running.
    // One op at a time; op == "" when idle.
    Q_PROPERTY(QString modelOpName READ modelOpName NOTIFY modelOpChanged)
    Q_PROPERTY(QString modelOpKind READ modelOpKind NOTIFY modelOpChanged)      // "install" | "uninstall" | "ollama" | ""
    Q_PROPERTY(QString modelOpStatus READ modelOpStatus NOTIFY modelOpChanged)  // human-readable line
    Q_PROPERTY(double  modelOpProgress READ modelOpProgress NOTIFY modelOpChanged) // 0..1, or -1 = indeterminate
    QString modelOpName() const     { return m_modelOpName; }
    QString modelOpKind() const     { return m_modelOpKind; }
    QString modelOpStatus() const   { return m_modelOpStatus; }
    double  modelOpProgress() const { return m_modelOpProgress; }

    // First-run setup wizard
    Q_INVOKABLE void completeSetup();                    // mark the wizard done
    Q_INVOKABLE void resetSetup();                       // re-trigger the wizard (testing)
    Q_INVOKABLE void recheckOllama();                    // re-query /api/tags (AI step)
    Q_INVOKABLE QStringList personalityPresets() const;  // preset persona names
    Q_INVOKABLE void applyPreset(const QString &name);   // set persona to a preset
    Q_INVOKABLE void applyNamePersonality();             // persona built from the Flipper's name

    // These three are called from QML and were sitting under private:. Qt's
    // QML engine refuses to invoke them there, so every call threw and took
    // the rest of its handler down with it -- which is what stopped
    // Backend.installFirmware() from ever running.
    Q_INVOKABLE void logAction(const QString &what) const;
    Q_INVOKABLE void reloadMemory();           // re-read memory.txt from the card
    Q_INVOKABLE void syncMemoryToFlipper();    // back up memory + self to the SD

    // Copy the user's own files off the SD card. The stock Backup button saves
    // /int -- pairing keys, dolphin state, region -- which is settings, not the
    // captures, scripts and dumps people actually mean by "backup".
    // Backup, restore and format are minutes of work with no visible sign that
    // anything is happening -- the LOGS panel is not somewhere most people
    // think to look. These drive the same kind of progress screen the firmware
    // install uses.
    Q_PROPERTY(bool transferActive READ transferActive NOTIFY transferChanged)
    Q_PROPERTY(QString transferTitle READ transferTitle NOTIFY transferChanged)
    Q_PROPERTY(QString transferNote READ transferNote NOTIFY transferChanged)
    Q_PROPERTY(double transferProgress READ transferProgress NOTIFY transferChanged)
    bool transferActive() const { return m_transferActive; }
    QString transferTitle() const { return m_transferTitle; }
    QString transferNote() const { return m_transferNote; }
    double transferProgress() const { return m_transferProgress; }



    Q_INVOKABLE void backupSdCard();
    // Puts a backup folder back onto /ext. Takes the folder the picker returned.
    Q_INVOKABLE void restoreSdCard(const QString &folderUrl);
    // Empties the SD card. /int is reset separately, by the factory reset the
    // QML chains after this one finishes.
    Q_INVOKABLE void formatSdCard();
    // True once the card has been emptied and before anything is installed onto
    // it again: a reinstall repairs an existing install, and after a format
    // there is nothing on the card left to repair.
    Q_PROPERTY(bool sdFormatted READ sdFormatted NOTIFY sdFormattedChanged)
    bool sdFormatted() const { return m_sdFormatted; }

signals:
    void backupProgress(const QString &note, double frac);
    void backupFinished(const QString &path, int items);
    void backupFailed(const QString &error);
    void restoreProgress(const QString &note, double frac);
    void restoreFinished(int items);
    void restoreFailed(const QString &error);
    void formatProgress(const QString &note, double frac);
    void formatFinished();
    void formatFailed(const QString &error);
    void sdFormattedChanged();
    void transferChanged();

    void replyReceived(const QString &text);
    void errorOccurred(const QString &text);
    void thinkingChanged();
    void mutedChanged();
    void voiceChanged();
    void modelChanged();
    void voiceVolumeChanged();
    void musicVolumeChanged();
    void setupCompleteChanged();
    void manualNameChanged();
    void agentChanged();
    void ollamaInstalledChanged();
    void modelOpChanged();                                  // progress tick, any op
    void modelInstallFinished(const QString &name, bool ok, const QString &message);
    void modelUninstallFinished(const QString &name, bool ok, const QString &message);
    void ollamaInstallFinished(bool ok, const QString &message);
    void scriptSaved(const QString &path);        // manual save succeeded
    void scriptSaveError(const QString &message);  // manual save failed
    void fileOpened(const QString &path, const QString &content); // editor: file read
    void fileSaved(const QString &path);           // editor: file written
    void fileEditError(const QString &message);    // editor: read/write failed
    void partialReceived(const QString &text);   // live-typing: reply text so far

private:
    void setThinking(bool value);
    // memory.txt can be edited by hand; these keep the in-memory copy honest.
    void applyMemoryText(const QString &text, const QString &source);
    void refreshMemoryFromDisk();
    void writeMemoryCache() const;
    bool adoptMemoryIfMemoryFile(const QString &path, const QString &content);

    QString systemPrompt() const;
    QString assistantName() const;   // device name -> who the log credits
    void applyProsody(const QString &text);   // nudge SAPI pitch/rate to match mood (fallback)
    void speak(const QString &text);          // route to Piper if present, else SAPI
    void speakWithPiper(const QString &spoken, const QString &moodText);
    double piperLengthScale(const QString &moodText) const;
    void discoverPiper();
    void refreshModels();   // query Ollama /api/tags for installed models

    // Model manager internals.
    void runModelOp(const QString &kind, const QString &name, const QStringList &args);
    void appendModelOpOutput(const QByteArray &chunk); // parse a QProcess output chunk
    void finishModelOp(bool ok, const QString &message);
    void setModelOp(const QString &kind, const QString &name,
                    const QString &status, double progress);

    void loadHistory();   // restore past conversation from disk
    void saveHistory();   // persist conversation (user + final replies only)

    void dispatchToOllama();                                   // POST history + tools
    void onStreamData(QNetworkReply *reply);      // parse streamed NDJSON chunks
    void onStreamFinished(QNetworkReply *reply);
    void finalizeStream();                        // a full response arrived
    void runToolCalls(const QJsonArray &toolCalls, int index); // execute tools sequentially
    void runOneTool(const QString &name, const QJsonObject &args,
                    std::function<void(const QString &)> done); // one async RPC tool
    void ensureFlipperDir(const QByteArray &dirPath,
                          std::function<void()> done);           // mkdir -p on the SD card

    // Host agent: edit/build/test LOTEI's own source, hard-sandboxed to a folder.
    bool agentReady() const;                                     // enabled + valid workspace
    QString resolveAgentPath(const QString &rel, bool mustExist) const; // contain to workspace
    void runHostTool(const QString &name, const QJsonObject &args,
                     std::function<void(const QString &)> done);
    void rememberFact(const QString &fact);   // append a durable fact to memory
    void loadPortableMemory();                 // pull memory/self from the Flipper SD
    int  forgetFacts(const QString &match);    // remove matching facts (or all); returns count

    QNetworkAccessManager m_net;
    ApplicationBackend *m_appBackend = nullptr;
    FlipperCli         *m_cli = nullptr;   // for the run_cli tool (set by Application)

    QJsonArray m_history;        // running messages (user / assistant / tool)
    QString    m_deviceContext;  // latest diagnostics snapshot from QML
    int        m_toolRounds = 0;
    bool       m_turnNeedsTools = false;   // set per turn by the intent router
    bool       m_thinking = false;
    bool       m_muted = false;
    qreal      m_voiceVolume = 1.0;
    qreal      m_musicVolume = 0.55;
    QString     m_model;    // selected Ollama model (persisted)
    QStringList m_models;   // models discovered via /api/tags
    QStringList m_noToolModels;  // models Ollama rejects tools for (e.g. Gemma) -> chat-only
    bool        m_setupComplete = false;
    bool        m_ollamaOnline = false;
    QString     m_manualName;   // Flipper name from setup (fallback when no device)
    bool        m_agentEnabled = false;  // host-workspace self-edit tools opt-in
    QString     m_agentRoot;             // absolute workspace folder LOTEI may edit
    // SD-card backup. Entries are copied one at a time so the device is never
    // asked for two transfers at once.
    // Walked by hand with storageList + storageRead rather than the utility's
    // downloadDirectory(): that returned a tree of empty folders and skipped
    // most entries outright, and without its source there was no way to tell
    // why. These two primitives are the ones already proven in this feature.
    QString     m_backupRoot;
    QStringList m_backupDirs;     // remote directories still to enumerate
    QList<QPair<QString,qint64>> m_backupPending;  // <remote path, size>
    int         m_backupSkipped = 0;
    int         m_backupTotal = 0;
    int         m_backupFiles = 0;
    qint64      m_backupBytes = 0;
    void backupEnumerateNext();
    void backupCopyNext();
    // Files land in a working folder first, then get packed into bkp.tgz and
    // the folder is dropped -- so a half-finished run never leaves a partial
    // archive sitting where a good one should be.
    void backupPackArchive();
    QString m_backupArchive;

    bool    m_transferActive = false;
    QString m_transferTitle;
    QString m_transferNote;
    double  m_transferProgress = -1.0;      // negative means indeterminate
    void    setTransfer(const QString &title, const QString &note, double frac);
    void    endTransfer();
    void    finishTransfer(bool ok, const QString &text);

    QStringList m_formatQueue;
    int         m_formatTotal = 0;
    bool        m_sdFormatted = false;
    void        runFormatQueue();

    QString     m_memory;                // durable facts to remember across sessions
    bool        m_portableLoaded = false; // loaded memory from the Flipper this session
#ifdef HZUI_VOICE
    QTextToSpeech m_tts;   // SAPI fallback engine
#endif

    // Piper neural TTS (primary, when piper.exe + voices sit next to the app)
    bool          m_piperOk = false;
    QString       m_piperExe;
    QStringList   m_piperVoices;
    int           m_piperVoiceIdx = 0;
    QProcess     *m_piperProc = nullptr;
    QMediaPlayer *m_voicePlayer = nullptr;
    QAudioOutput *m_voiceAudio = nullptr;
    QString       m_voiceTmpDir;
    int           m_voiceSeq = 0;

    QByteArray m_streamBuf;       // buffer for partial streamed lines
    QString    m_streamContent;   // accumulated reply text this response
    QJsonArray m_streamTools;     // accumulated tool calls this response
    QNetworkReply *m_currentReply = nullptr;

    // ---- Model manager --------------------------------------------------
    bool m_ollamaInstalled = false;
    bool m_ollamaChecked = false;
    // Absolute path to the CLI, resolved by detectOllama(). Empty == not found.
    // Used to launch pull/rm so they don't rely on the inherited PATH either.
    QString m_ollamaPath;

    QProcess   *m_modelOpProc = nullptr;   // pull / rm / install-ollama, one at a time
    QString     m_modelOpKind;             // "install" | "uninstall" | "ollama" | ""
    QString     m_modelOpName;             // model name, or "" for the Ollama install itself
    QString     m_modelOpStatus;
    double      m_modelOpProgress = -1.0;
    QByteArray  m_modelOpBuf;              // partial line buffer (pull output uses \r a lot)
    QByteArray  m_modelOpLine;             // bytes since the last line break, carried between reads
    bool        m_modelOpCancelled = false;  // user hit cancel -- report "cancelled", not exit code noise
};

// Runtime-editable color palette. Every Theme.color.* value flows from here, so
// changing one recolors the whole UI live. Persists to QSettings.
class LoteiPalette : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap colors READ colors NOTIFY changed)

public:
    explicit LoteiPalette(QObject *parent = nullptr);

    QVariantMap colors() const { return m_colors; }

    Q_INVOKABLE QStringList names() const { return m_order; }     // editable colors, in order
    Q_INVOKABLE QString hex(const QString &name) const;           // "#rrggbb" of a color
    Q_INVOKABLE void setColor(const QString &name, const QColor &c);
    Q_INVOKABLE void reset();                                     // back to the pink defaults

signals:
    void changed();

private:
    void load();
    void save() const;

    QVariantMap m_defaults;
    QVariantMap m_colors;
    QStringList m_order;
    QTimer     *m_saveTimer = nullptr;
};

// Tracks community Flipper firmwares (Official, Momentum, Unleashed, RogueMaster),
// fetches each one's latest version live, and downloads the update .tgz so
// qFlipper's normal installer (ApplicationBackend::installFirmware) can flash it.
// A Flipper runs one firmware at a time -- this makes the latest build of any
// fork one click away; it never flashes without an explicit click.
class FirmwareStore : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool open READ isOpen WRITE setOpen NOTIFY openChanged)
    Q_PROPERTY(QVariantList sources READ sources NOTIFY changed)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    // The version string the Flipper reports. Set from QML; every source in
    // sources() is then tagged as installed / up to date / neither, so the UI
    // can offer UPDATE instead of INSTALL for the firmware already running.
    Q_PROPERTY(QString deviceVersion READ deviceVersion WRITE setDeviceVersion NOTIFY changed)
    // Resolved from deviceVersion, for the main screen: which source is running,
    // what its newest build is, and whether that differs from what is installed.
    Q_PROPERTY(int installedIndex READ installedIndex NOTIFY changed)
    Q_PROPERTY(QString installedName READ installedName NOTIFY changed)
    Q_PROPERTY(QString installedLatest READ installedLatest NOTIFY changed)
    Q_PROPERTY(bool installedReady READ installedReady NOTIFY changed)
    Q_PROPERTY(bool updateAvailable READ updateAvailable NOTIFY changed)
    Q_PROPERTY(QString installedDate READ installedDate NOTIFY changed)
    // A firmware picked in the store but not yet flashed. The panel only ever
    // stages a choice; the install button on the main screen is what commits it.
    Q_PROPERTY(int selectedIndex READ selectedIndex NOTIFY changed)
    Q_PROPERTY(bool hasSelection READ hasSelection NOTIFY changed)
    Q_PROPERTY(QString selectedName READ selectedName NOTIFY changed)
    Q_PROPERTY(QString selectedVersion READ selectedVersion NOTIFY changed)
    Q_PROPERTY(QString selectedDate READ selectedDate NOTIFY changed)
    // Feedback for the "check for updates" action: how many lookups are still
    // out, and a one-line verdict once they all land.
    Q_PROPERTY(bool checking READ checking NOTIFY changed)
    Q_PROPERTY(QString checkSummary READ checkSummary NOTIFY changed)
    // The channels of whatever firmware is running, so the update-channel
    // control can describe the actual firmware instead of the official one.
    Q_PROPERTY(QStringList installedChannels READ installedChannels NOTIFY changed)
    // Same list shaped like the stock channel model ({name: ...}), so the
    // existing ChannelDelegate renders it unchanged.
    Q_PROPERTY(QVariantList installedChannelModel READ installedChannelModel NOTIFY changed)
    Q_PROPERTY(QString installedChannel READ installedChannel NOTIFY changed)
    // Which channel the running build was flashed from, when this app did it.
    Q_PROPERTY(QString installedFromChannel READ installedFromChannel NOTIFY changed)
    Q_PROPERTY(bool channelSwitchPending READ channelSwitchPending NOTIFY changed)

public:
    explicit FirmwareStore(QObject *parent = nullptr);

    bool isOpen() const { return m_open; }
    void setOpen(bool value);
    QVariantList sources() const;
    bool busy() const { return m_busy; }
    QString deviceVersion() const { return m_deviceVersion; }
    void setDeviceVersion(const QString &v);

    // Which source the running firmware came from, by the shape of its version
    // string ("mntm-012" -> Momentum). -1 when it can't be told.
    int installedIndex() const;
    QString installedName() const;
    QString installedLatest() const;
    bool installedReady() const;
    bool updateAvailable() const;
    QString installedDate() const;
    bool checking() const { return m_pending > 0; }
    QString checkSummary() const { return m_checkSummary; }
    QStringList installedChannels() const;
    QVariantList installedChannelModel() const;
    QString installedChannel() const;
    QString installedFromChannel() const;
    bool channelSwitchPending() const;
    Q_INVOKABLE void setInstalledChannel(const QString &id);
    // Re-flash the firmware already on the device, from its own source.
    Q_INVOKABLE void reinstallInstalled();

    int selectedIndex() const { return m_selected; }
    bool hasSelection() const { return m_selected >= 0 && m_selected < m_sources.size(); }
    QString selectedName() const;
    QString selectedVersion() const;
    QString selectedDate() const;

    // Toggles: picking the row that is already staged clears it.
    Q_INVOKABLE void select(int index);
    Q_INVOKABLE void clearSelection();
    Q_INVOKABLE void installSelected();

    Q_INVOKABLE void refresh();               // force a re-fetch of every source
    void refreshIfStale();                    // only go to the network if the cache aged out

    // Derived results survive a restart; the raw payloads are too big to keep
    // and don't need to be, since only these fields drive the UI and install.
    void rememberFlashedChannel(int index);
    void noteLookupDone(int index);   // tally a finished lookup; verdict on the last
    void saveDerived(int index) const;
    bool loadDerived(int index);
    Q_INVOKABLE void install(int index);      // download that source's latest .tgz
    Q_INVOKABLE void cycleChannel(int index); // switch a source's channel (release/dev/rc)

signals:
    void openChanged();
    void changed();
    void busyChanged();
    void readyToInstall(const QString &fileUrl);              // hand off to Backend.installFirmware
    void progress(int index, qreal frac, const QString &note);
    void failed(int index, const QString &message);

private:
    enum class Kind { DirJson, GitHub };
    struct Source {
        QString     name;
        Kind        kind;
        QString     locator;      // directory.json URL, or "owner/repo" for GitHub
        QString     blurb;
        QStringList channels;     // available channel ids (discovered for DirJson, fixed for GitHub)
        QString     wantChannel;  // user-selected channel id (persisted); default "release"
        QString     latest;       // discovered version for the selected channel
        QString     date;         // release date of that version, "yyyy-MM-dd"
        QString     tgzUrl;       // discovered download URL for the selected channel
        QString     status;       // "", "checking", "ready", "error"
        QByteArray  raw;          // cached payload, so channel switches need no re-fetch
    };

    void fetchOne(int index);
    void deriveFromCache(int index);                  // recompute latest/tgz for the chosen channel
    QString currentChannelId(const Source &s) const;  // wantChannel, clamped to what's available
    // Declared here, after struct Source: taking one by reference above the
    // struct's own declaration doesn't compile.
    QString channelVersion(const Source &s, const QString &ch) const;
    int distinctChannelCount(const Source &s) const;   // 1 when every channel is the same build
    void setBusy(bool value);

    QNetworkAccessManager m_net;
    QList<Source> m_sources;
    bool m_open = false;
    bool m_busy = false;
    QString m_deviceVersion;
    bool m_fetchedOnce = false;
    int m_selected = -1;
    int m_pending = 0;               // lookups still in flight
    int m_foundUpdates = 0;          // sources that differ from what is running
    QString m_checkSummary;
    // The GitHub sources share an unauthenticated 60-requests-per-hour budget
    // for the whole machine, and four of them are fetched per refresh. Opening
    // the panel used to spend that budget every time.
    QDateTime m_lastFetch;
};

class QSerialPort;

// In-app Flipper CLI terminal. The Flipper's serial line speaks a text CLI by
// default and switches to protobuf RPC on `start_rpc_session` -- the two share
// one line, which is why you can't open the CLI (PuTTY etc.) while qFlipper holds
// the port. This pauses the RPC session (ApplicationBackend::releasePort), opens
// the same USB serial in text mode at 230400, and pipes it to an in-app terminal;
// closing it restarts RPC. USB-only for now (BLE has a CLI mode too -- later).
// Exposed to QML as the singleton `Cli`.
class FlipperCli : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool open READ isOpen WRITE setOpen NOTIFY openChanged)
    Q_PROPERTY(bool active READ active NOTIFY activeChanged)   // serial link live
    Q_PROPERTY(QString output READ output NOTIFY outputChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(bool verbose READ verbose WRITE setVerbose NOTIFY verboseChanged)
    Q_PROPERTY(bool colored READ colored WRITE setColored NOTIFY coloredChanged)
    Q_PROPERTY(QString promptText READ promptText NOTIFY promptChanged)

public:
    explicit FlipperCli(QObject *parent = nullptr);

    void setAppBackend(ApplicationBackend *backend) { m_appBackend = backend; }

    bool isOpen() const { return m_open; }
    void setOpen(bool value);
    bool active() const { return m_active; }
    QString output() const { return m_output; }
    QString status() const { return m_status; }
    bool verbose() const { return m_verbose; }
    void setVerbose(bool value);
    bool colored() const { return m_colored; }
    void setColored(bool value);
    // The panel colours its own output, so it needs to know which prefix is the
    // prompt rather than guessing at it.
    QString promptText() const { return prompt(); }

    Q_INVOKABLE void send(const QString &cmd);   // write a command + CR
    Q_INVOKABLE void interrupt();                // send Ctrl-C
    Q_INVOKABLE void clearOutput();

    // Tab completion. The panel hands over the text to the left of the caret;
    // the reply on completion() is what that text becomes.
    Q_INVOKABLE void complete(const QString &line);
    Q_INVOKABLE QString clipboardText() const;                  // Cmd/Ctrl+V
    Q_INVOKABLE void copyToClipboard(const QString &text) const;

    // Run a single CLI command in isolation (used by the assistant). Pauses RPC,
    // opens the port, sends the command, collects output until the line goes idle,
    // then closes and hands RPC back. Calls done(ok, output) when finished. Refuses
    // if the interactive CLI panel is open or another one-shot is in flight.
    void runOneShot(const QString &cmd, std::function<void(bool, QString)> done);
    bool oneShotBusy() const { return m_runBusy; }

signals:
    void openChanged();
    void activeChanged();
    void outputChanged();
    void statusChanged();
    void verboseChanged();
    void coloredChanged();
    void promptChanged();
    // Tab completion result: the replacement for the text left of the caret.
    void completion(const QString &line);
    // "edit <path>" fetched the file -- whatever panel does host-side text
    // editing can hook this to pop it open instead of just printing it.
    void editRequested(const QString &path, const QString &content);

private slots:
    void onReadyRead();

private:
    void connectCli();     // release RPC, open the serial in CLI/text mode
    void disconnectCli();  // close the serial, hand RPC back
    void appendOutput(const QString &text);
    void setActive(bool v);
    void setStatus(const QString &s);
    void finishOneShot(bool ok, const QString &out);   // cleanup + callback

    // Verbose log. Every byte of command this panel puts on the wire, and every
    // reply it swallows internally, is echoed into the terminal so a multi-step
    // op (cp -r, rm -r, find, a transfer) shows its whole trail rather than just
    // the summary line at the end.
    // logIt is false for the one command that is just the user's own line
    // translated (ls -> storage list): the panel already showed what they
    // typed, so repeating the wire form is noise. Everything the CLI runs on
    // its own behalf still goes in the log.
    void writeLine(const QString &cmd, bool logIt = true);

    // One conversation at a time. Every mode below owns the serial line
    // exclusively, so this is the single predicate that decides whether a new
    // one may start -- the checks used to be spelled out per call site, which
    // is how "cd" in flight and a raw command could eat each other's replies.
    bool busy() const;

    // No-reply watchdog for the interactive path. Every step that waits on the
    // device arms it; every completion disarms it; incoming bytes restart it,
    // so a slow transfer survives but true silence does not. Without this, a
    // reply that never lands leaves the panel dead with no prompt and no error.
    void armGuard();
    void disarmGuard();
    void onOpTimeout();
    void resetTransientState();   // shared by Ctrl-C, timeout and disconnect
    void trace(const QString &what);         // one wire command
    void traceReply(const QString &raw);     // what came back from it

    // Tab completion: shared tail for both the command-name and the path case.
    void applyCompletion(const QString &head, const QString &token,
                         const QStringList &hits, const QList<bool> &isDir,
                         const QString &original);

    // The firmware streams multi-line replies across several serial chunks, so
    // help listings and directory listings are held back and laid out in one
    // pass once the prompt returns.
    void flushCapture();

    // Host <-> Flipper file copy, driven over the plain CLI (RPC is paused while
    // the panel owns the port). Every transfer is verified with "storage md5"
    // against a local QCryptographicHash before it's called done.
    // exactDest: use devPath verbatim instead of treating a dot-less last
    // segment as a folder to drop the source's filename into.
    void uploadToFlipper(const QString &hostPath, const QString &devPath, bool exactDest = false);
    void downloadFromFlipper(const QString &devPath, const QString &hostPath);
    void finishXfer(bool ok, const QString &message);   // print (or chain to a batch)

    // Generic raw command on the interactive port: write it, buffer the reply
    // until the prompt returns, hand the raw text to the continuation. This is
    // what "storage tree" / "storage list" / "storage md5" / "storage mkdir"
    // run through outside of a file transfer.
    void sendRaw(const QString &cmd, std::function<void(const QString &)> onDone);

    // Recursive / batch ops built on top of the above: cp -r (both directions),
    // rm -r, wildcard cp/rm, and find -- all driven off "storage tree" or
    // "storage list", walking the result with the single-file primitives above.
    void ensureDeviceDir(const QString &path, std::function<void()> done);         // mkdir -p
    void startCopyUpTree(const QString &hostRoot, const QString &devRoot);         // cp -r host -> device
    void startCopyDownTree(const QString &devRoot, const QString &hostRoot);       // cp -r device -> host
    void removeTreeCore(const QString &path, std::function<void(bool)> done);      // no printing; used by both below
    void startRemoveTree(const QString &path);                                     // rm -r, single target
    void runRemoveQueue(const QStringList &targets);                              // rm of a wildcard match set
    void expandDeviceGlob(const QString &pattern, std::function<void(const QStringList &)> done);
    void runCopyQueue(const QStringList &devMatches, const QString &dst, bool dstHost); // cp of a wildcard match set
    void startFind(const QString &root, const QString &pattern);                   // find
    void startEdit(const QString &path);                                           // edit

    // Text utilities. The Flipper's firmware has no grep/head/tail/wc -- and no
    // room to grow one: it is an STM32WB55 with 256 KB of RAM. So the file is
    // read once with "storage read" and the filtering happens on this computer,
    // which is also why they are all built on the one helper below.
    void readDeviceText(const QString &path, std::function<void(bool, const QString &)> done);
    void startGrep(const QString &pattern, const QString &path);
    void startHeadTail(const QString &path, int n, bool head);
    void startWc(const QString &path);
    void startDu(const QString &path);                                             // du
    // Writing: both go out as a normal upload, so they inherit its MD5 check.
    void writeTextToDevice(const QString &text, const QString &devPath, bool append);
    void startWget(const QString &url, const QString &devPath, bool exactDest);    // wget

    ApplicationBackend *m_appBackend = nullptr;
    QSerialPort *m_port = nullptr;
    QString m_output;
    QString m_status;
    bool m_open = false;
    bool m_active = false;
    bool m_verbose = true;   // log everything by default
    bool m_colored = true;   // ls --color style output
    bool m_quiet = false;    // suppresses the log for Tab's own lookup
    // Whether the device has said anything since the port was opened. The
    // firmware announces itself on connect; this stops us asking for a second
    // prompt it already sent.
    bool m_sawDeviceBytes = false;
    // Host-side downloads for "wget".
    QNetworkAccessManager m_dl;
    QStringList m_history;   // commands typed this session, for "history"

    // Shell state: "cd" keeps a current folder that relative paths resolve against.
    QString prompt() const;                      // "Name@qflipper ~/nfc % "
    QString m_devName;                           // the Flipper's own name
    QString m_cwd = QStringLiteral("/ext");
    QString m_cdPrev = QStringLiteral("/ext");   // for "cd -"
    QString m_cdPending;                         // folder awaiting confirmation
    QByteArray m_cdRaw;

    // Buffered reply reformatting (help listing / directory listing).
    enum class Capture { None, Help, Listing };
    Capture m_capture = Capture::None;
    QString m_captureBuf;
    QTimer *m_captureFlush = nullptr;

    // An ANSI escape sequence cut in half by a read boundary, held back until
    // the rest of it arrives so it can be stripped as one piece.
    QString m_escTail;

    // Echo of a translated command still to be swallowed ("storage list /ext"
    // when the user typed "ls").
    QString m_echoPending;

    // What the user actually typed this turn. The verbose log skips it: the
    // panel already printed it, and the firmware echoes it back too.
    QString m_lastTyped;

    // Stops a firmware error split across serial chunks from logging twice.
    QString m_lastLoggedError;

    // Coalesces outputChanged so streaming commands can't relayout per chunk.
    QTimer *m_outputTick = nullptr;

    // Fires when the device has said nothing for long enough that whatever we
    // were waiting on is never coming.
    QTimer *m_opGuard = nullptr;

    // File transfer state. "Raw" is the generic one-shot-command state used by
    // sendRaw() (md5 checks, tree/list scans, mkdir, remove) -- anything that
    // isn't a payload transfer but still needs the reply buffered to the prompt.
    enum class Xfer { None, UploadReady, Download, Raw };
    Xfer m_xfer = Xfer::None;
    QByteArray m_xferPayload;
    QByteArray m_xferRaw;
    QString m_xferLabel;
    QString m_xferHostDst;
    QString m_xferDevPath;               // remote path of the in-flight transfer (for md5 verify)
    qint64 m_xferSize = -1;
    std::function<void(const QString &)> m_rawCb;   // continuation for sendRaw()
    std::function<void(bool)> m_xferChain;          // set by a batch op: called instead of prompting

    // One-shot (assistant) run state -- isolated from the interactive panel.
    QSerialPort *m_runPort = nullptr;
    QString m_runBuf;
    bool m_runBusy = false;
    QTimer *m_runIdle = nullptr;
    QTimer *m_runGuard = nullptr;
    std::function<void(bool, QString)> m_runDone;
};