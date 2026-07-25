import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

import Theme 1.0

RowLayout {
    id: control

    enum Style {
        Windows,
        MacOS,
        Linux
    }

    signal minimizeRequested
    signal closeRequested

    property bool closeEnabled: true
    property string controlPath

    readonly property int style: {
        // TODO: additional Linux style?
        if(Qt.platform.os === "osx") {
            return WindowControls.Style.MacOS
        } else {
            return WindowControls.Style.Windows
        }
    }

    readonly property string iconPath: {
        const p = controlPath + "/%1";

        switch(style) {
        case WindowControls.Style.MacOS:
           return p.arg("mac");

        case WindowControls.Style.Windows: default:
           return p.arg("windows");
        }
    }

    layoutDirection: {
        switch(style) {
        case WindowControls.Style.MacOS:
            return Qt.RightToLeft;

        case WindowControls.Style.Windows: default:
            return Qt.LeftToRight;
        }
    }

    spacing: {
        switch(style) {
        case WindowControls.Style.MacOS:
           return 8;

        case WindowControls.Style.Windows: default:
           return 12;
        }
    }

    DragHandler {
        target: null
        onActiveChanged: if(active) control.Window.window.startSystemMove();
    }

    Item {
        id: spacer
        Layout.fillWidth: true
    }

    // Minimize ( - )
    Rectangle {
        id: minimizeButton
        Layout.preferredWidth: 22
        Layout.preferredHeight: 20
        radius: 4
        border.width: 1
        border.color: Theme.color.mediumorange2
        color: minMouse.containsMouse ? Qt.rgba(1, 1, 1, 0.12) : "transparent"

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
            onClicked: minimizeRequested()
        }
    }

    // Close ( X )
    Rectangle {
        id: closeButton
        enabled: control.closeEnabled
        opacity: enabled ? 1 : 0.35
        Layout.preferredWidth: 22
        Layout.preferredHeight: 20
        radius: 4
        border.width: 1
        border.color: Theme.color.mediumorange2
        color: closeMouse.containsMouse ? Qt.rgba(1, 1, 1, 0.12) : "transparent"

        Text {
            anchors.centerIn: parent
            text: "✕"
            color: Theme.color.lightorange2
            font.family: "Share Tech Mono"; font.pixelSize: 14; font.bold: true
        }
        MouseArea {
            id: closeMouse
            anchors.fill: parent
            enabled: control.closeEnabled
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: closeRequested()
        }
    }
}
