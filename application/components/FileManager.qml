import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.impl 2.15

import Theme 1.0
import QFlipper 1.0

Item {
    id: control

    implicitWidth: 742
    implicitHeight: 344

    property MessageDialog messageDialog
    property ConfirmationDialog confirmationDialog

    // ---- multi-selection model (drag-band, Cmd+click, Cmd+A) ----
    property var selectedList: []                       // selected row indices
    function isSelected(i) { return selectedList.indexOf(i) >= 0 }
    // Clearing the selection has to drop the current item too. The delegate
    // paints at full opacity for EITHER multiSelected OR isCurrent, so emptying
    // selectedList on its own left the last-clicked file looking selected even
    // though nothing was.
    function clearSel() { selectedList = []; fileView.currentIndex = -1 }
    function selectOnly(i) { selectedList = [i] }
    function toggleSel(i) {
        var a = selectedList.slice();
        var p = a.indexOf(i);
        if(p >= 0) { a.splice(p, 1); } else { a.push(i); }
        selectedList = a;
    }
    function selectAll() {
        var a = [];
        var count = Backend.fileManager.rowCount ? Backend.fileManager.rowCount() : fileView.count;
        for(var i = 0; i < fileView.count; ++i) { a.push(i); }
        selectedList = a;
    }
    function selectRange(rect) {                          // rect in fileView content coords
        var cols = Math.max(1, Math.floor(fileView.width / fileView.cellWidth));
        var a = [];
        for(var i = 0; i < fileView.count; ++i) {
            var cx = (i % cols) * fileView.cellWidth;
            var cy = Math.floor(i / cols) * fileView.cellHeight;
            // intersect the cell box with the band
            if(cx < rect.x + rect.w && cx + fileView.cellWidth > rect.x &&
               cy < rect.y + rect.h && cy + fileView.cellHeight > rect.y) {
                a.push(i);
            }
        }
        selectedList = a;
    }

    onVisibleChanged: {
        if(Backend.backendState === ApplicationBackend.Ready) {
            Backend.screenStreamer.isEnabled = !visible
        }
        if(visible) {
            fileView.forceActiveFocus();
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.leftMargin: 7
        anchors.rightMargin: 7

        RowLayout {
            spacing: 7
            Layout.fillWidth: true

            Button {
                id: backButton
                action: backAction

                implicitWidth: 30
                implicitHeight: 30

                padding: 4
            }

            Button {
                id: fwdButton
                action: forwardAction

                implicitWidth: 30
                implicitHeight: 30

                padding: 5
            }

            Rectangle {
                width: 500
                height: 30

                color: "black"
                radius: 6

                border.width: 2
                border.color: Theme.color.mediumorange1

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 7

                    spacing: 5

                    IconImage {
                        Layout.alignment: Qt.AlignVCenter

                        color: Theme.color.lightorange2
                        sourceSize: Qt.size(16, 16)
                        source: {
                            const path = Backend.fileManager.currentPath

                            if(path.startsWith("/ext")) {
                                return "qrc:/assets/gfx/symbolic/filemgr/location-sdcard.svg"
                            } else if(path.startsWith("/int")) {
                                return "qrc:/assets/gfx/symbolic/filemgr/location-internal.svg"
                            } else {
                                return "";
                            }
                        }
                    }

                    Text {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignVCenter

                        font.pixelSize: 18
                        font.family: "Share Tech"

                        color: Theme.color.lightorange2
                        elide: Text.ElideMiddle

                        text: {
                            const path = Backend.fileManager.currentPath

                            if(path.startsWith("/ext")) {
                                return path.replace("/ext", "SD Card") + "/";
                            } else if(path.startsWith("/int")) {
                                return path.replace("/int", "Internal Flash") + "/";
                            } else {
                                return path;
                            }
                        }

                        onTextChanged: {
                            // A new folder starts with nothing selected. Setting
                            // currentIndex to 0 here used to leave the first item
                            // painted as selected before the user touched anything.
                            control.clearSel();
                        }

                    }
                }
            }

            Button {
                id: refreshButton
                action: refreshAction

                implicitWidth: 30
                implicitHeight: 30

                padding: 2
            }

            Switch {
                id: hiddenFileSwitch
                text: qsTr("Hidden files")
                checked: Preferences.showHiddenFiles
                onCheckedChanged: {
                    Preferences.showHiddenFiles = checked;
                    Backend.fileManager.refresh();
                }
            }
        }

        ScrollView {
            id: scrollView

            Layout.fillWidth: true
            Layout.fillHeight: true

            ScrollBar.vertical.anchors.right: scrollView.right
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

            padding: 0
            clip: true

            background: Item {}

            GridView {
                id: fileView
                cellWidth: 120
                cellHeight: 86

                boundsBehavior: Flickable.StopAtBounds

                // Selection overlay: sits above the delegates, owns click/drag/dbl-click.
                MouseArea {
                    id: bandArea
                    z: 100
                    anchors.fill: parent
                    hoverEnabled: false
                    preventStealing: true   // keep the drag from the Flickable so band works (wheel still scrolls)
                    acceptedButtons: Qt.LeftButton | Qt.RightButton | Qt.BackButton | Qt.ForwardButton

                    property real startX: 0
                    property real startY: 0
                    property bool banding: false
                    property bool didBand: false

                    function idxAt(mx, my) {
                        var cx = mx + fileView.contentX;
                        var cy = my + fileView.contentY;
                        var idx = fileView.indexAt(cx, cy);
                        if(idx < 0) { return -1; }
                        // indexAt answers for the whole cell. Once the grid is
                        // full there is no truly empty space left to click, so
                        // the padding around an icon has to count as empty --
                        // otherwise the selection can never be cleared.
                        var item = fileView.itemAtIndex(idx);
                        if(item && item.hitsContent) {
                            var p = fileView.contentItem.mapToItem(item, cx, cy);
                            if(!item.hitsContent(p.x, p.y)) { return -1; }
                        }
                        return idx;
                    }

                    onPressed: function(mouse) {
                        forceActiveFocus(Qt.MouseFocusReason);
                        var idx = idxAt(mouse.x, mouse.y);

                        if(mouse.button === Qt.RightButton) {
                            if(idx >= 0) { mouse.accepted = false; return; }   // let the delegate open its menu
                            return;                                            // empty right-click -> handled onClicked
                        }
                        if(mouse.button !== Qt.LeftButton) { return; }

                        // Just record the start. A rubber-band only begins once the
                        // mouse actually moves (below), so you can start a drag on top
                        // of ANY item -- folders included -- not only on empty space.
                        bandArea.startX = mouse.x; bandArea.startY = mouse.y;
                        bandArea.didBand = false;
                        bandArea.banding = false;
                    }

                    onPositionChanged: function(mouse) {
                        if(!(mouse.buttons & Qt.LeftButton)) { return; }
                        var dx = mouse.x - bandArea.startX;
                        var dy = mouse.y - bandArea.startY;

                        if(!bandArea.banding) {
                            if(Math.abs(dx) > 4 || Math.abs(dy) > 4) {
                                bandArea.banding = true;
                                bandArea.didBand = true;
                                if(!(mouse.modifiers & (Qt.ControlModifier | Qt.MetaModifier))) { control.clearSel(); }
                            } else {
                                return;   // not enough movement yet -> still a click
                            }
                        }

                        var x = Math.min(mouse.x, bandArea.startX);
                        var y = Math.min(mouse.y, bandArea.startY);
                        var w = Math.abs(dx);
                        var h = Math.abs(dy);
                        band.x = x; band.y = y; band.width = w; band.height = h;
                        control.selectRange({ x: x + fileView.contentX, y: y + fileView.contentY, w: w, h: h });
                    }

                    onReleased: function(mouse) { bandArea.banding = false; }

                    onClicked: function(mouse) {
                        if(mouse.button === Qt.BackButton && Backend.fileManager.canGoBack) { Backend.fileManager.historyBack(); return; }
                        if(mouse.button === Qt.ForwardButton && Backend.fileManager.canGoForward) { Backend.fileManager.historyForward(); return; }

                        var idx = idxAt(mouse.x, mouse.y);

                        if(mouse.button === Qt.RightButton) {
                            if(idx < 0 && !Backend.fileManager.isRoot) { emptyMenu.popup(); }
                            return;
                        }
                        if(mouse.button !== Qt.LeftButton) { return; }
                        if(bandArea.didBand) { return; }   // a drag just happened; keep that selection

                        if(idx < 0) {
                            control.clearSel();            // click on empty space clears selection
                        } else if(mouse.modifiers & (Qt.ControlModifier | Qt.MetaModifier)) {
                            control.toggleSel(idx);
                            fileView.currentIndex = idx;
                        } else {
                            control.selectOnly(idx);
                            fileView.currentIndex = idx;
                        }
                    }

                    onDoubleClicked: function(mouse) {
                        if(mouse.button !== Qt.LeftButton) { return; }
                        var idx = idxAt(mouse.x, mouse.y);
                        if(idx < 0) { return; }
                        var name = Backend.fileManager.fileNameAt(idx);
                        if(name.length === 0) { return; }
                        if(Backend.fileManager.isDirectoryAt(idx)) {
                            Backend.fileManager.cd(name);
                        } else {
                            var base = Backend.fileManager.currentPath;
                            var full = (base.charAt(base.length - 1) === "/") ? (base + name) : (base + "/" + name);
                            Lotei.openFileForEdit(full);
                        }
                    }
                }

                // rubber-band selection rectangle (viewport coords)
                Rectangle {
                    id: band
                    z: 99
                    visible: bandArea.banding && (width > 2 || height > 2)
                    color: Color.transparent(Theme.color.lightorange2, 0.12)
                    border.width: 1
                    border.color: Theme.color.mediumorange2
                }

                model: Backend.fileManager
                delegate: FileManagerDelegate {
                    confirmationDialog: control.confirmationDialog
                    fileManager: control
                }
            }
        }
    }

    DropArea {
        enabled: !Backend.fileManager.isRoot
        anchors.fill: parent
        onDropped: function(drop) {
            if(drop.source || !drop.hasUrls || drop.proposedAction !== Qt.CopyAction) {
                const msgObj = {
                    title: qsTr("Error"),
                    message: qsTr("Operation is not supported"),
                    customText: qsTr("Close")
                };

                messageDialog.openWithMessage(null, msgObj);

            } else {
                control.uploadUrls(drop.urls);
                drop.accept()
            }
        }
    }

    Menu {
        id: emptyMenu

        MenuItem { action: uploadHereAction }
        MenuItem { action: newFileAction }
        MenuItem { action: newDirAction }
    }

    Action {
        id: forwardAction
        enabled: Backend.fileManager.canGoForward

        icon.width: 8
        icon.height: 14
        icon.source: "qrc:/assets/gfx/symbolic/arrow-forward-small.svg"

        onTriggered: Backend.fileManager.historyForward()
    }

    Action {
        id: backAction
        enabled: Backend.fileManager.canGoBack

        icon.width: 8
        icon.height: 14
        icon.source: "qrc:/assets/gfx/symbolic/arrow-back-small.svg"

        onTriggered: Backend.fileManager.historyBack()
    }

    Action {
        id: refreshAction

        icon.width: 16
        icon.height: 16
        icon.source: "qrc:/assets/gfx/symbolic/refresh-small.svg"

        onTriggered: Backend.fileManager.refresh()
    }

    Action {
        id: uploadHereAction
        text: qsTr("Upload here...")
        icon.source: "qrc:/assets/gfx/symbolic/filemgr/action-upload.svg"

        onTriggered: beginUpload();
    }

    Action {
        id: newFileAction
        text: qsTr("New File")
        icon.source: "qrc:/assets/gfx/symbolic/filemgr/action-new.svg"
        onTriggered: {
            newFileField.text = "";
            newFilePanel.visible = true;
            newFileField.forceActiveFocus();
        }
    }

    Action {
        id: newDirAction
        text: qsTr("New Folder")
        onTriggered: Backend.fileManager.beginMkDir();
        icon.source: "qrc:/assets/gfx/symbolic/filemgr/action-new.svg"
    }

    Component.onCompleted: {
        Backend.fileManager.currentPathChanged.connect(function() {
            // Covers every way of arriving somewhere: cd, back, forward, refresh
            // and the jump to root. selectedList was never cleared on navigation
            // either, so indices left over from the previous folder came back
            // highlighted on rows that just happened to share the same numbers.
            control.clearSel();
        });
    }

    function uploadUrls(urls) {
        const doUpload = function() {
            Backend.fileManager.upload(urls);
        };

        if(Backend.fileManager.isTooLarge(urls)) {
            const isMultiple = urls.length > 1;
            const msgObj = {
                title: qsTr("Warning"),
                message: qsTr("Selected %1 too large.\nUpload anyway?").arg(isMultiple ? qsTr("files are") : qsTr("file is")),
                suggestedRole: ConfirmationDialog.RejectRole,
                customText: qsTr("Upload")
            };

            confirmationDialog.openWithMessage(doUpload, msgObj);

        } else {
            doUpload();
        }
    }

    function beginUpload() {
        SystemFileDialog.accepted.connect(function() {
            control.uploadUrls(SystemFileDialog.fileUrls);
        });
        SystemFileDialog.beginOpenFiles(SystemFileDialog.LastLocation, [ "All files (*)" ]);
    }

    Keys.onPressed: function(event) {
        switch(event.key) {
        case Qt.Key_A:
            if(event.modifiers & (Qt.ControlModifier | Qt.MetaModifier)) {
                control.selectAll();
                event.accepted = true;
            } else {
                event.accepted = false;
            }
            return;

        case Qt.Key_Escape:
            control.clearSel();
            event.accepted = true;
            return;

        case Qt.Key_Backspace:
            if(Backend.fileManager.canGoBack) {
                Backend.fileManager.historyBack();
            }
            event.accepted = true;
            return;

        case Qt.Key_L:
            if(Backend.fileManager.isRoot) {
                event.accepted = false;
            } else if(event.modifiers & Qt.ControlModifier) {
                beginUpload();
                event.accepted = true;
            } else {
                event.accepted = false;
            }
            return;

        case Qt.Key_N:
            if(Backend.fileManager.isRoot) {
                event.accepted = false;
            } else if(event.modifiers & Qt.ControlModifier) {
                Backend.fileManager.beginMkDir();
                event.accepted = true;
            } else {
                event.accepted = false;
            }
            return;

        case Qt.Key_G:
            if(event.modifiers & Qt.ControlModifier) {
                Backend.fileManager.refresh()
                event.accepted = true;
            } else {
                event.accepted = false;
            }
            return;

        default:
            event.accepted = false;
        }
    }

    // ---- new file panel (create a file with any extension, right in the app) ----
    Rectangle {
        id: newFilePanel
        visible: false
        z: 210
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: 52
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        height: 46
        color: "#0b0410"
        radius: 6
        border.width: 1
        border.color: Theme.color.mediumorange2

        function createFile() {
            var name = newFileField.text.trim();
            if(name.length === 0) { return; }
            var base = Backend.fileManager.currentPath;
            var full = (base.charAt(base.length - 1) === "/") ? (base + name) : (base + "/" + name);
            Lotei.writeFile(full, "");          // create empty file on the Flipper
            newFilePanel.visible = false;
            newFilePanel.pendingOpen = full;    // open it in the editor once written
        }
        property string pendingOpen: ""

        Connections {
            target: Lotei
            function onFileSaved(path) {
                if(newFilePanel.pendingOpen === path) {
                    Backend.fileManager.refresh();   // just show the new file; don't auto-open
                    newFilePanel.pendingOpen = "";
                }
            }
        }

        RowLayout {
            anchors.fill: parent
            anchors.margins: 8
            spacing: 8

            Text {
                text: "New file:"
                color: Theme.color.lightorange2
                font.family: "Share Tech Mono"; font.pixelSize: 13
            }
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 28
                color: "#000000"; radius: 4
                border.width: 1; border.color: Theme.color.mediumorange3
                TextInput {
                    id: newFileField
                    anchors.fill: parent
                    anchors.leftMargin: 8; anchors.rightMargin: 8
                    verticalAlignment: TextInput.AlignVCenter
                    color: Theme.color.lightorange2
                    font.family: "Share Tech Mono"; font.pixelSize: 13
                    clip: true
                    onAccepted: newFilePanel.createFile()
                    Text {
                        anchors.fill: parent
                        verticalAlignment: Text.AlignVCenter
                        visible: newFileField.text.length === 0
                        text: "name.txt / name.nfc / name.sub / name.ir"
                        color: Theme.color.mediumorange1
                        font: newFileField.font
                    }
                }
            }
            Text {
                text: "cancel"
                color: nfCancel.containsMouse ? Theme.color.lightorange2 : Theme.color.mediumorange1
                font.family: "Share Tech Mono"; font.pixelSize: 12
                MouseArea { id: nfCancel; anchors.fill: parent; anchors.margins: -6; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: newFilePanel.visible = false }
            }
            Text {
                text: "create"
                color: nfCreate.containsMouse ? Theme.color.lightgreen : Theme.color.lightorange2
                font.family: "Share Tech Mono"; font.pixelSize: 12; font.bold: true
                MouseArea { id: nfCreate; anchors.fill: parent; anchors.margins: -6; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: newFilePanel.createFile() }
            }
        }
    }

    // ---- in-app file editor (opens over the file grid on double-click) ----
    Rectangle {
        id: editor
        anchors.fill: parent
        anchors.margins: -6
        z: 200
        visible: false
        color: "#0b0410"
        radius: 8
        border.width: 1
        border.color: Theme.color.mediumorange2

        property string filePath: ""
        property string fileName: ""

        Connections {
            target: Lotei
            function onFileOpened(path, content) {
                editor.filePath = path;
                editor.fileName = path.substring(path.lastIndexOf('/') + 1);
                editArea.text = content;
                editStatus.text = "";
                editor.visible = true;
                editArea.forceActiveFocus();
            }
            function onFileSaved(path) {
                editStatus.text = "Saved";
                Backend.fileManager.refresh();
                editorCloseTimer.start();
            }
            function onFileEditError(msg) { editStatus.text = "Error: " + msg; }
        }

        Timer { id: editorCloseTimer; interval: 700; onTriggered: editor.visible = false }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 7

            RowLayout {
                Layout.fillWidth: true
                Text {
                    text: editor.fileName
                    color: Theme.color.lightorange2
                    font.family: "Share Tech Mono"; font.pixelSize: 15; font.bold: true
                    Layout.fillWidth: true
                    elide: Text.ElideMiddle
                }
                Text {
                    id: editStatus
                    color: Theme.color.lightgreen
                    font.family: "Share Tech Mono"; font.pixelSize: 12
                }
            }

            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                TextArea {
                    id: editArea
                    wrapMode: TextArea.NoWrap
                    color: Theme.color.lightgreen
                    selectByMouse: true
                    font.family: "Share Tech Mono"; font.pixelSize: 12
                    background: Rectangle { color: "#000000"; border.width: 1; border.color: Theme.color.mediumorange3; radius: 6 }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }
                Text {
                    text: "cancel"
                    color: cancelMouse.containsMouse ? Theme.color.lightorange2 : Theme.color.mediumorange1
                    font.family: "Share Tech Mono"; font.pixelSize: 13
                    MouseArea { id: cancelMouse; anchors.fill: parent; anchors.margins: -8; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: editor.visible = false }
                }
                Item { width: 26 }
                Text {
                    text: "save"
                    color: saveMouse.containsMouse ? Theme.color.lightgreen : Theme.color.lightorange2
                    font.family: "Share Tech Mono"; font.pixelSize: 13; font.bold: true
                    MouseArea { id: saveMouse; anchors.fill: parent; anchors.margins: -8; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: Lotei.writeFile(editor.filePath, editArea.text) }
                }
                Item { width: 14 }
            }
        }
    }
}
