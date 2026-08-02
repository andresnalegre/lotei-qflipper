import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import Theme 1.0
import QFlipper 1.0


Item {
    id: container

    property alias backupAction: backupAction
    property alias restoreAction: restoreAction
    property alias eraseAction: eraseAction
    property alias reinstallAction: reinstallAction
    property alias selfUpdateAction: selfUpdateAction

    implicitWidth: 318
    implicitHeight: 344

    readonly property int horizontalPadding: Math.floor((container.implicitWidth - control.implicitWidth) / 2)
    readonly property int verticalPadding: 10

    ColumnLayout {
        id: control
        spacing: 10

        x: horizontalPadding
        y: (container.implicitHeight - control.implicitHeight) / 2

        TransparentLabel {
            color: Theme.color.lightorange2
            text: qsTr("Firmware update channel")
        }

        ComboBox {
            id: channelComboBox

            // The stock model only ever lists the OFFICIAL channels, which on a
            // Flipper running Unleashed or Momentum describes a firmware the
            // device isn't using. Follow whatever is actually installed, and
            // fall back to the official list when that is what's running (or
            // when the firmware can't be identified).
            readonly property bool onFork: Firmware.installedChannels.length > 0 &&
                                           Firmware.installedName !== "Official"

            enabled: onFork
                     ? Firmware.installedChannels.length > 1
                     : (Backend.firmwareUpdateState !== Backend.Unknown &&
                        Backend.firmwareUpdateState !== Backend.Checking &&
                        Backend.firmwareUpdateState !== Backend.ErrorOccured)

            // ChannelDelegate now reads either model shape, so both cases get
            // the same rows -- no second, near-identical delegate to keep in
            // sync with the first.
            delegate: ChannelDelegate {}

            model: onFork ? Firmware.installedChannels : Backend.firmwareUpdateModel
            textRole: onFork ? "" : "name"

            Layout.fillWidth: true

            currentIndex: onFork
                          ? Firmware.installedChannels.indexOf(Firmware.installedChannel)
                          : (Backend.firmwareUpdateState !== Backend.Unknown ? find(Preferences.updateChannel) : -1)

            onActivated: function(index) {
                if(onFork) { Firmware.setInstalledChannel(Firmware.installedChannels[index]); }
                else       { Preferences.updateChannel = textAt(index); }
            }

        }

        TransparentLabel {
            color: Theme.color.lightorange2
            text: qsTr("Backup & Restore")
        }

        GridLayout {
            columns: 2
            rowSpacing: control.spacing
            columnSpacing: control.spacing

            Layout.fillWidth: true

            SmallButton {
                action: backupAction
                Layout.fillWidth: true

                icon.source: "qrc:/assets/gfx/symbolic/backup-symbolic.svg"
                icon.width: 18
                icon.height: 20

            }

            SmallButton {
                action: restoreAction
                Layout.fillWidth: true

                icon.source: "qrc:/assets/gfx/symbolic/restore-symbolic.svg"
                icon.width: 18
                icon.height: 20

            }

            SmallButton {
                action: eraseAction
                Layout.fillWidth: true

                icon.source: "qrc:/assets/gfx/symbolic/trashcan.svg"
                icon.width: 18
                icon.height: 20

            }

            SmallButton {
                action: reinstallAction
                Layout.fillWidth: true

                icon.source: "qrc:/assets/gfx/symbolic/update-symbolic.svg"
                icon.width: 16
                icon.height: 16

            }
        }

        TransparentLabel {
            color: Theme.color.lightorange2
            text: qsTr("Application update")
        }

        Button {
            action: selfUpdateAction
            Layout.fillWidth: true

            icon.source: "qrc:/assets/gfx/symbolic/update-symbolic.svg"
            icon.width: 16
            icon.height: 16

        }

        // Only ever one firmware is staged -- picking another replaces it, so
        // nothing accumulates. This is how that single slot goes back to empty.
        // It stays on screen and simply greys out when there is nothing staged:
        // hiding it meant you could only find it once you no longer needed to.
        Button {
            Layout.fillWidth: true
            action: clearImportAction
            // No icon: with one there, the label sat off to the right of it
            // rather than in the middle of the button.

        }

        Action {
            id: backupAction
            text: qsTr("Backup")
            enabled: Backend.deviceState && !Backend.deviceState.isRecoveryMode
        }

        Action {
            id: restoreAction
            text: qsTr("Restore")
            enabled: Backend.deviceState && !Backend.deviceState.isRecoveryMode
        }

        Action {
            id: eraseAction
            text: qsTr("Format")
            enabled: Backend.deviceState && !Backend.deviceState.isRecoveryMode
        }

        Action {
            id: reinstallAction
            text: qsTr("Reinstall")
            // Was gated on the OFFICIAL channel reporting "no updates", which on
            // a fork almost never happens -- so the button sat disabled. What it
            // really needs is a known source for the build already installed.
            // After a format there is nothing left on the card to repair, so a
            // full install is the only route that rebuilds it.
            enabled: !Lotei.sdFormatted &&
                     (Firmware.installedReady || Backend.firmwareUpdateState === Backend.NoUpdates)
        }

        Action {
            id: clearImportAction
            text: qsTr("Clear")
            enabled: Firmware.hasSelection && !Firmware.busy
            onTriggered: Firmware.clearSelection()
        }

        Action {
            id: selfUpdateAction
            // Held disabled on purpose: there is no release feed for this app
            // yet, so pressing it left the label stuck on "Checking..." waiting
            // for an answer that never comes. The hover text says as much.
            // Restore the two commented lines once releases are published.
            text: qsTr("Check app updates")
            enabled: false
            // text: App.updateStatus === App.Checking ? qsTr("Checking...") :
            //       App.updateStatus === App.NoUpdates && checkTimer.running ? qsTr("No updates") : qsTr("Check app updates")
            // enabled: App.updateStatus !== App.Checking && !checkTimer.running
            onTriggered: App.checkForUpdates()
        }

        Timer {
            id: checkTimer
            interval: 1000

            Component.onCompleted: {
                App.updateStatusChanged.connect(function() {
                    if(App.updateStatus === App.NoUpdates) {
                        start();
                    }
                });
            }
        }
    }
}
