import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import Theme 1.0
import QFlipper 1.0

Rectangle {
    id: root

    // ---- cross-message text selection -----------------------------------
    // Each message is its own TextEdit, and selectByMouse only ever works
    // inside one of them -- that is why dragging could never get past a message
    // boundary. So the drag is driven from here instead: the anchor and the
    // head are (message index, character offset) pairs, and applySelection()
    // pushes the resulting range down onto whichever delegates currently exist.
    property int selAnchorIdx: -1
    property int selAnchorPos: 0
    property int selHeadIdx: -1
    property int selHeadPos: 0

    function bodyAt(i) {
        var it = listView.itemAtIndex(i);
        return (it && it.bodyEdit) ? it.bodyEdit : null;
    }
    // Normalised low/high ends, so dragging upwards behaves like dragging down.
    function selRange() {
        var a = root.selAnchorIdx, ap = root.selAnchorPos;
        var b = root.selHeadIdx,   bp = root.selHeadPos;
        if (a < 0 || b < 0) { return null; }
        if (b < a || (b === a && bp < ap)) { return { lo: b, loPos: bp, hi: a, hiPos: ap }; }
        return { lo: a, loPos: ap, hi: b, hiPos: bp };
    }
    function applySelection() {
        var r = root.selRange();
        for (var i = 0; i < chatModel.count; i++) {
            var e = root.bodyAt(i);
            if (!e) { continue; }                       // scrolled out and recycled
            if (!r || i < r.lo || i > r.hi)   { e.deselect(); }
            else if (i === r.lo && i === r.hi) { e.select(r.loPos, r.hiPos); }
            else if (i === r.lo)               { e.select(r.loPos, e.length); }
            else if (i === r.hi)               { e.select(0, r.hiPos); }
            else                               { e.selectAll(); }
        }
    }
    // ListView.spacing leaves a few pixels between delegates where indexAt()
    // reports -1. Dragging through that gap used to jump the selection to the
    // end of the conversation, so probe outwards before giving up.
    function msgIndexAt(cx, cy) {
        var idx = listView.indexAt(cx, cy);
        if (idx >= 0) { return idx; }
        for (var d = 2; d <= 16; d += 2) {
            idx = listView.indexAt(cx, cy - d);
            if (idx >= 0) { return idx; }
            idx = listView.indexAt(cx, cy + d);
            if (idx >= 0) { return idx; }
        }
        // Genuinely past an end of the content.
        if (cy < 0) { return 0; }
        if (cy > listView.contentHeight) { return chatModel.count - 1; }
        return -1;
    }

    // The role label highlights only when the selection actually reaches the
    // start of that message -- a drag beginning mid-sentence should not light
    // up the speaker's name above it.
    function indexInSelection(i) {
        var r = root.selRange();
        if (r === null || i < r.lo || i > r.hi) { return false; }
        return (i > r.lo) || (r.loPos === 0);
    }
    function clearSelection() {
        root.selAnchorIdx = -1;
        root.selHeadIdx = -1;
        root.applySelection();
    }
    function selectAllMessages() {
        if (chatModel.count === 0) { return; }
        root.selAnchorIdx = 0;
        root.selAnchorPos = 0;
        root.selHeadIdx = chatModel.count - 1;
        var last = root.bodyAt(chatModel.count - 1);
        root.selHeadPos = last ? last.length : chatModel.get(chatModel.count - 1).text.length;
        root.applySelection();
    }
    // Build the text for the clipboard. Visible delegates give the rendered
    // text (markdown already resolved); ones that were recycled fall back to
    // the raw message, which for a code block is arguably the more useful form.
    function selectedText() {
        var r = root.selRange();
        if (!r) { return ""; }
        var parts = [];
        for (var i = r.lo; i <= r.hi; i++) {
            var e = root.bodyAt(i);
            var raw = chatModel.get(i).text;
            var who = (chatModel.get(i).role === "lotei" ? root.aiName : "you") + ": ";
            if (e) {
                var a = (i === r.lo) ? r.loPos : 0;
                var b = (i === r.hi) ? r.hiPos : e.length;
                parts.push(who + e.getText(a, b));
            } else {
                parts.push(who + raw);
            }
        }
        return parts.join("\n");
    }
    function copySelection() {
        var t = root.selectedText();
        if (t.length === 0) {
            var all = [];
            for (var i = 0; i < chatModel.count; i++) {
                all.push((chatModel.get(i).role === "lotei" ? root.aiName : "you") + ": " + chatModel.get(i).text);
            }
            t = all.join("\n\n");
        }
        Cli.copyToClipboard(t);
    }

    // ---- view state / geometry ------------------------------------------
    // "normal" = docked in the corner, "max" = big read view, "min" = collapsed
    property string viewState: "normal"

    // memory.txt may have been edited in the file manager while this panel was
    // hidden. Re-read the card's copy whenever the chat comes back into view,
    // so the conversation starts from what the file actually says.
    onVisibleChanged: if (visible) { Lotei.reloadMemory(); }

    readonly property int dockX: 24
    readonly property int dockY: 272
    readonly property int dockW: 384
    readonly property int dockH: 146
    readonly property int minH: 44

    property bool healthChecked: false
    property int streamIdx: -1   // index of the message currently being streamed
    property bool showScreen: false   // mirror the live Flipper screen in-panel
    // The assistant takes the connected Flipper's name as its own (fallback LOTEI).
    readonly property string aiName: (Backend.deviceState && Backend.deviceState.info && Backend.deviceState.info.name && Backend.deviceState.info.name.length > 0)
                                     ? Backend.deviceState.info.name
                                     : (Lotei.manualName.length > 0 ? Lotei.manualName : "LOTEI")

    x: viewState === "max" ? 14 : dockX
    y: viewState === "max" ? 78 : dockY
    width:  viewState === "max" ? ((parent ? parent.width  : 804) - 28) : dockW
    height: viewState === "max" ? ((parent ? parent.height : 394) - 92)
            : (viewState === "min" ? minH : dockH)

    Behavior on width  { NumberAnimation { duration: 140; easing.type: Easing.OutCubic } }
    Behavior on height { NumberAnimation { duration: 140; easing.type: Easing.OutCubic } }
    Behavior on y      { NumberAnimation { duration: 140; easing.type: Easing.OutCubic } }

    radius: 9
    color: "#0b0410"
    border.width: 2
    border.color: Theme.color.lightorange2

    // ---- helpers --------------------------------------------------------
    function deviceContext() {
        var ds = Backend.deviceState;
        if(!ds || !ds.info) {
            return "No Flipper is currently connected.";
        }
        var i = ds.info;
        var lines = [];
        lines.push("Name: " + i.name);
        if(ds.isRecoveryMode) {
            lines.push("Mode: Update & Recovery (DFU)");
        } else {
            var fw = i.firmware.version;
            if(i.firmware.commit && i.firmware.commit.length) { fw += " (commit " + i.firmware.commit + ")"; }
            lines.push("Firmware: " + fw);
            lines.push("SD card: " + (i.storage.isExternalPresent ? (i.storage.externalFree + "% free") : "not present"));
            lines.push("Databases: " + (i.storage.isAssetsInstalled ? "installed" : "missing"));
            lines.push("Radio firmware: " + (i.radioVersion.length ? i.radioVersion : "corrupted"));
        }
        lines.push("Hardware: " + i.hardware.version + "." + i.hardware.target + i.hardware.body + i.hardware.connect);
        return lines.join("\n");
    }

    function appendMessage(role, text) {
        chatModel.append({ "role": role, "text": text });
        listView.positionViewAtEnd();
    }

    function clearChat() {
        chatModel.clear();
        Lotei.clearHistory();
    }

    function sendCurrent() {
        var t = input.text.trim();
        if(t.length === 0 || Lotei.thinking) {
            return;
        }
        appendMessage("you", t);
        Lotei.send(t, deviceContext());
        input.text = "";
    }

    // Auto health-check once per connection.
    // Auto health-check on connect is disabled: the chat opens with one fixed
    // greeting and stays quiet until the user actually says something.
    function maybeHealthCheck() { }

    ListModel { id: chatModel }

    Connections {
        target: Lotei
        // live typing: grow one bubble as tokens arrive
        function onPartialReceived(text) {
            if(root.streamIdx < 0) {
                chatModel.append({ "role": "lotei", "text": text });
                root.streamIdx = chatModel.count - 1;
            } else {
                chatModel.setProperty(root.streamIdx, "text", text);
            }
            listView.positionViewAtEnd();
        }
        function onReplyReceived(text) {
            if(root.streamIdx >= 0) {
                chatModel.setProperty(root.streamIdx, "text", text);  // finalize
                root.streamIdx = -1;
            } else {
                root.appendMessage("lotei", text);
            }
            listView.positionViewAtEnd();
        }
        function onErrorOccurred(text) {
            if(root.streamIdx >= 0) {
                chatModel.setProperty(root.streamIdx, "text", text);
                root.streamIdx = -1;
            } else {
                root.appendMessage("lotei", text);
            }
        }
        // Feedback from the manual save panel (model-free save straight to SD).
        function onScriptSaved(path) {
            root.appendMessage("lotei", "✅ Salvo em " + path);
            listView.positionViewAtEnd();
        }
        function onScriptSaveError(message) {
            root.appendMessage("lotei", "⚠️ Couldn't save: " + message);
            listView.positionViewAtEnd();
        }
    }

    // Robust connect detection: poll device state; fires the health check once
    // whenever a Flipper becomes available (startup, connect, or reconnect).
    Timer { interval: 2000; repeat: true; running: true; onTriggered: root.maybeHealthCheck() }

    // One fixed greeting on startup; everything after that is the model talking.
    Component.onCompleted: {
        // The greeting goes first on purpose. It used to sit after a
        // reloadMemory() call, and anything that threw in there took the
        // greeting with it -- the chat opened silent.
        appendMessage("lotei", "Hey, how can I help you today?");
        Lotei.reloadMemory();
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 9
        spacing: 5

        // ---- header: name + status + window buttons ----
        RowLayout {
            Layout.fillWidth: true
            spacing: 5

            // ---- cyberpunk model badge (status dot + glitch text + cursor) ----
            Item {
                id: modelBadge
                Layout.alignment: Qt.AlignVCenter
                implicitWidth: dot.width + 8 + modelFg.implicitWidth + 10
                implicitHeight: Math.max(modelFg.implicitHeight, 8) + 2

                QtObject { id: glitch; property real off: 0 }
                Timer {
                    interval: 110; running: true; repeat: true
                    onTriggered: glitch.off = (Math.random() < 0.16) ? (Math.random() * 2.4) : 0
                }

                // pulsing status dot: green idle, magenta while thinking
                Rectangle {
                    id: dot
                    width: 6; height: 6; radius: 3
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    color: Lotei.thinking ? "#ff2fb0" : "#39ff14"
                    SequentialAnimation on opacity {
                        loops: Animation.Infinite; running: true
                        NumberAnimation { from: 1.0; to: 0.2; duration: 850; easing.type: Easing.InOutSine }
                        NumberAnimation { from: 0.2; to: 1.0; duration: 850; easing.type: Easing.InOutSine }
                    }
                }

                // chromatic-aberration layers (magenta + cyan behind, bright on top)
                Text {
                    text: Lotei.modelName.length > 0 ? Lotei.modelName : "no model"
                    color: "#ff2fb0"; opacity: 0.7
                    anchors.left: dot.right; anchors.leftMargin: 8 + glitch.off
                    anchors.verticalCenter: parent.verticalCenter
                    font.family: "Share Tech Mono"; font.pixelSize: 12; font.bold: true
                }
                Text {
                    text: Lotei.modelName.length > 0 ? Lotei.modelName : "no model"
                    color: "#00e5ff"; opacity: 0.7
                    anchors.left: dot.right; anchors.leftMargin: 8 - glitch.off
                    anchors.verticalCenter: parent.verticalCenter
                    font.family: "Share Tech Mono"; font.pixelSize: 12; font.bold: true
                }
                Text {
                    id: modelFg
                    text: Lotei.modelName.length > 0 ? Lotei.modelName : "no model"
                    color: "#eaffea"
                    anchors.left: dot.right; anchors.leftMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                    font.family: "Share Tech Mono"; font.pixelSize: 12; font.bold: true
                }
            }
            Item { Layout.fillWidth: true }

            // Model manager (gear icon)
            Rectangle {
                Layout.preferredWidth: 26
                Layout.preferredHeight: 20
                Layout.alignment: Qt.AlignVCenter
                radius: 3
                color: gearMouse.containsMouse ? Qt.rgba(1, 1, 1, 0.14) : "transparent"

                // Drawn rather than typed. "\u2699" has no glyph in Share Tech
                // Mono (nor in the app's other faces), so Qt was substituting a
                // system symbol font for that one character -- which is why the
                // gear came out smaller than its neighbours and sitting on a
                // different baseline. A Canvas has no font to fall back to: it
                // scales cleanly and takes the theme colour directly.
                Canvas {
                    id: gearIcon
                    width: 15; height: 15
                    anchors.centerIn: parent
                    property color tint: gearMouse.containsMouse ? "#eaffea" : Theme.color.lightorange2
                    onTintChanged: requestPaint()
                    onPaint: {
                        var ctx = getContext("2d");
                        ctx.reset();
                        ctx.translate(width / 2, height / 2);
                        ctx.fillStyle = gearIcon.tint;

                        var teeth = 8;
                        var rOuter = width * 0.50;
                        var rBody  = width * 0.34;
                        var toothW = width * 0.20;

                        for (var i = 0; i < teeth; i++) {
                            ctx.save();
                            ctx.rotate(i * Math.PI / teeth * 2);
                            ctx.fillRect(-toothW / 2, -rOuter, toothW, rOuter - rBody + 1);
                            ctx.restore();
                        }
                        ctx.beginPath();
                        ctx.arc(0, 0, rBody, 0, Math.PI * 2);
                        ctx.fill();

                        // Punch the hub out so it reads as a gear, not a blob.
                        ctx.globalCompositeOperation = "destination-out";
                        ctx.beginPath();
                        ctx.arc(0, 0, width * 0.14, 0, Math.PI * 2);
                        ctx.fill();
                    }
                }
                MouseArea {
                    id: gearMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: modelManager.openManager()
                }
                ToolTip { text: "Model manager"; visible: gearMouse.containsMouse; delay: 400 }
            }

            // Clear conversation
            Rectangle {
                Layout.preferredWidth: 42
                Layout.preferredHeight: 20
                Layout.alignment: Qt.AlignVCenter
                radius: 3
                color: clearMouse.containsMouse ? Qt.rgba(1, 1, 1, 0.14) : "transparent"

                Text {
                    anchors.centerIn: parent
                    text: "clear"
                    color: Theme.color.lightorange2
                    font.family: "Share Tech Mono"; font.pixelSize: 11
                }
                MouseArea {
                    id: clearMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.clearChat()
                }
                ToolTip { text: "Clear chat"; visible: clearMouse.containsMouse; delay: 400 }
            }

            // Minimize / restore (collapse to title bar)
            Rectangle {
                Layout.preferredWidth: 26
                Layout.preferredHeight: 20
                Layout.alignment: Qt.AlignVCenter
                radius: 3
                color: minMouse.containsMouse ? Qt.rgba(1, 1, 1, 0.14) : "transparent"

                Rectangle {
                    width: 11; height: 2; radius: 1
                    color: Theme.color.lightorange2
                    anchors.centerIn: parent
                }
                MouseArea {
                    id: minMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.viewState = (root.viewState === "min") ? "normal" : "min"
                }
            }

            // Maximize / restore (big read view)
            Rectangle {
                Layout.preferredWidth: 26
                Layout.preferredHeight: 20
                Layout.alignment: Qt.AlignVCenter
                radius: 3
                color: maxMouse.containsMouse ? Qt.rgba(1, 1, 1, 0.14) : "transparent"

                Rectangle {
                    width: root.viewState === "max" ? 8 : 11
                    height: width
                    color: "transparent"
                    border.width: 2
                    border.color: Theme.color.lightorange2
                    anchors.centerIn: parent
                    Behavior on width { NumberAnimation { duration: 120 } }
                }
                MouseArea {
                    id: maxMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.viewState = (root.viewState === "max") ? "normal" : "max"
                }
            }
        }

        // ---- live Flipper screen mirror (Phase A: tap qFlipper's stream) ----
        Rectangle {
            visible: root.showScreen && Backend.screenStreamer && Backend.screenStreamer.isEnabled
            Layout.fillWidth: true
            Layout.preferredHeight: visible ? (root.viewState === "max" ? 180 : 72) : 0
            color: "#0b0410"
            radius: 5
            border.width: 1
            border.color: Theme.color.mediumorange2
            ScreenCanvas {
                anchors.fill: parent
                anchors.margins: 4
                frame: Backend.screenStreamer.screenFrame
                foregroundColor: Theme.color.lightorange2
                backgroundColor: "#0b0410"
            }
        }

        // ---- message log (hidden when minimized) ----
        ListView {
            id: listView
            visible: root.viewState !== "min"
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: chatModel
            spacing: 6
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: ScrollBar { }

            // Shown while the model is generating -- the only real wait in the UI.
            footer: Item {
                width: ListView.view ? ListView.view.width : 0
                height: Lotei.thinking ? 20 : 0
                visible: Lotei.thinking
                Row {
                    spacing: 0
                    Text {
                        text: root.aiName + " is thinking"
                        color: Theme.color.mediumorange4
                        font.family: "Share Tech Mono"
                        font.pixelSize: 12
                    }
                    Text {
                        id: thinkDots
                        color: Theme.color.mediumorange4
                        font.family: "Share Tech Mono"
                        font.pixelSize: 12
                        property int step: 0
                        text: ["   ", ".  ", ".. ", "..."][step]
                    }
                    Timer {
                        running: Lotei.thinking
                        interval: 380; repeat: true
                        onTriggered: thinkDots.step = (thinkDots.step + 1) % 4
                    }
                }
            }

            delegate: Column {
                id: msgCol
                width: ListView.view.width
                spacing: 1

                // Lets root.applySelection() reach this message's text.
                property alias bodyEdit: bodyText

                TextEdit { id: clip; visible: false }  // hidden clipboard helper

                Row {
                    spacing: 8
                    Rectangle {
                        width: roleLabel.implicitWidth + 4
                        height: roleLabel.implicitHeight
                        color: root.indexInSelection(index) ? roleLabel.selectionColor : "transparent"
                        Text {
                            id: roleLabel
                            x: 2
                            readonly property color selectionColor: "#3b5bdb"
                            text: model.role === "lotei" ? root.aiName : "you"
                            color: model.role === "lotei" ? Theme.color.lightorange2 : Theme.color.mediumorange1
                            font.family: "Share Tech Mono"
                            font.pixelSize: 11
                        }
                    }
                }
                TextEdit {
                    id: bodyText
                    width: parent.width
                    text: model.text
                    readOnly: true
                    // Off on purpose: the drag below spans messages, and the
                    // built-in one would fight it inside this single message.
                    selectByMouse: false
                    persistentSelection: true
                    wrapMode: TextEdit.Wrap
                    textFormat: model.role === "lotei" ? TextEdit.MarkdownText : TextEdit.PlainText
                    color: "white"
                    font.family: "Share Tech Mono"
                    font.pixelSize: 13
                    onLinkActivated: function(link) { Qt.openUrlExternally(link) }

                    // Covers the message text and nothing else, so the save
                    // panel below keeps its own mouse handling untouched.
                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.LeftButton
                        cursorShape: Qt.IBeamCursor
                        // The list is a Flickable: without this it steals the
                        // drag halfway through and treats it as a scroll, which
                        // is what made the selection break up mid-gesture.
                        preventStealing: true

                        onPressed: function(mouse) {
                            root.selAnchorIdx = index;
                            root.selAnchorPos = bodyText.positionAt(mouse.x, mouse.y);
                            root.selHeadIdx = index;
                            root.selHeadPos = root.selAnchorPos;
                            root.applySelection();
                        }
                        onPositionChanged: function(mouse) {
                            if (!pressed) { return; }
                            // Which message is under the cursor now? Map into the
                            // list's content item so the answer stays right while
                            // the view scrolls.
                            var pt = mapToItem(listView.contentItem, mouse.x, mouse.y);
                            var idx = root.msgIndexAt(pt.x, pt.y);
                            if (idx < 0) { return; }
                            var e = root.bodyAt(idx);
                            if (!e) { return; }
                            var local = mapToItem(e, mouse.x, mouse.y);
                            root.selHeadIdx = idx;
                            root.selHeadPos = e.positionAt(local.x, local.y);
                            root.applySelection();
                        }
                        onDoubleClicked: function(mouse) {
                            root.selAnchorIdx = index;
                            root.selHeadIdx = index;
                            bodyText.selectWord();
                            root.selAnchorPos = bodyText.selectionStart;
                            root.selHeadPos = bodyText.selectionEnd;
                        }
                    }
                }

                // ---- Manual save panel (model-free) --------------------------
                // Shown for LOTEI replies that contain a code block. YOU pick the
                // folder + name and hit save; the app writes straight to the SD.
                // The AI drafts, you save -- no fumbled tool calls.
                Rectangle {
                    visible: model.role === "lotei" && model.text.indexOf("```") >= 0
                    width: parent.width
                    implicitHeight: panelCol.implicitHeight + 16
                    color: "#0A0010"
                    border.color: Theme.color.mediumorange2
                    border.width: 1
                    radius: 3

                    Column {
                        id: panelCol
                        x: 8; y: 8
                        width: parent.width - 16
                        spacing: 6

                        // top bar: folder (left) + buttons (right)
                        Item {
                            width: parent.width
                            height: 26

                            Row {
                                anchors.left: parent.left
                                anchors.verticalCenter: parent.verticalCenter
                                spacing: 6
                                ComboBox {
                                    id: folderBox
                                    width: 130
                                    height: 24
                                    model: ["badusb", "subghz", "nfc", "infrared", "lfrfid", "ibutton", "ext"]
                                }
                                TextField {
                                    id: nameField
                                    width: 150
                                    height: 24
                                    text: "script.txt"
                                    font.family: "Share Tech Mono"
                                    font.pixelSize: 12
                                }
                            }

                            Row {
                                anchors.right: parent.right
                                anchors.verticalCenter: parent.verticalCenter
                                spacing: 12
                                Text {
                                    text: "⧉ copy"
                                    color: copyBtn.containsMouse ? Theme.color.lightorange2 : Theme.color.mediumorange1
                                    font.family: "Share Tech Mono"; font.pixelSize: 12
                                    MouseArea {
                                        id: copyBtn; anchors.fill: parent; anchors.margins: -4
                                        hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                                        onClicked: { clip.text = scriptArea.text; clip.selectAll(); clip.copy(); }
                                    }
                                }
                                Text {
                                    text: "▼ save"
                                    color: saveBtn.containsMouse ? Theme.color.lightgreen : Theme.color.lightorange2
                                    font.family: "Share Tech Mono"; font.pixelSize: 12; font.bold: true
                                    MouseArea {
                                        id: saveBtn; anchors.fill: parent; anchors.margins: -4
                                        hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                                        onClicked: Lotei.saveScriptToFlipper(folderBox.currentText, nameField.text, scriptArea.text)
                                    }
                                }
                            }
                        }

                        // the script itself, editable, terminal-styled
                        Rectangle {
                            width: parent.width
                            implicitHeight: Math.min(scriptArea.implicitHeight + 12, 260)
                            color: "#000000"
                            border.color: Theme.color.mediumorange1
                            border.width: 1
                            radius: 2
                            Flickable {
                                anchors.fill: parent
                                anchors.margins: 6
                                contentHeight: scriptArea.implicitHeight
                                clip: true
                                TextArea {
                                    id: scriptArea
                                    width: parent.width
                                    text: Lotei.extractScript(model.text)
                                    wrapMode: TextArea.NoWrap
                                    color: Theme.color.lightgreen
                                    selectByMouse: true
                                    font.family: "Share Tech Mono"
                                    font.pixelSize: 12
                                    background: null
                                }
                            }
                        }
                    }
                }
            }
        }

        // ---- input row (hidden when minimized) ----
        RowLayout {
            visible: root.viewState !== "min"
            Layout.fillWidth: true
            spacing: 6

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 30
                radius: 6
                color: "black"
                border.width: 1
                border.color: Theme.color.mediumorange2

                TextInput {
                    id: input
                    anchors.fill: parent
                    anchors.leftMargin: 8
                    anchors.rightMargin: 8
                    verticalAlignment: TextInput.AlignVCenter
                    clip: true
                    color: "white"
                    selectionColor: Theme.color.lightorange2
                    font.family: "Share Tech Mono"
                    font.pixelSize: 13
                    enabled: !Lotei.thinking
                    onAccepted: root.sendCurrent()

                    Text {
                        anchors.fill: parent
                        verticalAlignment: Text.AlignVCenter
                        visible: input.text.length === 0 && !input.activeFocus
                        text: "Talk to " + root.aiName + "…"
                        color: Theme.color.mediumorange1
                        font: input.font
                    }
                }
            }

            Button {
                text: "Send"
                enabled: !Lotei.thinking && input.text.length > 0
                onClicked: root.sendCurrent()
            }
        }
    }

    // ---- Model manager panel (gear icon) ----------------------------------
    // Not a Popup any more. A modal Popup brought a style-drawn dim with it and
    // read as something layered on top of the app; the CLI panel next door is
    // just an Item filling the same content box, and that is what the rest of
    // this UI looks like. So this is the same shape: parented to the overlay
    // LoteiChat itself sits in (which fills mainContent, exactly the area the
    // CLI covers), toggled by `open`, no dim behind it.
    Item {
        id: modelManager
        parent: root.parent ? root.parent : root
        anchors.fill: parent
        z: 200

        property bool open: false
        visible: opacity > 0
        enabled: visible
        // Needed for Escape to reach Keys.onEscapePressed below; dropped again
        // on close so the chat input can take the keyboard back.
        focus: open
        opacity: open ? 1 : 0
        Behavior on opacity { NumberAnimation { duration: 120; easing.type: Easing.InOutQuad } }

        // Model list is only pulled fresh when the panel opens, not polled --
        // it changes only in response to actions this same panel drives.
        function openManager() {
            catalogModel.refresh();
            Lotei.detectOllama();
            modelManager.open = true;
        }
        function close() { modelManager.open = false; }

        // Swallows anything aimed at the screen behind, the way cliOverlay does.
        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.AllButtons
            onWheel: function(wheel) { wheel.accepted = true }
        }

        Keys.onEscapePressed: modelManager.close()


        // Backing list model for the catalog. A plain ListModel refreshed from
        // Lotei.modelCatalog() -- simplest option here since the catalog is
        // small (a handful of curated entries), not something worth a C++
        // QAbstractListModel for.
        ListModel { id: catalogModel
            function refresh() {
                // Update rows in place rather than clear()+append(): clearing
                // drops the list to zero items for a frame, which snaps
                // catalogView's scroll position back to the top on every
                // refresh -- annoying if you'd scrolled down and an install
                // finishes, or Ollama comes online, mid-scroll.
                var rows = Lotei.modelCatalog();
                for (var i = 0; i < rows.length; i++) {
                    if (i < count) { set(i, rows[i]); } else { append(rows[i]); }
                }
                while (count > rows.length) { remove(count - 1); }
            }
        }

        // modelOpKind/Name/Status/Progress all share one NOTIFY (modelOpChanged),
        // which fires on every chunk of `ollama pull` output -- many times a
        // second while a download is running. The busy row's progress bar and
        // status text already bind straight to Lotei.modelOpProgress/modelOpStatus
        // below, so they don't need a list rebuild to update. Only rebuild when
        // a row actually needs to flip into/out of "busy" -- i.e. when the op
        // kind itself changes -- otherwise every tick was tearing down and
        // recreating all the delegates, which is what caused the flicker/glitch
        // during a pull.
        QtObject {
            id: modelOpTracker
            property string kind: ""
        }

        Connections {
            target: Lotei
            function onModelOpChanged() {
                if (Lotei.modelOpKind !== modelOpTracker.kind) {
                    modelOpTracker.kind = Lotei.modelOpKind;
                    catalogModel.refresh();
                }
            }
            function onModelInstallFinished()     { catalogModel.refresh(); }
            function onModelUninstallFinished()   { catalogModel.refresh(); }
            function onOllamaInstallFinished()    { catalogModel.refresh(); }
            function onOllamaInstalledChanged()   { catalogModel.refresh(); }
            function onModelChanged()             { if (modelManager.visible) { catalogModel.refresh(); } }
        }

        // Same panel chrome as the CLI: filled card on the content box, one
        // border, title in the header row, close on the right.
        Rectangle {
            anchors.fill: parent
            color: "#0b0410"
            radius: 8
            border.width: 2
            border.color: Theme.color.mediumorange2
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 18
            spacing: 10

            RowLayout {
                Layout.fillWidth: true
                Text {
                    text: "MODEL MANAGER"
                    color: Theme.color.lightorange2
                    font.family: "Share Tech Mono"; font.pixelSize: 20; font.bold: true
                    Layout.fillWidth: true
                }
                Text {
                    text: "\u2715"   // ✕
                    color: closeMouse.containsMouse ? Theme.color.lightorange2 : Theme.color.mediumorange4
                    font.family: "Share Tech Mono"; font.pixelSize: 18
                    MouseArea {
                        id: closeMouse
                        anchors.fill: parent
                        anchors.margins: -6
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: modelManager.close()
                    }
                }
            }

            // ---- "Ollama itself isn't installed" banner ----
            Rectangle {
                visible: !Lotei.ollamaInstalled
                Layout.fillWidth: true
                Layout.preferredHeight: ollamaBannerCol.implicitHeight + 16
                radius: 5
                color: "#2a0a0a"
                border.width: 1
                border.color: "#ff5a5a"

                ColumnLayout {
                    id: ollamaBannerCol
                    x: 10; y: 8
                    width: parent.width - 20
                    spacing: 6

                    Text {
                        text: "Ollama isn't installed on this machine. LOTEI needs it to run any model."
                        color: "#ffb3b3"
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        font.family: "Share Tech Mono"; font.pixelSize: 11
                    }
                    RowLayout {
                        spacing: 8
                        Button {
                            text: (Lotei.modelOpKind === "ollama") ? "Installing…" : "Install Ollama"
                            enabled: Lotei.modelOpKind !== "ollama"
                            onClicked: Lotei.installOllama()
                        }
                        Text {
                            visible: Lotei.modelOpKind === "ollama"
                            text: Lotei.modelOpStatus
                            color: "#ffb3b3"
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                            font.family: "Share Tech Mono"; font.pixelSize: 10
                        }
                        // Manual re-probe, for the macOS path where the install
                        // happens outside the app (download page / Ollama.app).
                        Text {
                            visible: Lotei.modelOpKind !== "ollama"
                            text: "recheck"
                            color: recheckMouse.containsMouse ? "#ff6a6a" : "#ffb3b3"
                            font.family: "Share Tech Mono"; font.pixelSize: 10; font.bold: true
                            MouseArea {
                                id: recheckMouse
                                anchors.fill: parent
                                anchors.margins: -4
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: Lotei.detectOllama()
                            }
                        }
                        Text {
                            visible: Lotei.modelOpKind === "ollama"
                            text: "cancel"
                            color: cancelOllamaMouse.containsMouse ? "#ff6a6a" : "#ffb3b3"
                            font.family: "Share Tech Mono"; font.pixelSize: 10; font.bold: true
                            MouseArea {
                                id: cancelOllamaMouse
                                anchors.fill: parent
                                anchors.margins: -4
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: Lotei.cancelModelOp()
                            }
                        }
                    }
                }
            }

            Text {
                text: "Curated models known to work well with LOTEI's tools:"
                color: Theme.color.mediumorange1
                font.family: "Share Tech Mono"; font.pixelSize: 11
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }

            // ---- catalog list ----
            ListView {
                id: catalogView
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: catalogModel
                spacing: 6
                enabled: Lotei.ollamaInstalled

                delegate: Rectangle {
                    width: catalogView.width
                    height: rowCol.implicitHeight + 16
                    radius: 5
                    color: model.active ? "#1a0d24" : "#120818"
                    border.width: 1
                    border.color: model.active ? Theme.color.lightorange2 : Theme.color.mediumorange2

                    ColumnLayout {
                        id: rowCol
                        x: 10; y: 8
                        width: parent.width - 20
                        spacing: 4

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 6
                            Text {
                                text: model.label + "  ·  " + model.size
                                color: "#eaffea"
                                font.family: "Share Tech Mono"; font.pixelSize: 12; font.bold: true
                                Layout.fillWidth: true
                            }
                            Text {
                                visible: model.active
                                text: "active"
                                color: "#39ff14"
                                font.family: "Share Tech Mono"; font.pixelSize: 10
                            }
                        }

                        Text {
                            text: model.blurb
                            color: Theme.color.mediumorange1
                            font.family: "Share Tech Mono"; font.pixelSize: 10
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }

                        // Busy row: progress bar + status line instead of the buttons.
                        ColumnLayout {
                            visible: model.busy
                            Layout.fillWidth: true
                            spacing: 4

                            // Deliberately NOT a ProgressBar. The DefaultAmber
                            // style's ProgressBar is built for the full-screen
                            // firmware update: its contentItem centres a 48px
                            // "HaxrCorp 4089" percentage label, and that label is
                            // `Math.round(control.value) + "%"` -- i.e. it assumes
                            // value runs 0..100, while ProgressBar's default range
                            // (and modelOpProgress) is 0..1. Two consequences, both
                            // visible in the screenshots: the control's implicit
                            // height collapses to zero, so the 48px label escapes
                            // and floats unclipped over the row as that pixelated
                            // blob; and Math.round(0.63) is 1, so it reads "0%"
                            // until the download passes halfway and then "1%" for
                            // the entire rest of it. That is the stuck 1%.
                            Item {
                                id: barTrack
                                Layout.fillWidth: true
                                Layout.preferredHeight: 6
                                clip: true

                                Rectangle {
                                    anchors.fill: parent
                                    radius: 3
                                    color: "transparent"
                                    border.width: 1
                                    border.color: Theme.color.mediumorange2
                                }

                                // Determinate fill.
                                Rectangle {
                                    visible: Lotei.modelOpProgress >= 0
                                    x: 1; y: 1
                                    height: parent.height - 2
                                    radius: 2
                                    color: Theme.color.lightorange2
                                    width: Math.max(0, (barTrack.width - 2) * Math.min(1, Lotei.modelOpProgress))
                                    Behavior on width { NumberAnimation { duration: 150 } }
                                }

                                // Indeterminate sweep, for the stages that report
                                // no percentage ("pulling manifest", "verifying").
                                Rectangle {
                                    id: barSweep
                                    visible: Lotei.modelOpProgress < 0
                                    y: 1
                                    height: parent.height - 2
                                    width: Math.max(12, barTrack.width * 0.25)
                                    radius: 2
                                    color: Theme.color.mediumorange1
                                    SequentialAnimation on x {
                                        running: barSweep.visible && modelManager.visible
                                        loops: Animation.Infinite
                                        NumberAnimation {
                                            from: 1; to: Math.max(1, barTrack.width - barSweep.width - 1)
                                            duration: 850; easing.type: Easing.InOutQuad
                                        }
                                        NumberAnimation {
                                            from: Math.max(1, barTrack.width - barSweep.width - 1); to: 1
                                            duration: 850; easing.type: Easing.InOutQuad
                                        }
                                    }
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 8
                                Text {
                                    text: Lotei.modelOpStatus
                                    color: Theme.color.mediumorange1
                                    font.family: "Share Tech Mono"; font.pixelSize: 9
                                    elide: Text.ElideRight
                                    maximumLineCount: 1
                                    Layout.fillWidth: true
                                }
                                Text {
                                    visible: Lotei.modelOpProgress >= 0
                                    text: Math.round(Lotei.modelOpProgress * 100) + "%"
                                    color: Theme.color.lightorange2
                                    font.family: "Share Tech Mono"; font.pixelSize: 9; font.bold: true
                                }
                                Text {
                                    text: "cancel"
                                    color: cancelOpMouse.containsMouse ? "#ff6a6a" : Theme.color.mediumorange1
                                    font.family: "Share Tech Mono"; font.pixelSize: 9; font.bold: true
                                    MouseArea {
                                        id: cancelOpMouse
                                        anchors.fill: parent
                                        anchors.margins: -4
                                        hoverEnabled: true
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: Lotei.cancelModelOp()
                                    }
                                }
                            }
                        }

                        // Idle row: status pill + action button.
                        RowLayout {
                            visible: !model.busy
                            Layout.fillWidth: true
                            spacing: 8

                            Rectangle {
                                Layout.preferredWidth: installedPill.implicitWidth + 12
                                Layout.preferredHeight: 18
                                radius: 3
                                color: model.installed ? "#0f3d1f" : "transparent"
                                border.width: model.installed ? 0 : 1
                                border.color: Theme.color.mediumorange2
                                Text {
                                    id: installedPill
                                    anchors.centerIn: parent
                                    text: model.installed ? "installed" : "not installed"
                                    color: model.installed ? "#39ff14" : Theme.color.mediumorange1
                                    font.family: "Share Tech Mono"; font.pixelSize: 10
                                }
                            }

                            Item { Layout.fillWidth: true }

                            Text {
                                visible: !model.installed
                                text: "install"
                                color: installMouse.containsMouse ? Theme.color.lightgreen : Theme.color.lightorange2
                                font.family: "Share Tech Mono"; font.pixelSize: 11; font.bold: true
                                MouseArea {
                                    id: installMouse
                                    anchors.fill: parent
                                    anchors.margins: -4
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    enabled: Lotei.modelOpKind === ""
                                    onClicked: Lotei.installModel(model.tag)
                                }
                            }
                            Rectangle {
                                visible: model.installed && !model.active
                                Layout.preferredWidth: equipLabel.implicitWidth + 14
                                Layout.preferredHeight: 18
                                radius: 3
                                color: equipMouse.containsMouse ? Theme.color.lightorange2 : "transparent"
                                border.width: 1
                                border.color: Theme.color.lightorange2
                                Text {
                                    id: equipLabel
                                    anchors.centerIn: parent
                                    text: "EQUIP"
                                    color: equipMouse.containsMouse ? "#0b0410" : Theme.color.lightorange2
                                    font.family: "Share Tech Mono"; font.pixelSize: 10; font.bold: true
                                }
                                MouseArea {
                                    id: equipMouse
                                    anchors.fill: parent
                                    anchors.margins: -4
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    enabled: Lotei.modelOpKind === ""
                                    onClicked: Lotei.setModel(model.tag)
                                }
                            }
                            Text {
                                visible: model.installed
                                text: "uninstall"
                                color: uninstallMouse.containsMouse ? "#ff6a6a" : Theme.color.mediumorange1
                                font.family: "Share Tech Mono"; font.pixelSize: 11
                                MouseArea {
                                    id: uninstallMouse
                                    anchors.fill: parent
                                    anchors.margins: -4
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    enabled: Lotei.modelOpKind === ""
                                    onClicked: Lotei.uninstallModel(model.tag)
                                }
                            }
                        }
                    }
                }
            }

            // Result line for the op that just finished. Until now the only
            // places modelOpStatus was shown were bound to an op being *in
            // flight* (modelOpKind === "ollama", or a row's busy state), so a
            // fast failure -- start, fail, finish, kind back to "" -- left no
            // trace anywhere in the UI and read as "the button does nothing".
            Text {
                visible: Lotei.modelOpKind === "" && Lotei.modelOpStatus.length > 0
                text: Lotei.modelOpStatus
                color: Theme.color.mediumorange1
                font.family: "Share Tech Mono"; font.pixelSize: 10
                wrapMode: Text.WordWrap
                maximumLineCount: 3
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
        }
    }
}
