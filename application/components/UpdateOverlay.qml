import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import Theme 1.0
import QFlipper 1.0

AbstractOverlay {
    id: overlay

    // Shared by the title and the bar here, and mirrored on the device widget
    // in MainWindow, so the three keep their spacing while the group centres.
    readonly property int contentShift: 26

    TextLabel {
        id: updateLabel
        capitalized: false
        anchors.horizontalCenter: parent.horizontalCenter
        // The whole group sat high: 19px of air above the title against ~72
        // below the status line, in a 430px container. Everything here and the
        // device widget move down by the same 26px so the two margins match.
        y: 19 + overlay.contentShift

        color: Theme.color.lightorange2

        font.family: "Born2bSportyV2"
        font.pixelSize: 48

        text: {
            switch(Backend.backendState) {
            case Backend.UpdatingDevice:
                return qsTr("Updating your Flipper");
            case Backend.RepairingDevice:
                return qsTr("Repairing your Flipper");
            case Backend.CreatingBackup:
                return qsTr("Creating Backup");
            case Backend.RestoringBackup:
                return qsTr("Restoring Backup");
            case Backend.FactoryResetting:
                return qsTr("Performing Factory Reset");
            case Backend.InstallingFirmware:
                return qsTr("Installing Firmware");
            case Backend.InstallingWirelessStack:
                return qsTr("Installing Wireless Firmware");
            case Backend.InstallingFUS:
                return qsTr("Installing FUS Firmware");
            default:
                return text;
            }
        }
    }

    ProgressBar {
        id: progressBar

        width: 280
        height: 56

        from: 0
        to: 100

        // Anchored rather than positioned by arithmetic. The label above and
        // the one below both use anchors.horizontalCenter; Math.round() here
        // snapped to a whole pixel while they centre exactly, so on an odd
        // parent width the bar sat half a pixel off from both of them.
        anchors.horizontalCenter: parent.horizontalCenter
        y: 270 + overlay.contentShift

        value: deviceState ? deviceState.progress : 0
        indeterminate: !deviceState ? true : deviceState.progress < 0
    }

    TextLabel {
        id: messageLabel
        anchors.top: progressBar.bottom
        anchors.topMargin: 20
        // Centred on the bar, not on the overlay: the two belong together, and
        // tying them to the same reference means they cannot drift apart.
        anchors.horizontalCenter: progressBar.horizontalCenter
        horizontalAlignment: Text.AlignHCenter
        text: !deviceState ? text : deviceState.isError ? deviceState.errorString : deviceState.statusString
        color: Theme.color.lightorange2
    }

    MouseArea {
        x: 620
        y: 120

        hoverEnabled: true

        width: layout.implicitWidth
        height: layout.implicitHeight

        opacity: Backend.backendState === ApplicationBackend.UpdatingDevice ? !!deviceInfo && !deviceInfo.storage.isExternalPresent :
                 Backend.backendState === ApplicationBackend.RepairingDevice ? !!deviceInfo && !deviceState.isRecoveryMode && !deviceInfo.storage.isExternalPresent : 0

        enabled: opacity > 0

        ColumnLayout {
            id: layout

            Image {
                source: "qrc:/assets/gfx/images/warning-sdcard.svg"
                sourceSize: Qt.size(44, 58)
                Layout.alignment: Qt.AlignHCenter
            }

            TextLabel {
                text: qsTr("No SD")
                Layout.alignment: Qt.AlignHCenter
            }
        }

        ToolTip {
            implicitWidth: 250
            text: qsTr("SD Card is not installed. Some functionality will not be available.")
            visible: parent.containsMouse
        }

        Behavior on opacity {
            PropertyAnimation {
                duration: 150
                easing.type: Easing.InOutQuad
            }
        }
    }

}
