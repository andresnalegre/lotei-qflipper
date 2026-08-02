import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import Qt.labs.platform 1.1 as Pf

import Theme 1.0
import QFlipper 1.0

Item {
    id: mainWindow

    signal expandStarted
    signal expandFinished
    signal collapseStarted
    signal collapseFinished
    signal resizeStarted
    signal resizeFinished

    property alias controls: windowControls

    readonly property int baseWidth: 850
    readonly property int baseHeight: 540

    // Open height = exactly the content "screen" box (mainContent), top aligned to
    // its top edge so there's no sliver showing above the log.
    readonly property int logHeight: mainContent.height + 13
    readonly property int minimumLogHeight: 200

    readonly property int shadowSize: 16
    readonly property int shadowOffset: 4

    readonly property var deviceState: Backend.deviceState
    readonly property var deviceInfo: deviceState ? deviceState.info : undefined

    Component.onCompleted: {
        if(App.updateStatus === App.CanUpdate) {
            askForSelfUpdate();
        } else {
            App.updateStatusChanged.connect(askForSelfUpdate);
        }
    }

    width: baseWidth
    height: baseHeight

    x: shadowSize
    y: shadowSize - shadowOffset

    Pf.Menu {
        Pf.MenuItem {
            text: qsTr("Check for updates")
            role: Pf.MenuItem.ApplicationSpecificRole
            shortcut: "Ctrl+U"
            onTriggered: App.checkForUpdates()
        }

        Pf.MenuItem {
            text: qsTr("Refresh firmware")
            role: Pf.MenuItem.ApplicationSpecificRole
            shortcut: "Ctrl+R"
            onTriggered: Backend.checkFirmwareUpdates()
        }
    }

    PropertyAnimation {
        id: logExpand

        duration: 500
        target: logView
        property: "height"

        to: logHeight
        easing.type: Easing.InOutQuad

        onStarted: mainWindow.expandStarted()
        onFinished: mainWindow.expandFinished()
    }

    PropertyAnimation {
        id: logCollapse
        target: logExpand.target
        easing: logExpand.easing
        duration: logExpand.duration
        property: logExpand.property

        to: 0

        onStarted: mainWindow.collapseStarted()
        onFinished: mainWindow.collapseFinished()
    }

    ConfirmationDialog {
        id: confirmationDialog
        radius: bg.radius
        parent: bg
    }

    SelfUpdateDialog {
        id: selfUpdateDialog
        radius: bg.radius
        parent: bg
    }

    WindowShadow {
        id: shadow
        anchors.fill: mainWindow
        anchors.margins: -mainWindow.shadowSize
        anchors.topMargin: -(mainWindow.shadowSize - mainWindow.shadowOffset)
        anchors.bottomMargin: -(mainWindow.shadowSize + mainWindow.shadowOffset)
        opacity: 0.75
    }

    Rectangle {
        id: blackBorder
        anchors.fill: parent
        anchors.margins: -1
        radius: bg.radius + 1
        opacity: 0.5
        color: "black"
    }

    Rectangle {
        id: bg
        radius: 10
        anchors.fill: parent

        color: "black"
        border.color: Theme.color.mediumorange3
        border.width: 2
    }

    WindowControls {
        id: windowControls
        closeEnabled: Backend.backendState <= ApplicationBackend.ScreenStreaming ||
                      Backend.backendState >= ApplicationBackend.Finished

        controlPath: "qrc:/assets/gfx/controls"

        anchors.top: mainWindow.top
        anchors.left: mainContent.left
        anchors.right: mainContent.right
        anchors.bottom: mainContent.top

        anchors.topMargin: bg.border.width
    }

    // The top-bar controls belong to the home screen, so they follow it rather
    // than each repeating the reasoning. Every screen added since -- the update
    // run, the finish screen, our own transfers -- meant another clause to
    // remember in four places; this is the one place it lives now.
    // A transfer keeps the backend in Ready while it runs, so that alone is not
    // enough; once it ends the backend moves to Finished or ErrorOccured and
    // takes care of the rest.
    readonly property bool homeVisible: Backend.backendState === ApplicationBackend.Ready &&
                                        !Lotei.transferActive

    GridBackground {
        id: mainContent

        anchors.horizontalCenter: parent.horizontalCenter
        y: 38

        width: 800 + border.width * 2
        height: 430 + border.width * 2

        TextLabel {
            id: versionLabel
            visible: false

            anchors.top: parent.top
            anchors.right: parent.right
            anchors.margins: 10
            anchors.rightMargin: 16

            color: Theme.color.lightorange2
            opacity: 0.5

            font.family: "ProggySquareTT"
            font.pixelSize: 16

            text: App.version

            // TODO: Implement copy version to clipboard
        }

        TextLabel {
            id: colorsButton
            visible: false
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.margins: 10
            anchors.leftMargin: 16

            color: Theme.color.lightorange2
            opacity: colorsMouse.containsMouse ? 1.0 : 0.5

            font.family: "ProggySquareTT"
            font.pixelSize: 16
            text: "COLORS"

            MouseArea {
                id: colorsMouse
                anchors.fill: parent
                anchors.margins: -6
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: colorEditor.open = true
            }
        }

        TextLabel {
            id: portToggle
            anchors.top: mainContent.top
            anchors.left: mainContent.left
            anchors.leftMargin: 195
            anchors.topMargin: 36

            visible: mainWindow.homeVisible
            color: Backend.portReleased ? Theme.color.lightgreen : Theme.color.lightorange2
            opacity: portMouse.containsMouse ? 1.0 : (Backend.portReleased ? 1.0 : 0.5)

            font.family: "ProggySquareTT"
            font.pixelSize: 16
            text: Backend.portReleased ? "RECONNECT" : "RELEASE PORT"

            MouseArea {
                id: portMouse
                anchors.fill: parent
                anchors.margins: -6
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: Backend.portReleased ? Backend.reacquirePort() : Backend.releasePort()
            }
        }

        // BLE connection panel trigger
        TextLabel {
            id: bleButton
            anchors.top: portToggle.top
            anchors.left: portToggle.right
            anchors.leftMargin: 20

            visible: mainWindow.homeVisible
            color: (Ble.sessionActive || Ble.connected) ? Theme.color.lightgreen : Theme.color.lightorange2
            opacity: (bleMouse.containsMouse || Ble.sessionActive) ? 1.0 : 0.5

            font.family: "ProggySquareTT"
            font.pixelSize: 16
            text: Ble.sessionActive ? "BLE ●" : "BLE"

            MouseArea {
                id: bleMouse
                anchors.fill: parent
                anchors.margins: -6
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: bleOverlay.open = true
            }
        }

        // In-app Flipper CLI terminal trigger
        TextLabel {
            id: cliButton
            anchors.top: portToggle.top
            anchors.left: bleButton.right
            anchors.leftMargin: 20

            visible: mainWindow.homeVisible
            color: Cli.active ? Theme.color.lightgreen : Theme.color.lightorange2
            opacity: (cliMouse.containsMouse || Cli.active) ? 1.0 : 0.5

            font.family: "ProggySquareTT"
            font.pixelSize: 16
            text: Cli.active ? "CLI ●" : "CLI"

            MouseArea {
                id: cliMouse
                anchors.fill: parent
                anchors.margins: -6
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: Cli.open = true
            }
        }

        DeviceWidget {
            id: deviceWidget
            opacity: Backend.backendState !== ApplicationBackend.ScreenStreaming &&
                     Backend.backendState !== ApplicationBackend.ErrorOccured ? 1 : 0

            // Ready parks it on the right, beside the install button. The
            // WaitingForDevices screen wants it left of centre to make room for
            // the "connect your Flipper" text. Everything in between -- the
            // update run and the finish screen -- is a full-width overlay with
            // the title, progress bar and status all centred, so the device has
            // to be centred too; the -100 was leaving it visibly off-axis.
            x: {
                if(Backend.backendState === ApplicationBackend.Ready && !Lotei.transferActive) {
                    return Math.round(mainContent.width / 2);
                }
                const centred = Math.round((mainContent.width - width) / 2);
                return Backend.backendState === ApplicationBackend.WaitingForDevices ? centred - 100 : centred;
            }
            // Ready parks it at the top of the home layout. The other screens
            // are centred full-width overlays, so it moves down with the rest of
            // their group. The finish screen shifts further than the update one
            // because it has no status line under the button -- these mirror
            // contentShift in UpdateOverlay.qml and FinishOverlay.qml.
            y: {
                // A transfer borrows the update screen, so the widget sits
                // where that screen expects it.
                if(Lotei.transferActive)                                 { return 94 + 26; }
                if(Backend.backendState === ApplicationBackend.Ready)    { return 94; }
                if(Backend.backendState === ApplicationBackend.Finished) { return 94 + 42; }
                return 94 + 26;
            }

            onScreenStreamRequested: Backend.startFullScreenStreaming()
        }

        NoDeviceOverlay {
            id: noDeviceOverlay
            anchors.fill: parent
            opacity: Backend.backendState === ApplicationBackend.WaitingForDevices ? 1 : 0
        }

        HomeOverlay {
            id: homeOverlay
            backgroundRect: bg
            anchors.fill: parent
            // Hidden during a transfer: the backend stays Ready throughout
            // (these are our own operations, not a qFlipper state change), so
            // without this the home screen shows through the update overlay.
            opacity: mainWindow.homeVisible ? 1 : 0
        }

        UpdateOverlay {
            id: updateOverlay
            backgroundRect: bg
            anchors.fill: parent
            // Backup, restore and format reuse this screen rather than getting
            // one of their own -- same layout, same device widget, only the
            // title and the status line differ.
            opacity: (Backend.backendState > ApplicationBackend.ScreenStreaming &&
                      Backend.backendState < ApplicationBackend.Finished) ||
                     Lotei.transferActive ? 1 : 0
        }

        FinishOverlay {
            id: finishOverlay
            backgroundRect: bg
            anchors.fill: parent
            opacity: Backend.backendState === ApplicationBackend.Finished ||
                     Backend.backendState === ApplicationBackend.ErrorOccured ? 1 : 0
        }

        StreamOverlay {
            id: streamOverlay
            anchors.fill: parent
            opacity: Backend.backendState === ApplicationBackend.ScreenStreaming ? 1 : 0
        }
    }

    RowLayout {
        id: footerLayour
        width: mainContent.width

        height: 42
        spacing: 15

        anchors.horizontalCenter: mainContent.horizontalCenter
        anchors.top: mainContent.bottom
        anchors.topMargin: 13

        Button {
            id: logButton
            text: qsTr("LOGS")

            Layout.preferredWidth: 110
            Layout.fillHeight: true

            icon.source: checked ? "qrc:/assets/gfx/symbolic/arrow-up.svg" :
                                   "qrc:/assets/gfx/symbolic/arrow-down.svg"
            icon.width: 24
            icon.height: 24

            checkable: true

            Image {
                x: parent.width - width / 2 - 1
                y: -height / 2 + 1

                source: "qrc:/assets/gfx/images/alert-badge.svg"
                sourceSize: Qt.size(18, 18)

                visible: Logger.errorCount > 0 && !logButton.checked
            }

            onCheckedChanged: {
                Logger.errorCount = 0;

                if(checked) {
                    if(!logCollapse.running) {
                        logExpand.start();
                    } else {
                        checked = false;
                    }

                } else {
                    if(!logExpand.running) {
                        logCollapse.start();
                    } else {
                        checked = true;
                    }
                }
            }
        }

        // Music needs QtMultimedia, which is Windows-only in this build. On Linux
        // Lotei.hasAudio is false, so the player (and its QtMultimedia import) is
        // never loaded and the slot collapses.
        Loader {
            active: Lotei.hasAudio
            source: Lotei.hasAudio ? "qrc:/components/MusicPlayer.qml" : ""
            visible: active
            Layout.preferredWidth: active ? 250 : 0
            Layout.fillHeight: true
        }

        StatusBar {
            id: statusBar
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }

    TextView {
        id: logView
        visible: height > 0
        clip: true
        z: 10001   // above the tabs/menu (which sit at z ~9999) so it covers all

        // Overlay that slides UP from the footer over the content (Game-Boy style:
        // the window never changes size). Height is animated 0 <-> logHeight.
        anchors.left: mainContent.left
        anchors.right: mainContent.right
        anchors.bottom: footerLayour.top
        anchors.bottomMargin: 0

        height: 0

        background: Rectangle { color: "#0A0010"; radius: 8 }

        content.textFormat: TextArea.RichText
        content.text: Logger.logText

        menu: Menu {
            id: logMenu
            width: 170

            MenuItem {
                text: "Select all"
                onTriggered: logView.content.selectAll()
            }

            MenuItem {
                text: "Copy to clipboard"
                onTriggered: logView.content.copy()
            }

            MenuItem {
                text: "Browse all logs..."
                onTriggered: Qt.openUrlExternally(Logger.logsPath)
            }
        }
    }

    MouseArea {
        id: resizer

        property int prevMouseY

        width: parent.width
        height: 28

        visible: false   // window is fixed size now; no drag-to-resize
        cursorShape: Qt.SizeVerCursor

        anchors.bottom: parent.bottom

        preventStealing: true

        onPressed: {
            prevMouseY = mouseY;
            mainWindow.resizeStarted();
        }

        onReleased: mainWindow.resizeFinished()

        onMouseYChanged: {
            const dy = mouseY - prevMouseY;
            mainWindow.height = Math.max(mainWindow.height + dy, mainWindow.baseHeight + mainWindow.minimumLogHeight);
        }
    }

    Text {
        id: fullLogButton
        visible: opacity
        opacity: resizer.visible

        text: "<a href=\"#\">%1</a>".arg(qsTr("Open Full Log"))

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 6

        font.pixelSize: 14
        font.family: "Share Tech"
        font.capitalization: Font.AllUppercase

        color: Theme.color.lightorange2
        linkColor: Theme.color.lightorange2

        onLinkActivated: Qt.openUrlExternally(Logger.logsFile)

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.NoButton
            cursorShape: Qt.PointingHandCursor
        }

        Behavior on opacity {
            PropertyAnimation {
                duration: 200
                easing.type: Easing.OutCubic
            }
        }
    }

    // ---- LOTEI color editor: live per-color palette tweaking ----
    Item {
        id: colorEditor
        property bool open: false
        anchors.fill: parent
        visible: opacity > 0
        opacity: open ? 1 : 0
        z: 9999
        Behavior on opacity { NumberAnimation { duration: 140 } }

        Rectangle {
            anchors.fill: parent
            color: "#000000"
            opacity: 0.55
            MouseArea { anchors.fill: parent; onClicked: colorEditor.open = false }
        }

        Rectangle {
            anchors.centerIn: parent
            width: 478
            height: Math.min(parent.height - 70, 580)
            color: "#0b0410"
            radius: 10
            border.width: 2
            border.color: Theme.color.mediumorange2
            MouseArea {   // swallow clicks so they don't close
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.AllButtons
                onWheel: function(wheel) { wheel.accepted = true }
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 14
                spacing: 10

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10
                    Text {
                        text: "COLOR EDITOR"
                        color: Theme.color.lightorange2
                        font.family: "Share Tech Mono"; font.pixelSize: 16; font.bold: true
                        Layout.fillWidth: true
                    }
                    Text {
                        text: "reset to pink"
                        color: resetMouse.containsMouse ? Theme.color.lightorange2 : Theme.color.mediumorange1
                        font.family: "Share Tech Mono"; font.pixelSize: 12
                        MouseArea { id: resetMouse; anchors.fill: parent; anchors.margins: -5; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: Palette.reset() }
                    }
                    Text {
                        text: "✕"
                        color: closeMouse.containsMouse ? Theme.color.lightorange2 : Theme.color.mediumorange1
                        font.family: "Share Tech Mono"; font.pixelSize: 15
                        MouseArea { id: closeMouse; anchors.fill: parent; anchors.margins: -6; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: colorEditor.open = false }
                    }
                }

                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: Palette.names()
                    spacing: 7
                    boundsBehavior: Flickable.StopAtBounds
                    ScrollBar.vertical: ScrollBar {}

                    delegate: RowLayout {
                        width: ListView.view.width - 10
                        spacing: 7
                        property string cname: modelData
                        property color cval: Palette.colors[cname]

                        Rectangle {
                            Layout.preferredWidth: 26; Layout.preferredHeight: 26
                            radius: 4; color: parent.cval
                            border.width: 1; border.color: "#666"
                        }
                        Text {
                            text: parent.cname
                            color: Theme.color.lightorange2
                            font.family: "Share Tech Mono"; font.pixelSize: 11
                            Layout.preferredWidth: 92
                            elide: Text.ElideRight
                        }

                        // R / G / B sliders
                        Item {
                            Layout.preferredWidth: 52; Layout.preferredHeight: 16
                            Rectangle { anchors.verticalCenter: parent.verticalCenter; width: parent.width; height: 4; radius: 2; color: "#3a2a3a"
                                Rectangle { width: parent.width * cval.r; height: parent.height; radius: 2; color: "#e88888" } }
                            Rectangle { width: 8; height: 8; radius: 4; color: "#ffffff"; anchors.verticalCenter: parent.verticalCenter; x: parent.width * cval.r - 4 }
                            MouseArea { anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                                onPressed: Palette.setColor(cname, Qt.rgba(Math.max(0, Math.min(1, mouseX / width)), cval.g, cval.b, 1))
                                onPositionChanged: if (pressed) Palette.setColor(cname, Qt.rgba(Math.max(0, Math.min(1, mouseX / width)), cval.g, cval.b, 1)) }
                        }
                        Item {
                            Layout.preferredWidth: 52; Layout.preferredHeight: 16
                            Rectangle { anchors.verticalCenter: parent.verticalCenter; width: parent.width; height: 4; radius: 2; color: "#2a3a2a"
                                Rectangle { width: parent.width * cval.g; height: parent.height; radius: 2; color: "#88e888" } }
                            Rectangle { width: 8; height: 8; radius: 4; color: "#ffffff"; anchors.verticalCenter: parent.verticalCenter; x: parent.width * cval.g - 4 }
                            MouseArea { anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                                onPressed: Palette.setColor(cname, Qt.rgba(cval.r, Math.max(0, Math.min(1, mouseX / width)), cval.b, 1))
                                onPositionChanged: if (pressed) Palette.setColor(cname, Qt.rgba(cval.r, Math.max(0, Math.min(1, mouseX / width)), cval.b, 1)) }
                        }
                        Item {
                            Layout.preferredWidth: 52; Layout.preferredHeight: 16
                            Rectangle { anchors.verticalCenter: parent.verticalCenter; width: parent.width; height: 4; radius: 2; color: "#2a2a3a"
                                Rectangle { width: parent.width * cval.b; height: parent.height; radius: 2; color: "#8888e8" } }
                            Rectangle { width: 8; height: 8; radius: 4; color: "#ffffff"; anchors.verticalCenter: parent.verticalCenter; x: parent.width * cval.b - 4 }
                            MouseArea { anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                                onPressed: Palette.setColor(cname, Qt.rgba(cval.r, cval.g, Math.max(0, Math.min(1, mouseX / width)), 1))
                                onPositionChanged: if (pressed) Palette.setColor(cname, Qt.rgba(cval.r, cval.g, Math.max(0, Math.min(1, mouseX / width)), 1)) }
                        }

                        Text {
                            text: (Palette.colors, Palette.hex(cname))
                            color: Theme.color.mediumorange4
                            font.family: "Share Tech Mono"; font.pixelSize: 11
                            Layout.preferredWidth: 60
                        }
                    }
                }
            }
        }
    }

    // ---- BLE connection spike (Phase 1): scan / connect / prove a byte pipe ----
    Item {
        id: bleOverlay
        property bool open: false
        anchors.fill: parent
        visible: open
        z: 9995

        Rectangle {
            anchors.fill: parent; color: "#000000"; opacity: 0.72
            MouseArea { anchors.fill: parent; onClicked: bleOverlay.open = false }
        }

        Rectangle {
            anchors.centerIn: parent
            width: 580; height: 490
            color: "#0b0410"; radius: 12; border.width: 2; border.color: Theme.color.mediumorange2
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.AllButtons
                onWheel: function(wheel) { wheel.accepted = true }
            }

            ColumnLayout {
                anchors.fill: parent; anchors.margins: 20; spacing: 12

                RowLayout {
                    Layout.fillWidth: true
                    Text { text: "📶  BLE CONNECT"; color: Theme.color.lightorange2; font.family: "Share Tech Mono"; font.pixelSize: 20; font.bold: true }
                    Item { Layout.fillWidth: true }
                    Text {
                        text: "✕"; color: closeBleMouse.containsMouse ? Theme.color.lightorange2 : Theme.color.mediumorange4
                        font.family: "Share Tech Mono"; font.pixelSize: 18
                        MouseArea { id: closeBleMouse; anchors.fill: parent; anchors.margins: -6; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: bleOverlay.open = false }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true; spacing: 10
                    Rectangle {
                        Layout.preferredWidth: 110; Layout.preferredHeight: 30; radius: 6
                        border.width: 1; border.color: Theme.color.mediumorange2
                        color: scanMouse.containsMouse && !Ble.scanning ? Theme.color.mediumorange2 : "transparent"
                        Text { anchors.centerIn: parent; text: Ble.scanning ? "SCANNING…" : "SCAN"; color: Theme.color.lightorange2; font.family: "Share Tech Mono"; font.pixelSize: 12; font.bold: true }
                        MouseArea { id: scanMouse; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: if (!Ble.scanning) Ble.scan() }
                    }
                    Rectangle {
                        visible: Ble.connected
                        Layout.preferredWidth: 70; Layout.preferredHeight: 30; radius: 6
                        border.width: 1; border.color: Theme.color.mediumorange2
                        color: pingMouse.containsMouse ? Theme.color.mediumorange2 : "transparent"
                        Text { anchors.centerIn: parent; text: "PING"; color: Theme.color.lightorange2; font.family: "Share Tech Mono"; font.pixelSize: 12; font.bold: true }
                        MouseArea { id: pingMouse; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: Ble.ping() }
                    }
                    Rectangle {
                        visible: Ble.sessionActive
                        Layout.preferredWidth: 110; Layout.preferredHeight: 30; radius: 6
                        border.width: 1; border.color: Theme.color.mediumorange2
                        color: disMouse.containsMouse ? Theme.color.mediumorange2 : "transparent"
                        Text { anchors.centerIn: parent; text: "DISCONNECT"; color: Theme.color.lightorange2; font.family: "Share Tech Mono"; font.pixelSize: 12; font.bold: true }
                        MouseArea { id: disMouse; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: Ble.disconnectAll() }
                    }
                    Item { Layout.fillWidth: true }
                    Text {
                        text: Ble.sessionActive ? "● RPC session live" : (Ble.connected ? "● spike link" : "○ not connected")
                        color: Ble.sessionActive ? Theme.color.lightgreen : (Ble.connected ? Theme.color.lightorange2 : Theme.color.mediumorange1)
                        font.family: "Share Tech Mono"; font.pixelSize: 12
                    }
                }

                Text { text: "devices (click one → connects it as your ACTIVE device over BLE — device card, screen & LOTEI, all wireless):"; color: Theme.color.mediumorange4; font.family: "Share Tech Mono"; font.pixelSize: 11; Layout.fillWidth: true; wrapMode: Text.WordWrap }
                Flow {
                    Layout.fillWidth: true; spacing: 6
                    Repeater {
                        model: Ble.devices
                        delegate: Rectangle {
                            radius: 5; height: 26; width: dtxt.width + 18
                            border.width: 1; border.color: Theme.color.mediumorange2
                            color: dMouse.containsMouse ? Theme.color.mediumorange2 : "transparent"
                            Text { id: dtxt; anchors.centerIn: parent; text: modelData.name; color: Theme.color.lightorange2; font.family: "Share Tech Mono"; font.pixelSize: 11 }
                            MouseArea { id: dMouse; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: { Ble.connectDevice(index); bleOverlay.open = false } }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true; Layout.fillHeight: true
                    color: "#160a1c"; radius: 6; border.width: 1; border.color: Theme.color.mediumorange1
                    Flickable {
                        id: logFlick
                        anchors.fill: parent; anchors.margins: 8; clip: true
                        contentHeight: logText.height; contentWidth: width
                        Text {
                            id: logText
                            width: logFlick.width
                            text: Ble.status
                            color: Theme.color.lightorange2; font.family: "Share Tech Mono"; font.pixelSize: 11
                            wrapMode: Text.WrapAnywhere
                            onTextChanged: logFlick.contentY = Math.max(0, logText.height - logFlick.height)
                        }
                    }
                }

                Text {
                    Layout.fillWidth: true; wrapMode: Text.WordWrap
                    text: "⚠ if connect fails: pair the Flipper in Windows Bluetooth settings first (enter the PIN it shows), then SCAN again."
                    color: Theme.color.mediumorange1; font.family: "Share Tech Mono"; font.pixelSize: 10
                }
            }
        }
    }

    // ---- in-app Flipper CLI terminal (pauses RPC while open; USB only) ----
    Item {
        id: cliOverlay
        anchors.left: mainContent.left
        anchors.right: mainContent.right
        anchors.top: mainContent.top
        anchors.bottom: mainContent.bottom
        visible: Cli.open
        z: 9996

        Rectangle {
            anchors.fill: parent
            color: "#0b0410"; radius: 8; border.width: 2; border.color: Theme.color.mediumorange2
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.AllButtons
                onWheel: function(wheel) { wheel.accepted = true }
            }

            ColumnLayout {
                anchors.fill: parent; anchors.margins: 18; spacing: 10

                RowLayout {
                    Layout.fillWidth: true
                    Text { text: "FLIPPER CLI"; color: Theme.color.lightorange2; font.family: "Share Tech Mono"; font.pixelSize: 20; font.bold: true }
                    Item { Layout.fillWidth: true }
                    Text {
                        text: "✕"; color: closeCliMouse.containsMouse ? Theme.color.lightorange2 : Theme.color.mediumorange4
                        font.family: "Share Tech Mono"; font.pixelSize: 18
                        MouseArea { id: closeCliMouse; anchors.fill: parent; anchors.margins: -6; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: Cli.open = false }
                    }
                }

                Text {
                    visible: false
                    Layout.fillWidth: true; wrapMode: Text.WordWrap
                    text: Cli.status.length > 0 ? Cli.status
                        : "Flipper text CLI over USB. qFlipper's session pauses while this is open, and resumes when you close it."
                    color: Cli.active ? Theme.color.lightgreen : Theme.color.mediumorange4
                    font.family: "Share Tech Mono"; font.pixelSize: 11
                }

                Rectangle {
                    id: cliTerm
                    Layout.fillWidth: true; Layout.fillHeight: true
                    color: "#160a1c"; radius: 6; border.width: 1; border.color: Theme.color.mediumorange1

                    // Typed line lives here until Enter; the device echoes it back
                    // afterwards, so it's cleared the moment it's sent.
                    property string inputLine: ""
                    property int inputPos: 0        // caret offset inside inputLine
                    property int completeAt: -1     // caret offset Tab was pressed at
                    property var history: []
                    property int histIdx: 0

                    // ls --color, in effect. The buffer stays plain text all the
                    // way through the backend -- the colouring is decided here,
                    // per line, so it follows the theme palette instead of
                    // baking hex codes into C++.
                    function cssColor(c) {
                        var s = c.toString();
                        return (s.length === 9) ? ("#" + s.slice(3)) : s;   // drop #aarrggbb alpha
                    }
                    function esc(s) {
                        return s.replace(/&/g, "&amp;").replace(/</g, "&lt;")
                                .replace(/>/g, "&gt;").replace(/ /g, "&nbsp;");
                    }
                    // Every substitution above is one character for one
                    // character, so document positions still line up with the
                    // plain text -- that is what keeps the caret in place.
                    function span(c, t) {
                        return '<span style="color:' + cliTerm.cssColor(c) + '">' + t + '</span>';
                    }
                    function buildHtml(txt) {
                        var pt = Cli.promptText;
                        var lines = txt.split("\n");
                        var out = [];
                        for (var i = 0; i < lines.length; i++) {
                            var l = lines[i];
                            if (l.length === 0) { out.push(""); continue; }
                            if (pt.length > 0 && l.indexOf(pt) === 0) {
                                out.push(cliTerm.span(Theme.color.lightgreen, cliTerm.esc(pt))
                                       + cliTerm.span(Theme.color.lightorange2, cliTerm.esc(l.slice(pt.length))));
                                continue;
                            }
                            // Everything below has to survive the firmware's
                            // ASCII banner, which is full of leading spaces and
                            // stray slashes. So the tests are deliberately
                            // narrow: the log is matched on the marker the
                            // backend writes, status lines on "[ ", and a folder
                            // has to be a plain name with exactly one slash, at
                            // the end, and none of the punctuation that only
                            // ever shows up in line art.
                            var c;
                            if (/^ +\u00b7 /.test(l)) {
                                c = Theme.color.mediumorange4;          // verbose log
                            } else if (/^\[ /.test(l)) {
                                c = /error|mismatch|no such|can't|couldn't|failed/i.test(l)
                                    ? Theme.color.lightred1 : Theme.color.mediumorange3;
                            } else if (/^[^\/]+\/$/.test(l) && !/[\\|'"`,:;=<>~^*?]/.test(l)) {
                                c = Theme.color.lightgreen;             // folders
                            } else {
                                c = Theme.color.lightorange2;
                            }
                            out.push(cliTerm.span(c, cliTerm.esc(l)));
                        }
                        return out.join("<br>");
                    }
                    property string html: Cli.colored
                                        ? cliTerm.buildHtml(Cli.output + cliTerm.inputLine)
                                        : ""

                    // The caret is drawn as a block over the text (see cliCaret
                    // below), never inserted into it -- pushing a glyph in
                    // shifted everything to its right by one cell.
                    function setLine(s, pos) {
                        cliTerm.inputLine = s;
                        cliTerm.inputPos = Math.max(0, Math.min(pos, s.length));
                    }
                    function insertAtCursor(t) {
                        var p = cliTerm.inputPos;
                        cliTerm.setLine(cliTerm.inputLine.slice(0, p) + t + cliTerm.inputLine.slice(p),
                                        p + t.length);
                    }
                    function wordLeft(p) {
                        var s = cliTerm.inputLine;
                        while (p > 0 && s.charAt(p - 1) === " ") { p--; }
                        while (p > 0 && s.charAt(p - 1) !== " ") { p--; }
                        return p;
                    }
                    function wordRight(p) {
                        var s = cliTerm.inputLine;
                        while (p < s.length && s.charAt(p) === " ") { p++; }
                        while (p < s.length && s.charAt(p) !== " ") { p++; }
                        return p;
                    }
                    function runLine() {
                        var line = cliTerm.inputLine;
                        cliTerm.setLine("", 0);
                        cliTerm.completeAt = -1;
                        if (line.length > 0) { cliTerm.history.push(line); }
                        cliTerm.histIdx = cliTerm.history.length;
                        cliTerm.setFollow(true);
                        Cli.send(line);
                    }
                    // Paste: complete lines run as commands and the tail stays on
                    // the input line, the way a terminal treats a pasted block.
                    function pasteText(raw) {
                        if (!raw || raw.length === 0) { return; }
                        var lines = raw.replace(/\r\n/g, "\n").replace(/\r/g, "\n")
                                       .replace(/\t/g, " ").split("\n");
                        for (var i = 0; i < lines.length - 1; i++) {
                            cliTerm.insertAtCursor(lines[i]);
                            cliTerm.runLine();
                        }
                        cliTerm.insertAtCursor(lines[lines.length - 1]);
                        cliTerm.setFollow(true);
                    }

                    // Follow the tail only while the view is already parked at the
                    // bottom. The instant the user scrolls up or selects something,
                    // the view freezes -- otherwise a chatty command like `top`
                    // drags the scroll back down and wipes the selection every time
                    // it prints.
                    property bool follow: true
                    property string frozen: ""

                    function atBottom() {
                        return cliFlick.contentHeight <= cliFlick.height
                            || cliFlick.contentY >= cliFlick.contentHeight - cliFlick.height - 6;
                    }
                    // Guarded so Flickable's movement signals can't mistake our own
                    // scroll for the user grabbing the view.
                    property bool autoScrolling: false
                    function toBottom() {
                        cliTerm.autoScrolling = true;
                        cliFlick.contentY = Math.max(0, cliFlick.contentHeight - cliFlick.height);
                        cliTerm.autoScrolling = false;
                    }
                    function setFollow(v) {
                        if (v === cliTerm.follow) { return; }
                        if (!v) { cliTerm.frozen = cliText.text; }
                        cliTerm.follow = v;
                        if (v) { cliText.deselect(); Qt.callLater(cliTerm.toBottom); }
                    }

                    Flickable {
                        id: cliFlick
                        anchors.fill: parent; anchors.margins: 8; clip: true
                        contentHeight: cliText.height; contentWidth: width
                        boundsBehavior: Flickable.StopAtBounds

                        onMovementEnded: if (!cliTerm.autoScrolling) { cliTerm.setFollow(cliTerm.atBottom()); }

                        TextEdit {
                            id: cliText
                            width: cliFlick.width
                            // Steady block caret, no blink: the text used to change
                            // twice a second, which re-wrapped the last line and made
                            // the view hop once it was parked at the bottom.
                            textFormat: Cli.colored ? TextEdit.RichText : TextEdit.PlainText
                            text: cliTerm.follow
                                  ? (Cli.colored ? cliTerm.html : (Cli.output + cliTerm.inputLine))
                                  : cliTerm.frozen
                            readOnly: true
                            activeFocusOnPress: false
                            selectByMouse: true
                            persistentSelection: true
                            color: Theme.color.lightorange2; font.family: "Share Tech Mono"; font.pixelSize: 12
                            wrapMode: TextEdit.WrapAnywhere

                            // Height, not text: if the height didn't change there's
                            // nothing new to scroll to, and re-running toBottom on
                            // every caret blink is what produced the jitter.
                            onHeightChanged: if (cliTerm.follow) { cliTerm.toBottom(); }
                            // Selecting anything detaches, so the next chunk of
                            // output can't tear the selection apart.
                            onSelectedTextChanged: if (selectedText.length > 0) { cliTerm.setFollow(false); }

                            FontMetrics { id: cliMetrics; font: cliText.font }

                            // Block caret, drawn on top of the character it sits
                            // on rather than inserted into the line. Inserting a
                            // glyph pushed everything to its right one cell over,
                            // which is the gap that appeared when arrowing back
                            // into a command to edit it.
                            Rectangle {
                                id: cliCaret
                                visible: Cli.active && cliTerm.follow
                                color: Theme.color.lightorange2
                                opacity: 0.55
                                width: cliMetrics.advanceWidth("M")
                                x: caretRect.x
                                y: caretRect.y
                                height: caretRect.height

                                // Reading text and inputPos here is what keeps
                                // this re-evaluating as the line changes.
                                readonly property rect caretRect: {
                                    cliText.text;
                                    cliText.width;
                                    var n = cliTerm.inputLine.length;
                                    var p = Math.max(0, Math.min(cliTerm.inputPos, n));
                                    return cliText.positionToRectangle(Cli.output.length + p);
                                }
                            }
                        }
                    }

                    // Wheel is handled here so scrolling up detaches immediately,
                    // without waiting for a flick to settle.
                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.NoButton
                        onWheel: function(wheel) {
                            var maxY = Math.max(0, cliFlick.contentHeight - cliFlick.height);
                            cliFlick.contentY = Math.max(0, Math.min(maxY, cliFlick.contentY - wheel.angleDelta.y));
                            // Scrolling down back to the tail re-attaches; scrolling
                            // up detaches. Never flip on a downward nudge that lands
                            // a pixel short of the bottom.
                            if (wheel.angleDelta.y > 0) { cliTerm.setFollow(false); }
                            else if (cliTerm.atBottom()) { cliTerm.setFollow(true); }
                            wheel.accepted = true;
                        }
                    }

                    // Keyboard goes straight to the terminal -- no input box, no
                    // send button. Lines are still assembled client-side so the
                    // shortcuts (cd, cp, ls…) keep working.
                    Item {
                        id: cliKeys
                        anchors.fill: parent
                        focus: true

                        Keys.onPressed: function(event) {
                            if (!Cli.active) { return; }
                            var k = event.key;
                            var m = event.modifiers;
                            // On macOS Qt reports Cmd as ControlModifier and the real
                            // Ctrl as MetaModifier, so copy and interrupt land on the
                            // keys a Mac user expects. Elsewhere: Ctrl+Shift+C copies,
                            // Ctrl+C interrupts -- terminal semantics either way.
                            var mac = (Qt.platform.os === "osx" || Qt.platform.os === "macos");
                            var copyKey = mac ? (m & Qt.ControlModifier)
                                              : ((m & Qt.ControlModifier) && (m & Qt.ShiftModifier));
                            var ctrlKey = mac ? (m & Qt.MetaModifier)
                                              : ((m & Qt.ControlModifier) && !(m & Qt.ShiftModifier));

                            // Interrupt first, always. It must never be swallowed by
                            // a leftover selection -- that's what left `top` running
                            // with no way out.
                            if ((k === Qt.Key_C && ctrlKey) || k === Qt.Key_Escape) {
                                cliTerm.setLine("", 0);
                                cliTerm.completeAt = -1;
                                cliTerm.setFollow(true);
                                Cli.interrupt();
                                event.accepted = true;
                                return;
                            }
                            if (k === Qt.Key_C && copyKey) {
                                cliText.copy();
                                event.accepted = true;
                                return;
                            }
                            if (k === Qt.Key_A && copyKey) {
                                cliText.selectAll();
                                event.accepted = true;
                                return;
                            }
                            // Paste. Cmd+V on macOS, Ctrl+V or Ctrl+Shift+V elsewhere.
                            if (k === Qt.Key_V && (copyKey || ctrlKey)) {
                                cliTerm.pasteText(Cli.clipboardText());
                                event.accepted = true;
                                return;
                            }
                            // Tab completes the word under the caret: command
                            // names for the first one, real directory contents
                            // for the rest.
                            if (k === Qt.Key_Tab || k === Qt.Key_Backtab) {
                                cliTerm.completeAt = cliTerm.inputPos;
                                cliTerm.setFollow(true);
                                Cli.complete(cliTerm.inputLine.slice(0, cliTerm.inputPos));
                                event.accepted = true;
                                return;
                            }
                            // End now belongs to the caret; Ctrl-G still parks
                            // the view back at the tail.
                            if (k === Qt.Key_G && ctrlKey) {
                                cliTerm.setFollow(true);
                                event.accepted = true;
                                return;
                            }
                            if (k === Qt.Key_PageUp || k === Qt.Key_PageDown) {
                                var step = cliFlick.height * 0.9;
                                var maxY = Math.max(0, cliFlick.contentHeight - cliFlick.height);
                                cliFlick.contentY = Math.max(0, Math.min(maxY,
                                    cliFlick.contentY + (k === Qt.Key_PageUp ? -step : step)));
                                cliTerm.setFollow(cliTerm.atBottom());
                                event.accepted = true;
                                return;
                            }

                            // Word / line jumps: Option+arrow and Cmd+arrow on
                            // macOS, Ctrl+arrow and Home/End everywhere else.
                            var wordMod = mac ? (m & Qt.AltModifier)
                                              : ((m & Qt.AltModifier) || (m & Qt.ControlModifier));
                            var lineMod = mac ? (m & Qt.ControlModifier) : 0;
                            var p = cliTerm.inputPos;
                            var s = cliTerm.inputLine;

                            if (k === Qt.Key_Return || k === Qt.Key_Enter) {
                                cliTerm.runLine();
                                event.accepted = true;
                            } else if (k === Qt.Key_Left) {
                                cliTerm.inputPos = lineMod ? 0
                                                 : (wordMod ? cliTerm.wordLeft(p) : Math.max(0, p - 1));
                                event.accepted = true;
                            } else if (k === Qt.Key_Right) {
                                cliTerm.inputPos = lineMod ? s.length
                                                 : (wordMod ? cliTerm.wordRight(p) : Math.min(s.length, p + 1));
                                event.accepted = true;
                            } else if (k === Qt.Key_Home || (k === Qt.Key_A && ctrlKey)) {
                                cliTerm.inputPos = 0;
                                event.accepted = true;
                            } else if (k === Qt.Key_End || (k === Qt.Key_E && ctrlKey)) {
                                cliTerm.inputPos = s.length;
                                event.accepted = true;
                            } else if (k === Qt.Key_Backspace) {
                                if (wordMod) {
                                    var w = cliTerm.wordLeft(p);
                                    cliTerm.setLine(s.slice(0, w) + s.slice(p), w);
                                } else if (p > 0) {
                                    cliTerm.setLine(s.slice(0, p - 1) + s.slice(p), p - 1);
                                }
                                event.accepted = true;
                            } else if (k === Qt.Key_Delete) {
                                if (p < s.length) { cliTerm.setLine(s.slice(0, p) + s.slice(p + 1), p); }
                                event.accepted = true;
                            } else if (k === Qt.Key_U && ctrlKey) {
                                cliTerm.setLine(s.slice(p), 0);
                                event.accepted = true;
                            } else if (k === Qt.Key_K && ctrlKey) {
                                cliTerm.setLine(s.slice(0, p), p);
                                event.accepted = true;
                            } else if (k === Qt.Key_W && ctrlKey) {
                                var wl = cliTerm.wordLeft(p);
                                cliTerm.setLine(s.slice(0, wl) + s.slice(p), wl);
                                event.accepted = true;
                            } else if (k === Qt.Key_L && ctrlKey) {
                                cliTerm.setLine("", 0);
                                cliTerm.setFollow(true);
                                Cli.send("clear");
                                event.accepted = true;
                            } else if (k === Qt.Key_Up) {
                                if (cliTerm.histIdx > 0) {
                                    cliTerm.histIdx--;
                                    var hu = cliTerm.history[cliTerm.histIdx];
                                    cliTerm.setLine(hu, hu.length);
                                }
                                event.accepted = true;
                            } else if (k === Qt.Key_Down) {
                                if (cliTerm.histIdx < cliTerm.history.length - 1) {
                                    cliTerm.histIdx++;
                                    var hd = cliTerm.history[cliTerm.histIdx];
                                    cliTerm.setLine(hd, hd.length);
                                } else {
                                    cliTerm.histIdx = cliTerm.history.length;
                                    cliTerm.setLine("", 0);
                                }
                                event.accepted = true;
                            } else if (event.text.length > 0 && event.text.charCodeAt(0) >= 32
                                       && !ctrlKey && !copyKey) {
                                cliTerm.insertAtCursor(event.text);
                                cliTerm.setFollow(true);
                                event.accepted = true;
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            acceptedButtons: Qt.LeftButton
                            cursorShape: Qt.IBeamCursor
                            onPressed: function(mouse) { cliKeys.forceActiveFocus(); mouse.accepted = false; }
                        }
                    }

                    Connections {
                        target: Cli
                        function onOpenChanged() {
                            if (Cli.open) { cliTerm.setLine("", 0); cliTerm.setFollow(true); cliKeys.forceActiveFocus(); }
                        }
                        function onActiveChanged() { if (Cli.active) { cliKeys.forceActiveFocus(); } }
                        // Tab came back: the reply replaces everything left of
                        // where the caret was when it was pressed.
                        function onCompletion(line) {
                            if (cliTerm.completeAt < 0) { return; }
                            var tail = cliTerm.inputLine.slice(cliTerm.completeAt);
                            cliTerm.completeAt = -1;
                            cliTerm.setLine(line + tail, line.length);
                            cliTerm.setFollow(true);
                        }
                    }
                }
            }
        }
    }

    // ---- file editor (the ONE editor) --------------------------------------
    // Opened from two places, deliberately sharing one panel so they can never
    // look different: the CLI's "edit <path>" (Cli.editRequested) and the file
    // manager's double-click (Lotei.fileOpened). editorOverlay.source records
    // which opened it, so Save routes to the matching backend.
    Item {
        id: editorOverlay
        anchors.left: mainContent.left
        anchors.right: mainContent.right
        anchors.top: mainContent.top
        anchors.bottom: mainContent.bottom
        visible: open
        z: 9997

        property bool open: false
        property string path: ""
        property string source: ""      // "cli" or "fm"
        property bool dirty: false
        property bool justSaved: false

        function beginEdit(src, p, content) {
            editorOverlay.source = src;
            editorOverlay.path = p;
            cliEditArea.text = content;
            editorOverlay.dirty = false;
            editorOverlay.justSaved = false;
            editorOverlay.open = true;
            cliEditArea.forceActiveFocus();
        }
        function doSave() {
            if (editorOverlay.source === "fm") { Lotei.writeFile(editorOverlay.path, cliEditArea.text); }
            else { Cli.saveEditedFile(editorOverlay.path, cliEditArea.text); }
        }
        function markSaved() {
            editorOverlay.dirty = false;
            editorOverlay.justSaved = true;
            cliSavedResetTimer.restart();
        }

        Connections {
            target: Cli
            function onEditRequested(path, content) { editorOverlay.beginEdit("cli", path, content); }
            function onEditSaved(path) { editorOverlay.markSaved(); }
            function onEditSaveError(path, message) { cliEditStatus.text = "save failed: " + message; }
        }
        Connections {
            target: Lotei
            function onFileOpened(path, content) { editorOverlay.beginEdit("fm", path, content); }
            function onFileSaved(path) {
                if (editorOverlay.source === "fm") {
                    editorOverlay.markSaved();
                    Backend.fileManager.refresh();
                }
            }
            function onFileEditError(msg) {
                if (editorOverlay.source === "fm") { cliEditStatus.text = "save failed: " + msg; }
            }
        }

        Timer { id: cliSavedResetTimer; interval: 1500; onTriggered: editorOverlay.justSaved = false }

        Rectangle {
            anchors.fill: parent
            color: "#0b0410"; radius: 8; border.width: 2; border.color: Theme.color.mediumorange2

            MouseArea {
                anchors.fill: parent; hoverEnabled: true
                acceptedButtons: Qt.AllButtons
                onWheel: function(wheel) { wheel.accepted = true }
            }

            ColumnLayout {
                anchors.fill: parent; anchors.margins: 18; spacing: 10

                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        text: "EDIT"; color: Theme.color.lightorange2
                        font.family: "Share Tech Mono"; font.pixelSize: 20; font.bold: true
                    }
                    Text {
                        text: editorOverlay.path + (editorOverlay.dirty ? "  *" : "")
                        color: Theme.color.mediumorange1
                        font.family: "Share Tech Mono"; font.pixelSize: 11
                        Layout.leftMargin: 8; Layout.alignment: Qt.AlignVCenter
                        elide: Text.ElideMiddle; Layout.fillWidth: true
                    }
                    Text {
                        text: "\u2715"
                        color: cliCloseMouse.containsMouse ? Theme.color.lightorange2 : Theme.color.mediumorange4
                        font.family: "Share Tech Mono"; font.pixelSize: 18
                        MouseArea {
                            id: cliCloseMouse; anchors.fill: parent; anchors.margins: -6
                            hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                            onClicked: editorOverlay.open = false
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true; Layout.fillHeight: true
                    color: "#160a1c"; radius: 6; border.width: 1; border.color: Theme.color.mediumorange1

                    Flickable {
                        id: cliEditFlick
                        anchors.fill: parent; anchors.margins: 8
                        clip: true
                        contentWidth: width
                        contentHeight: cliEditArea.implicitHeight
                        ScrollBar.vertical: ScrollBar { }

                        TextArea.flickable: TextArea {
                            id: cliEditArea
                            wrapMode: TextArea.NoWrap
                            selectByMouse: true
                            persistentSelection: true
                            color: Theme.color.lightgreen
                            selectionColor: "#3b5bdb"
                            selectedTextColor: "white"
                            font.family: "Share Tech Mono"; font.pixelSize: 12
                            background: null
                            onTextChanged: if (editorOverlay.open) { editorOverlay.dirty = true; editorOverlay.justSaved = false; }
                            Keys.onPressed: function(e) {
                                if ((e.modifiers & Qt.ControlModifier || e.modifiers & Qt.MetaModifier)
                                    && e.key === Qt.Key_S) {
                                    editorOverlay.doSave();
                                    e.accepted = true;
                                }
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true; spacing: 10
                    Text {
                        id: cliEditStatus
                        text: editorOverlay.justSaved ? "saved" : ""
                        color: editorOverlay.justSaved ? Theme.color.lightgreen : Theme.color.mediumorange4
                        font.family: "Share Tech Mono"; font.pixelSize: 11
                        Layout.fillWidth: true
                    }
                    Text {
                        text: "CANCEL"
                        color: cliCancelMouse.containsMouse ? Theme.color.lightorange2 : Theme.color.mediumorange4
                        font.family: "Share Tech Mono"; font.pixelSize: 14; font.bold: true
                        MouseArea {
                            id: cliCancelMouse; anchors.fill: parent; anchors.margins: -6
                            hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                            onClicked: editorOverlay.open = false
                        }
                    }
                    Item { width: 18 }
                    Text {
                        text: "SAVE"
                        color: cliSaveMouse.containsMouse ? Theme.color.lightgreen
                             : (editorOverlay.dirty ? Theme.color.lightorange2 : Theme.color.mediumorange4)
                        font.family: "Share Tech Mono"; font.pixelSize: 14; font.bold: true
                        MouseArea {
                            id: cliSaveMouse; anchors.fill: parent; anchors.margins: -6
                            hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                            onClicked: editorOverlay.doSave()
                        }
                    }
                }
            }
        }

        Keys.onEscapePressed: editorOverlay.open = false
    }


    // Reinstall and Update run from the tools tab and the main screen, where
    // the store panel is closed -- and its failed()/progress() handlers live
    // inside that panel. A download that died there failed in total silence.
    // Reopening the panel puts the error back in front of the user.
    Connections {
        target: Firmware
        function onFailed(index, message) {
            if(!Firmware.open) { Firmware.open = true; }
        }
    }

    // Hand the running firmware version to the store so it can tell which row
    // is the one already installed. Deliberately at root scope: this used to
    // live inside the store overlay, which is only visible while the panel is
    // open -- one refactor of that Item into a Loader and the version would
    // quietly stop arriving, taking the update check with it.
    Binding {
        target: Firmware
        property: "deviceVersion"
        value: (Backend.deviceState && Backend.deviceState.info && Backend.deviceState.info.firmware)
               ? Backend.deviceState.info.firmware.version : ""
    }

    // ---- custom-firmware store (Official / Momentum / Unleashed / RogueMaster) ----
    Item {
        id: firmwareOverlay
        anchors.left: mainContent.left
        anchors.right: mainContent.right
        anchors.top: mainContent.top
        anchors.bottom: mainContent.bottom
        visible: Firmware.open
        z: 9990

        property real   dlProgress: 0
        property string dlNote: ""
        property string errText: ""

        Connections {
            target: Firmware
            function onProgress(index, frac, note) {
                firmwareOverlay.dlProgress = frac; firmwareOverlay.dlNote = note; firmwareOverlay.errText = "";
            }
            function onFailed(index, message) { firmwareOverlay.errText = message; firmwareOverlay.dlNote = ""; }
            function onReadyToInstall(fileUrl) {
                firmwareOverlay.dlNote = ""; firmwareOverlay.errText = "";
                Firmware.open = false;              // reveal qFlipper's normal update UI
                Backend.installFirmware(fileUrl);
            }
        }

        Rectangle {
            anchors.fill: parent
            color: "#0b0410"; radius: 8
            border.width: 2; border.color: Theme.color.mediumorange2
            MouseArea {   // swallow clicks behind the panel
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.AllButtons
                onWheel: function(wheel) { wheel.accepted = true }
            }

            ColumnLayout {
                anchors.fill: parent; anchors.margins: 22; spacing: 12

                RowLayout {
                    Layout.fillWidth: true
                    Text { text: "CUSTOM FIRMWARE"; color: Theme.color.lightorange2; font.family: "Share Tech Mono"; font.pixelSize: 20; font.bold: true }
                    Item { Layout.fillWidth: true }
                    Text {
                        text: "✕"; color: closeFwMouse.containsMouse ? Theme.color.lightorange2 : Theme.color.mediumorange4
                        font.family: "Share Tech Mono"; font.pixelSize: 18
                        MouseArea { id: closeFwMouse; anchors.fill: parent; anchors.margins: -6; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: Firmware.open = false }
                    }
                }

                Text {
                    Layout.fillWidth: true; wrapMode: Text.WordWrap
                    textFormat: Text.StyledText
                    color: Theme.color.mediumorange4; font.family: "Share Tech Mono"; font.pixelSize: 12
                    // The version itself gets the same green it has on the main
                    // screen, so the two places agree at a glance.
                    text: (Backend.deviceState && Backend.deviceState.info && Backend.deviceState.info.firmware)
                          ? ("Flipper Version : <font color=\"" + Theme.color.lightgreen + "\">"
                             + Backend.deviceState.info.firmware.version + "</font>")
                          : "Connect your Flipper to flash. Latest builds are shown below."
                }

                ListView {
                    Layout.fillWidth: true; Layout.fillHeight: true
                    spacing: 8; clip: true
                    boundsBehavior: Flickable.StopAtBounds
                    model: Firmware.sources
                    delegate: Rectangle {
                            id: fwRow
                            width: ListView.view.width
                            height: 56
                            radius: 8; color: "#160a1c"
                            border.width: 1; border.color: Theme.color.mediumorange2

                            readonly property int padH: 12
                            readonly property int colGap: 12
                            readonly property int wChannel: 116
                            readonly property int wVersion: 164
                            readonly property int wInstall: 88

                            Rectangle {
                                id: instBtn
                                anchors.right: parent.right
                                anchors.rightMargin: fwRow.padH
                                anchors.verticalCenter: parent.verticalCenter
                                width: fwRow.wInstall; height: 30
                                radius: 6
                                // Staging only needs a fetched source; no device
                                // has to be attached to pick one. The build
                                // already running is the one thing to refuse.
                                property bool canGo: modelData.ready && !modelData.upToDate
                                enabled: canGo
                                opacity: canGo ? 1.0 : 0.4
                                color: instMouse.containsMouse && canGo ? Theme.color.mediumorange2 : "transparent"
                                border.width: 1
                                border.color: Theme.color.mediumorange2
                                Text {
                                    anchors.centerIn: parent
                                    text: modelData.action
                                    color: Theme.color.lightorange2
                                    font.family: "Share Tech Mono"; font.pixelSize: 12; font.bold: true
                                }
                                MouseArea {
                                    id: instMouse; anchors.fill: parent; hoverEnabled: true
                                    cursorShape: instBtn.canGo ? Qt.PointingHandCursor : Qt.ArrowCursor
                                    onClicked: {
                                        if(!instBtn.canGo) return;
                                        const wasStaged = modelData.selected;
                                        Firmware.select(index);
                                        // Staging is done -- send them where the
                                        // install actually happens. Unstaging
                                        // keeps the panel open to pick another.
                                        if(!wasStaged) Firmware.open = false;
                                    }
                                }
                            }

                            Column {
                                id: verLbl
                                anchors.right: instBtn.left
                                anchors.rightMargin: fwRow.colGap
                                anchors.verticalCenter: parent.verticalCenter
                                width: fwRow.wVersion
                                spacing: 1

                                Text {
                                    width: parent.width
                                    horizontalAlignment: Text.AlignHCenter
                                    elide: Text.ElideRight
                                    text: modelData.status === "checking" ? "checking…"
                                        : modelData.status === "error"    ? "unavailable"
                                        : modelData.latest
                                    color: modelData.status === "error" ? Theme.color.mediumorange1 : Theme.color.lightgreen
                                    font.family: "Share Tech Mono"; font.pixelSize: 12
                                }
                                Text {
                                    width: parent.width
                                    horizontalAlignment: Text.AlignHCenter
                                    elide: Text.ElideRight
                                    // Empty when the feed didn't carry a date;
                                    // the row just loses the second line.
                                    visible: text.length > 0
                                    text: modelData.status === "ready" ? (modelData.date || "") : ""
                                    color: Theme.color.mediumorange4
                                    font.family: "Share Tech Mono"; font.pixelSize: 10
                                }
                            }

                            Rectangle {
                                id: chBox
                                anchors.right: verLbl.left
                                anchors.rightMargin: fwRow.colGap
                                anchors.verticalCenter: parent.verticalCenter
                                width: fwRow.wChannel; height: 26
                                radius: 5
                                // A source with a single channel still shows
                                // which one it is -- it just isn't a dropdown.
                                // Hiding the box entirely left the row looking
                                // like the channel was missing rather than fixed.
                                readonly property bool pickable: (modelData.channelCount || 0) > 1
                                enabled: pickable
                                color: (chMouse.containsMouse && pickable) ? Theme.color.mediumorange2 : "transparent"
                                border.width: 1; border.color: Theme.color.mediumorange2
                                Text {
                                    id: chLbl
                                    anchors.left: parent.left; anchors.leftMargin: 8
                                    // Centred in the box whether or not it can
                                    // be opened; the arrow floats over the right
                                    // edge instead of pushing the text off to
                                    // the left, so pickable and fixed rows read
                                    // the same.
                                    anchors.right: parent.right
                                    anchors.rightMargin: 8
                                    anchors.verticalCenter: parent.verticalCenter
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                    elide: Text.ElideRight
                                    // Always the source's own name for the
                                    // stream, whether or not it can be changed:
                                    // Xero publishes "release", ARF only ever
                                    // tags "dev". Calling ARF's stream "release"
                                    // would suggest a stability it never claims.
                                    text: modelData.channel
                                    color: Theme.color.lightorange2; font.family: "Share Tech Mono"; font.pixelSize: 11
                                }
                                Text {
                                    id: chArrow
                                    anchors.right: parent.right; anchors.rightMargin: 8
                                    anchors.verticalCenter: parent.verticalCenter
                                    visible: chBox.pickable      // nothing to open when there is one channel
                                    text: "▾"; color: Theme.color.lightorange2; font.family: "Share Tech Mono"; font.pixelSize: 9
                                }
                                MouseArea {
                                    id: chMouse; anchors.fill: parent; hoverEnabled: chBox.pickable
                                    cursorShape: chBox.pickable ? Qt.PointingHandCursor : Qt.ArrowCursor
                                    onClicked: if (chBox.pickable) Firmware.cycleChannel(index)
                                }
                            }

                            Column {
                                anchors.left: parent.left
                                anchors.leftMargin: fwRow.padH
                                anchors.right: chBox.left
                                anchors.rightMargin: fwRow.colGap
                                anchors.verticalCenter: parent.verticalCenter
                                spacing: 2
                                Text {
                                    width: parent.width; elide: Text.ElideRight
                                    text: modelData.name
                                    color: Theme.color.lightorange2; font.family: "Share Tech Mono"; font.pixelSize: 15; font.bold: true
                                }
                                Text {
                                    width: parent.width; elide: Text.ElideRight
                                    text: modelData.blurb
                                    color: Theme.color.mediumorange4; font.family: "Share Tech Mono"; font.pixelSize: 11
                                }
                            }
                        }
                }

                Text {
                    visible: firmwareOverlay.dlNote.length > 0 || firmwareOverlay.errText.length > 0
                    Layout.fillWidth: true; wrapMode: Text.WordWrap
                    text: firmwareOverlay.errText.length > 0
                          ? firmwareOverlay.errText
                          : (firmwareOverlay.dlNote + "  " + Math.round(firmwareOverlay.dlProgress * 100) + "%")
                    color: firmwareOverlay.errText.length > 0 ? Theme.color.lightred1 : Theme.color.lightgreen
                    font.family: "Share Tech Mono"; font.pixelSize: 12
                }

                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        id: recheckFwLabel
                        // Reports what it is doing, then what it found, then
                        // goes back to being a button. Silently returning to
                        // idle gave no way to tell a finished check from one
                        // that never ran.
                        property bool showingResult: false
                        text: Firmware.busy            ? "downloading…"
                            : Firmware.checking        ? "checking for new updates…"
                            : showingResult && Firmware.checkSummary.length > 0 ? Firmware.checkSummary
                            : "↻ check for updates"
                        // Result in the app's own colour: green here read as a
                        // status light, competing with the green used for the
                        // installed version elsewhere on screen.
                        color: showingResult && Firmware.checkSummary.length > 0
                               ? Theme.color.lightorange2
                               : (recheckFwMouse.containsMouse ? Theme.color.lightorange2 : Theme.color.mediumorange4)
                        font.family: "Share Tech Mono"; font.pixelSize: 12

                        onTextChanged: if (Firmware.checking) { recheckFwLabel.showingResult = true; }
                        Connections {
                            target: Firmware
                            function onChanged() {
                                if (!Firmware.checking && recheckFwLabel.showingResult) { resultTimer.restart(); }
                            }
                        }
                        Timer {
                            id: resultTimer; interval: 4000
                            onTriggered: recheckFwLabel.showingResult = false
                        }

                        MouseArea {
                            id: recheckFwMouse; anchors.fill: parent; anchors.margins: -4
                            hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                            onClicked: if (!Firmware.busy && !Firmware.checking) {
                                           recheckFwLabel.showingResult = true;
                                           Firmware.refresh();
                                       }
                        }
                    }
                    Item { Layout.fillWidth: true }
                    Row {
                        spacing: 5
                        Text {
                            text: "\u26a0"
                            color: "#ffcc33"          // the warning mark carries the alarm
                            font.family: "Share Tech Mono"; font.pixelSize: 11
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Text {
                            // White rather than red: this line sits on screen the
                            // whole time the panel is open, and red at that size
                            // is tiring to read for a message that is a caution,
                            // not an error.
                            text: "flashing replaces your current firmware"
                            color: "#ffffff"
                            font.family: "Share Tech Mono"; font.pixelSize: 10
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }
            }
        }
    }

    // ---- first-run setup wizard (shown only until Lotei.setupComplete) ----
    Item {
        id: setupWizard
        anchors.fill: parent
        visible: !Lotei.setupComplete
        z: 10000
        property int step: 0

        Rectangle { anchors.fill: parent; color: "#000000"; opacity: 0.72 }
        MouseArea {   // block anything behind
            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.AllButtons
            onWheel: function(wheel) { wheel.accepted = true }
        }

        Rectangle {
            anchors.centerIn: parent
            width: 560
            height: 440
            color: "#0b0410"
            radius: 12
            border.width: 2
            border.color: Theme.color.mediumorange2

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 22
                spacing: 14

                Text {
                    text: "🐬  SET UP LOTEI"
                    color: Theme.color.lightorange2
                    font.family: "Share Tech Mono"; font.pixelSize: 20; font.bold: true
                }

                StackLayout {
                    currentIndex: setupWizard.step
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    // 0 · welcome
                    ColumnLayout {
                        spacing: 12
                        Text { text: "Meet LOTEI"; color: Theme.color.lightorange2; font.family: "Share Tech Mono"; font.pixelSize: 16; font.bold: true }
                        Text { Layout.fillWidth: true; wrapMode: Text.WordWrap; color: Theme.color.lightorange2; font.family: "Share Tech Mono"; font.pixelSize: 13
                            text: "A snarky, 100% local AI dolphin lives inside your Flipper app. Let's get him set up — about 30 seconds." }
                        Item { Layout.fillHeight: true }
                    }

                    // 1 · name
                    ColumnLayout {
                        spacing: 12
                        Text { text: "1 · Name your Flipper"; color: Theme.color.lightorange2; font.family: "Share Tech Mono"; font.pixelSize: 15; font.bold: true }
                        Text { Layout.fillWidth: true; wrapMode: Text.WordWrap; color: Theme.color.lightorange2; font.family: "Share Tech Mono"; font.pixelSize: 13
                            text: "LOTEI takes your Flipper's name as his own. Type it, or plug in your Flipper and read it." }
                        RowLayout {
                            Layout.fillWidth: true; spacing: 10
                            TextField {
                                id: nameField
                                Layout.fillWidth: true
                                placeholderText: "e.g. Lotei"
                                text: (Backend.deviceState && Backend.deviceState.info && Backend.deviceState.info.name) ? Backend.deviceState.info.name : ""
                            }
                            Button {
                                text: "Read from Flipper"
                                enabled: Backend.deviceState && Backend.deviceState.info && Backend.deviceState.info.name && Backend.deviceState.info.name.length > 0
                                onClicked: nameField.text = Backend.deviceState.info.name
                            }
                        }
                        Item { Layout.fillHeight: true }
                    }

                    // 2 · AI brain
                    ColumnLayout {
                        spacing: 10
                        Text { text: "2 · LOTEI's brain (local AI)"; color: Theme.color.lightorange2; font.family: "Share Tech Mono"; font.pixelSize: 15; font.bold: true }
                        Text {
                            Layout.fillWidth: true; wrapMode: Text.WordWrap
                            font.family: "Share Tech Mono"; font.pixelSize: 13; color: Theme.color.lightorange2
                            text: !Lotei.ollamaOnline
                                  ? "Ollama isn't running. Install it from ollama.com, then run:  ollama pull phi3.5  — then hit re-check. (You can skip and set this up later.)"
                                  : (Lotei.availableModels().length === 0
                                     ? "Ollama's running, but no models yet. Run:  ollama pull phi3.5  — then re-check."
                                     : "Found Ollama. Pick LOTEI's model:")
                        }
                        Flow {
                            Layout.fillWidth: true; spacing: 8
                            Repeater {
                                model: Lotei.ollamaOnline ? Lotei.availableModels() : []
                                delegate: Rectangle {
                                    radius: 5; height: 26; width: mtxt.width + 18
                                    color: modelData === Lotei.modelName ? Theme.color.mediumorange2 : "transparent"
                                    border.width: 1; border.color: Theme.color.mediumorange2
                                    Text { id: mtxt; anchors.centerIn: parent; text: modelData; color: Theme.color.lightorange2; font.family: "Share Tech Mono"; font.pixelSize: 12 }
                                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: Lotei.setModel(modelData) }
                                }
                            }
                        }
                        Text {
                            text: "↻ re-check"
                            color: recheckMouse.containsMouse ? Theme.color.lightorange2 : Theme.color.mediumorange1
                            font.family: "Share Tech Mono"; font.pixelSize: 12
                            MouseArea { id: recheckMouse; anchors.fill: parent; anchors.margins: -4; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: Lotei.recheckOllama() }
                        }
                        Item { Layout.fillHeight: true }
                    }

                    // 3 · personality
                    ColumnLayout {
                        spacing: 10
                        Text { text: "3 · Give LOTEI a personality"; color: Theme.color.lightorange2; font.family: "Share Tech Mono"; font.pixelSize: 15; font.bold: true }
                        Button {
                            text: "🎭 Build one from his name" + (nameField.text.length > 0 ? " (" + nameField.text + ")" : "")
                            onClicked: { Lotei.applyNamePersonality(); personaLabel.text = "→ personality built from the name" }
                        }
                        Text { text: "…or pick a preset:"; color: Theme.color.lightorange2; font.family: "Share Tech Mono"; font.pixelSize: 13 }
                        Flow {
                            Layout.fillWidth: true; spacing: 8
                            Repeater {
                                model: Lotei.personalityPresets()
                                delegate: Rectangle {
                                    radius: 5; height: 26; width: ptxt.width + 18
                                    color: "transparent"; border.width: 1; border.color: Theme.color.mediumorange2
                                    Text { id: ptxt; anchors.centerIn: parent; text: modelData; color: Theme.color.lightorange2; font.family: "Share Tech Mono"; font.pixelSize: 12 }
                                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { Lotei.applyPreset(modelData); personaLabel.text = "→ " + modelData } }
                                }
                            }
                        }
                        Text { id: personaLabel; text: ""; color: Theme.color.lightgreen; font.family: "Share Tech Mono"; font.pixelSize: 12 }

                        Rectangle { Layout.fillWidth: true; height: 1; color: Theme.color.mediumorange2; opacity: 0.4 }
                        CheckBox {
                            id: agentToggle
                            text: "🤖 Agent mode — let LOTEI edit & test his own code"
                            checked: Lotei.agentEnabled
                            onToggled: Lotei.agentEnabled = checked
                            contentItem: Text {
                                text: agentToggle.text; leftPadding: agentToggle.indicator.width + 6
                                color: Theme.color.lightorange2; font.family: "Share Tech Mono"
                                font.pixelSize: 12; verticalAlignment: Text.AlignVCenter
                            }
                        }
                        Button {
                            visible: Lotei.agentEnabled
                            text: Lotei.agentDir.length ? "📁 " + Lotei.agentDir : "📁 Pick workspace folder (your qFlipper source)"
                            onClicked: agentFolderDialog.open()
                        }
                        Text {
                            visible: Lotei.agentEnabled
                            Layout.fillWidth: true; wrapMode: Text.WordWrap
                            text: "⚠️ Lets a local model run shell commands and overwrite files inside that folder only. Keep it under git."
                            color: Theme.color.lightorange3; font.family: "Share Tech Mono"; font.pixelSize: 11
                        }
                        Pf.FolderDialog { id: agentFolderDialog; onAccepted: Lotei.agentDir = folder }

                        Item { Layout.fillHeight: true }
                    }

                    // 4 · done
                    ColumnLayout {
                        spacing: 12
                        Text { text: "🎉  All set!"; color: Theme.color.lightorange2; font.family: "Share Tech Mono"; font.pixelSize: 16; font.bold: true }
                        Text { Layout.fillWidth: true; wrapMode: Text.WordWrap; color: Theme.color.lightorange2; font.family: "Share Tech Mono"; font.pixelSize: 13
                            text: "LOTEI's ready. You can change his model (click it in the header), voice, colors, and more any time." }
                        Item { Layout.fillHeight: true }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12
                    Text {
                        text: "skip"
                        color: skipMouse.containsMouse ? Theme.color.lightorange2 : Theme.color.mediumorange1
                        font.family: "Share Tech Mono"; font.pixelSize: 12
                        MouseArea { id: skipMouse; anchors.fill: parent; anchors.margins: -6; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: Lotei.completeSetup() }
                    }
                    Item { Layout.fillWidth: true }
                    Button {
                        text: "Back"
                        visible: setupWizard.step > 0
                        onClicked: setupWizard.step = setupWizard.step - 1
                    }
                    Button {
                        text: setupWizard.step >= 4 ? "Finish" : "Next"
                        onClicked: {
                            if (setupWizard.step === 1) Lotei.manualName = nameField.text.trim();
                            if (setupWizard.step >= 4) { Lotei.completeSetup(); return; }
                            setupWizard.step = setupWizard.step + 1;
                            if (setupWizard.step === 2) Lotei.recheckOllama();
                        }
                    }
                }
            }
        }
    }

    function askForSelfUpdate() {
        if(App.updateStatus === App.CanUpdate) {
            selfUpdateDialog.open();
        }
    }
}
