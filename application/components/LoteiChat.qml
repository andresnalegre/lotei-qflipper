import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import Theme 1.0
import QFlipper 1.0

Rectangle {
    id: root

    // ---- view state / geometry ------------------------------------------
    // "normal" = docked in the corner, "max" = big read view, "min" = collapsed
    property string viewState: "normal"

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
            root.appendMessage("lotei", "⚠️ Não consegui salvar: " + message);
            listView.positionViewAtEnd();
        }
    }

    // Robust connect detection: poll device state; fires the health check once
    // whenever a Flipper becomes available (startup, connect, or reconnect).
    Timer { interval: 2000; repeat: true; running: true; onTriggered: root.maybeHealthCheck() }

    // One fixed greeting on startup; everything after that is the model talking.
    Component.onCompleted: {
        appendMessage("lotei", "Hey boss, how can I help you today?");
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
                    text: Lotei.modelName
                    color: "#ff2fb0"; opacity: 0.7
                    anchors.left: dot.right; anchors.leftMargin: 8 + glitch.off
                    anchors.verticalCenter: parent.verticalCenter
                    font.family: "Share Tech Mono"; font.pixelSize: 12; font.bold: true
                }
                Text {
                    text: Lotei.modelName
                    color: "#00e5ff"; opacity: 0.7
                    anchors.left: dot.right; anchors.leftMargin: 8 - glitch.off
                    anchors.verticalCenter: parent.verticalCenter
                    font.family: "Share Tech Mono"; font.pixelSize: 12; font.bold: true
                }
                Text {
                    id: modelFg
                    text: Lotei.modelName
                    color: "#eaffea"
                    anchors.left: dot.right; anchors.leftMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                    font.family: "Share Tech Mono"; font.pixelSize: 12; font.bold: true
                }
            }
            Item { Layout.fillWidth: true }

            // Clear conversation
            Rectangle {
                Layout.preferredWidth: 40
                Layout.preferredHeight: 18
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
                Layout.preferredWidth: 22
                Layout.preferredHeight: 18
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
                Layout.preferredWidth: 22
                Layout.preferredHeight: 18
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
                width: ListView.view.width
                spacing: 1

                TextEdit { id: clip; visible: false }  // hidden clipboard helper

                Row {
                    spacing: 8
                    Text {
                        text: model.role === "lotei" ? root.aiName : "you"
                        color: model.role === "lotei" ? Theme.color.lightorange2 : Theme.color.mediumorange1
                        font.family: "Share Tech Mono"
                        font.pixelSize: 11
                    }
                }
                TextEdit {
                    width: parent.width
                    text: model.text
                    readOnly: true
                    selectByMouse: true
                    persistentSelection: true
                    wrapMode: TextEdit.Wrap
                    textFormat: model.role === "lotei" ? TextEdit.MarkdownText : TextEdit.PlainText
                    color: "white"
                    font.family: "Share Tech Mono"
                    font.pixelSize: 13
                    onLinkActivated: function(link) { Qt.openUrlExternally(link) }
                }

                // ---- Manual save panel (model-free) --------------------------
                // Shown for LOTEI replies that contain a code block. YOU pick the
                // folder + name and hit Salvar; the app writes straight to the SD.
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
                                    text: "⧉ copiar"
                                    color: copyBtn.containsMouse ? Theme.color.lightorange2 : Theme.color.mediumorange1
                                    font.family: "Share Tech Mono"; font.pixelSize: 12
                                    MouseArea {
                                        id: copyBtn; anchors.fill: parent; anchors.margins: -4
                                        hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                                        onClicked: { clip.text = scriptArea.text; clip.selectAll(); clip.copy(); }
                                    }
                                }
                                Text {
                                    text: "▼ salvar"
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
}
