import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQml.Models 2.15

import Theme 1.0
import QFlipper 1.0

AbstractOverlay {
    id: overlay

    signal selfUpdateRequested

    // qFlipper's own firmwareUpdateState only ever compares the device against
    // the OFFICIAL channel. On a Flipper running Unleashed or Momentum that
    // comparison is meaningless: it reports "Update" and, if pressed, quietly
    // replaces the fork with official firmware. So when a fork is detected the
    // button is driven by the firmware store instead, which knows the right
    // source to compare against.
    readonly property bool onFork: Firmware.installedIndex >= 0 &&
                                   Firmware.installedName !== "Official"

    readonly property int centerX: 590
    readonly property int centerOffset: Math.min(overlay.width - (centerX + systemPathLabel.width + 34), 0)

    onDeviceInfoChanged: tabs.currentIndex = 0;

    TabButton {
        id: developerTab
        icon.source: "qrc:/assets/gfx/symbolic/developer-mode.svg"
        icon.width: 27
        icon.height: 27

        enabled: App.isDeveloperMode
        visible: App.isDeveloperMode

        ToolTip {
            text: qsTr("Developer mode. Use with caution!")
            visible: parent.hovered
        }
    }

    MessageDialog {
        id: messageDialog
        parent: backgroundRect
        radius: backgroundRect.radius
    }

    ConfirmationDialog {
        id: confirmationDialog
        parent: backgroundRect
        radius: backgroundRect.radius
    }

    ProgressDialog {
        id: progressDialog
        parent: backgroundRect
        radius: backgroundRect.radius

        title: qsTr("Please wait")
        text: qsTr("File operation in progress...")

        value: deviceState ? deviceState.progress : -1
        indeterminate: deviceState ? deviceState.progress < 0 : true

        Component.onCompleted: {
            Backend.fileManager.isBusyChanged.connect(function() {
                if(Backend.fileManager.isBusy) open();
                else close();
            });
        }
    }

    ChangelogDialog {
        id: changelogDialog
        parent: backgroundRect
        radius: backgroundRect.radius
    }

    SDWarningDialog {
        id: sdWarningDialog
        parent: backgroundRect
        radius: backgroundRect.radius
        onRejected: Backend.refreshStorageInfo()
        onAccepted: {
            updateButtonFunc();
            result = false;
        }
    }

    TabPane {
        z: 100
        anchors.top: tabs.bottom
        anchors.left: tabs.left
        anchors.topMargin: -2

        currentIndex: tabs.currentIndex
        backgroundColor: Qt.rgba(0, 0, 0, fileManagerTab.checked)

        items: [
            DeviceInfo { id: deviceInfoPane },
            DeviceActions { id: deviceActions },
            FileManager { id: fileManager; messageDialog: messageDialog; confirmationDialog: confirmationDialog; },
            DeveloperActions { id: developerActions }
        ]
    }

    NikitaTalk {
        z: 150
        visible: tabs.currentIndex === 0
    }

    TabBar {
        id: tabs
        z: 101
        x: 28
        y: 18

        layer.enabled: true

        TabButton {
            icon.source: "qrc:/assets/gfx/symbolic/info.svg"
            icon.width: 25
            icon.height: 25

            ToolTip {
                text: qsTr("Device information")
                visible: parent.hovered
            }
        }

        TabButton {
            icon.source: "qrc:/assets/gfx/symbolic/wrench.svg"
            icon.width: 27
            icon.height: 27

            ToolTip {
                text: qsTr("Advanced controls")
                visible: parent.hovered
            }
        }

        TabButton {
            id: fileManagerTab
            enabled: Backend.deviceState && !Backend.deviceState.isRecoveryMode

            icon.source: "qrc:/assets/gfx/symbolic/file-symbolic.svg"
            icon.width: 23
            icon.height: 29

            onCheckedChanged: if(checked) Backend.fileManager.refresh()

            ToolTip {
                text: qsTr("File manager")
                visible: parent.hovered
            }
        }
    }

    TextLabel {
        id: nameLabel
        x: centerX + centerOffset - width - 38
        y: 31

        color: Theme.color.lightorange2

        font.family: "Born2bSportyV2"
        font.pixelSize: 48

        capitalized: false
        text: deviceInfo ? deviceInfo.name : text
    }

    ColumnLayout {
        x: centerX + centerOffset - 22
        anchors.bottom: nameLabel.baseline
        spacing: 6

        TransparentLabel {
            id: connectionLabel
            height: 14

            icon.source: "qrc:/assets/gfx/symbolic/usb-connected.svg"
            icon.width: 18
            icon.height: 10

            color: (!deviceState || !deviceState.isOnline) ? Theme.color.lightred1 : deviceState.isRecoveryMode ?
                                           Theme.color.lightblue : Theme.color.lightgreen
            text: (!deviceState || !deviceState.isOnline) ? qsTr("Disconnected") : deviceState.isRecoveryMode ?
                                           qsTr("Recovery mode") : qsTr("Connected")
        }

        TransparentLabel {
            id: systemPathLabel
            height: connectionLabel.height
            color: connectionLabel.color
            text: deviceInfo ? deviceInfo.systemLocation : text
            capitalized: false

            TextEdit {
                id: systemPathLabelTextCopy
                text: parent.text
                visible: false
            }

            MouseArea {
                anchors.fill: parent
                onDoubleClicked: {
                    systemPathLabelTextCopy.selectAll();
                    systemPathLabelTextCopy.copy();
                }
            }
        }
    }

    MainButton {
        id: updateButton
        action: updateButtonAction

        x: Math.round(centerX - width / 2)
        y: 277

        accent: {
            if(Firmware.hasSelection) return MainButton.Default;
            if(overlay.onFork) {
                return Firmware.updateAvailable ? MainButton.Green : MainButton.Default;
            }
            switch(Backend.firmwareUpdateState) {
            case ApplicationBackend.CanRepair:
                return MainButton.Blue;
            case ApplicationBackend.CanUpdate:
                return MainButton.Green;
            default:
                return MainButton.Default;
            }
        }

        icon.source: Backend.firmwareUpdateState === ApplicationBackend.ErrorOccured ? "qrc:/assets/gfx/symbolic/update-symbolic.svg" : ""
        icon.width: 32
        icon.height: 32

        ToolTip {
            id: installTip
            text: {
                if(Firmware.hasSelection) {
                    return qsTr("Install %1 %2 imported from the firmware store")
                           .arg(Firmware.selectedName).arg(Firmware.selectedVersion);
                }
                if(Nikita.sdFormatted) {
                    return qsTr("The card was formatted. Import a firmware from Custom firmware to put it back, then Restore your files.");
                }
                if(overlay.onFork) {
                    if(!Firmware.installedReady) {
                        return qsTr("Couldn't reach the %1 release feed, so it isn't known whether a newer build exists. Press to try again.")
                               .arg(Firmware.installedName);
                    }
                    if(Firmware.channelSwitchPending && Firmware.installedLatest === Firmware.deviceVersion) {
                        return qsTr("Move %1 from the %2 channel to %3. Same build, different channel.")
                               .arg(Firmware.installedName)
                               .arg(Firmware.installedFromChannel)
                               .arg(Firmware.installedChannel);
                    }
                    return Firmware.updateAvailable
                           ? qsTr("Update %1 to %2 (%3 channel)")
                             .arg(Firmware.installedName).arg(Firmware.installedLatest).arg(Firmware.installedChannel)
                           : qsTr("%1 is already the newest build. Use Custom firmware to switch to a different one.")
                             .arg(Firmware.installedName);
                }
                switch(Backend.firmwareUpdateState) {
                case ApplicationBackend.CanRepair:
                    return qsTr("Repair a broken firmware installation. May erase your progress and settings.");
                case ApplicationBackend.CanUpdate:
                    return qsTr("Update Flipper to the latest version");
                case ApplicationBackend.CanInstall:
                    return qsTr("Install firmware from currently selected update channel");
                case ApplicationBackend.ErrorOccured:
                    return qsTr("Press to check internet connection and try to update Flipper again");
                default:
                    return "";
                }
            }

            implicitWidth: 300
            x: Math.round((parent.width - width) / 2)
            y: parent.height + 6
            visible: parent.hovered && text.length !== 0 && !Firmware.open && !Cli.open

            contentItem: Text {
                text: installTip.text
                color: Theme.color.lightorange2
                font: installTip.font
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WordWrap
            }
        }
    }

    LinkButton {
        id: releaseButton
        action: changelogAction
        x: centerX - width - 6

        anchors.top: updateButton.bottom
        anchors.topMargin: 5

        linkColor: {
            if(Firmware.deviceVersion.length > 0) {
                return Theme.color.lightgreen;      // this is what you are running
            } else if(Preferences.updateChannel === "development") {
                return Theme.color.lightred2;
            } else if(Preferences.updateChannel === "release-candidate") {
                return "blueviolet";
            } else if(Preferences.updateChannel === "release") {
                return Theme.color.lightgreen;
            } else {
                return Theme.color.lightorange2;
            }
        }

        visible: Firmware.deviceVersion.length > 0 ||
                 (Backend.firmwareUpdateState !== ApplicationBackend.Unknown &&
                  Backend.firmwareUpdateState !== ApplicationBackend.Checking &&
                  Backend.firmwareUpdateState !== ApplicationBackend.ErrorOccured)
    }

    LinkButton {
        id: fileButton
        x: centerX + 6

        anchors.top: updateButton.bottom
        anchors.topMargin: 5

        action: installFromFileAction
    }

    LinkButton {
        id: customFwButton
        x: Math.round(centerX - width / 2)

        anchors.top: fileButton.bottom
        anchors.topMargin: -6

        linkColor: Theme.color.lightorange2
        action: customFirmwareAction
    }

    Action {
        id: updateButtonAction

        enabled: Firmware.hasSelection
                 ? !Firmware.busy
                 : Nikita.sdFormatted
                 ? false
                 : overlay.onFork
                 ? ((Firmware.updateAvailable || !Firmware.installedReady) && !Firmware.busy)
                 : (Backend.firmwareUpdateState === ApplicationBackend.CanUpdate ||
                    Backend.firmwareUpdateState === ApplicationBackend.CanInstall ||
                    Backend.firmwareUpdateState === ApplicationBackend.CanRepair ||
                    Backend.firmwareUpdateState === ApplicationBackend.ErrorOccured)

        text: {
            // A firmware imported from the store is an explicit choice, so it
            // outranks any automatic update offer.
            if(Firmware.hasSelection) return qsTr("Install");

            // After a format the card has none of the resources the firmware
            // expects, so what is on the device is not something to update or
            // repair; it needs a full install. Importing one from Custom
            // firmware is the way back, and that is the branch just above.
            if(Nikita.sdFormatted) return qsTr("No data");

            // On a fork the only honest offer is "update to a newer build of
            // the SAME firmware". Installing something else stays behind the
            // two explicit doors below: Custom firmware and Install from file.
            if(overlay.onFork) {
                if(Firmware.busy)              return qsTr("Checking...");
                // The source couldn't be reached, so whether a newer build
                // exists is unknown. Offer the retry instead of a dead label.
                if(!Firmware.installedReady)   return qsTr("Check");
                // Same build, different channel: that is a switch, not an
                // update, and saying "update" for it would be a lie.
                if(Firmware.channelSwitchPending &&
                   Firmware.installedLatest === Firmware.deviceVersion) {
                    return qsTr("Switch to %1").arg(Firmware.installedChannel);
                }
                if(Firmware.updateAvailable)   return qsTr("Update");
                return qsTr("Up to date");
            }

            switch(Backend.firmwareUpdateState) {
            case Backend.Unknown:
                return qsTr("No data");
            case Backend.Checking:
                return qsTr("Checking...");
            case Backend.CanRepair:
                return qsTr("Repair");
            case Backend.CanUpdate:
                return qsTr("Update");
            case Backend.CanInstall:
                return qsTr("Install");
            case Backend.NoUpdates:
                return qsTr("No updates");
            case Backend.ErrorOccured:
                return qsTr("Try again");
            }
        }

        onTriggered: Backend.firmwareUpdateState !== ApplicationBackend.ErrorOccured ?
                     updateButtonFunc() : Backend.checkFirmwareUpdates()
    }

    Action {
        id: changelogAction
        enabled: Firmware.deviceVersion.length > 0 ||
                 (Backend.firmwareUpdateState !== ApplicationBackend.Unknown &&
                  Backend.firmwareUpdateState !== ApplicationBackend.Checking &&
                  Backend.firmwareUpdateState !== ApplicationBackend.ErrorOccured)

        text: {
            // What is actually on the Flipper. This used to show the official
            // channel's newest release, which on a fork names a version the
            // device has never run.
            if(Firmware.deviceVersion.length > 0) {
                return Firmware.deviceVersion;
            }

            let str;

            if(!enabled) {
                return qsTr("No data");
            } else if(Preferences.updateChannel === "development") {
                str = "Dev";
            } else if(Preferences.updateChannel === "release-candidate") {
                str = "RC";
            } else if(Preferences.updateChannel === "release") {
                str = "Release";
            } else {
                str = "Unknown";
            }

            return "%1 %2".arg(str).arg(Backend.latestFirmwareVersion.number.split("-")[0]);
        }

        onTriggered: changelogDialog.open()
    }

    Action {
       id: installFromFileAction
       text: qsTr("Install from file")
       onTriggered: installFromFile()
    }

    Action {
       id: customFirmwareAction
       // Once something is staged, this line is what shows it on the main
       // screen, and reopening the panel is how it gets changed or dropped.
       // Name only. The link is centred with no width limit, so a version
       // string like "RM0722-1811-ff9f4feb" ran off the panel, and a line
       // that changes width with the pick reads as inconsistent anyway. The
       // exact version is in the panel and in the confirmation before flashing.
       text: Firmware.hasSelection ? Firmware.selectedName : qsTr("Custom firmware")
       onTriggered: Firmware.open = true
    }

    function updateButtonFunc() {
        // Nothing known about the running fork yet: this press is a retry,
        // not an install.
        if(overlay.onFork && !Firmware.hasSelection && !Firmware.installedReady) {
            Firmware.refresh();
            return;
        }

        // Something was imported from the store: flash exactly that.
        if(Firmware.hasSelection) {
            const pickObj = {
                title : qsTr("Install firmware?"),
                customText: qsTr("Install"),
                message: qsTr("%1 <font color=\"%2\">%3</font><br/>will be installed")
                         .arg(Firmware.selectedName)
                         .arg(Theme.color.lightgreen)
                         .arg(Firmware.selectedVersion)
            };

            const pickCanGo = deviceInfo.storage.isExternalPresent ||
                              deviceState.isRecoveryMode ||
                              sdWarningDialog.result;
            if(pickCanGo) {
                confirmationDialog.openWithMessage(function() { Firmware.installSelected(); }, pickObj);
            } else {
                sdWarningDialog.open();
            }
            return;
        }

        // A fork updates from its own source, not from the official channel.
        if(overlay.onFork) {
            const forkObj = {
                title : qsTr("Update firmware?"),
                customText: qsTr("Update"),
                message: qsTr("%1 <font color=\"%2\">%3</font><br/>from the %4 channel will be installed")
                         .arg(Firmware.installedName)
                         .arg(Theme.color.lightgreen)
                         .arg(Firmware.installedLatest)
                         .arg(Firmware.installedChannel)
            };

            const forkCanUpdate = deviceInfo.storage.isExternalPresent ||
                                  deviceState.isRecoveryMode ||
                                  sdWarningDialog.result;
            if(forkCanUpdate) {
                confirmationDialog.openWithMessage(function() {
                    Firmware.install(Firmware.installedIndex);
                }, forkObj);
            } else {
                sdWarningDialog.open();
            }
            return;
        }

        const channelName = Preferences.updateChannel;
        const messageObj = deviceState.isRecoveryMode ? {
                title : qsTr("Repair Device?"),
                customText: qsTr("Repair"),
                message : qsTr("Firmware <font color=\"%1\">%2</font><br/>will be installed")
                          .arg(releaseButton.linkColor)
                          .arg(releaseButton.text)
            } : {
                title : qsTr("Update firmware?"),
                customText: qsTr("Update"),
                message: qsTr("New firmware <font color=\"%1\">%2</font><br/>will be installed")
                         .arg(releaseButton.linkColor)
                         .arg(releaseButton.text),
            };

        const canUpdate = deviceInfo.storage.isExternalPresent ||
                          deviceState.isRecoveryMode ||
                          sdWarningDialog.result;
        if(canUpdate) {
            confirmationDialog.openWithMessage(Backend.mainAction, messageObj);
        } else {
            sdWarningDialog.open();
        }
    }

    function installFromFile() {
        SystemFileDialog.accepted.connect(function() {
            const messageObj = {
                title : qsTr("Install from file?"),
                customText: qsTr("Install"),
                message: qsTr("Firmware from file %1<br/>will be installed").arg(baseName(SystemFileDialog.fileUrl))
            };

            const actionFunc = function() {
                Backend.installFirmware(SystemFileDialog.fileUrl);
            }

            confirmationDialog.openWithMessage(actionFunc, messageObj);
        });

        // Same list as before, minus "All files (*.*)": that entry let a
        // backup .tgz through, which has no update.fuf inside and only fails
        // after being uploaded to the card. Kept as two entries on purpose:
        // with a single filter macOS hides the type selector entirely and the
        // expected extension stops being visible.
        const nameFilters = (deviceState.isRecoveryMode ? [] : ["Firmware bundle files (*.tgz)"]).concat(["Firmware files (*.dfu)"]);
        SystemFileDialog.beginOpenFile(SystemFileDialog.LastLocation, nameFilters);
    }

    // Backs up the SD card: the captures, scripts, dumps and folders the user
    // made. The stock action saved /int instead (pairing keys, dolphin state,
    // region), which is settings: useful, but not what is lost when a card dies
    // and not what anyone means by "my files".
    function backupDevice() {
        // Reconnect each time: the handler closes over nothing, but leaving old
        // connections around would fire one confirmation per cancelled attempt.
        SystemFileDialog.accepted.disconnect(onBackupAccepted);
        SystemFileDialog.accepted.connect(onBackupAccepted);

        const defaultName = "%1-backup-%2.tgz".arg(deviceInfo.name)
                            .arg(Qt.formatDateTime(new Date(), "yyyyMMdd-hhmmss"));
        SystemFileDialog.beginSaveFile(SystemFileDialog.LastLocation,
                                       ["Flipper backup (*.tgz)"], defaultName);
    }

    // Fires once per accepted pick, then unhooks itself so a later Backup does
    // not stack handlers.
    function onBackupAccepted() {
        SystemFileDialog.accepted.disconnect(onBackupAccepted);

        const target = SystemFileDialog.fileUrl;
        const messageObj = {
            title : qsTr("Let's Back Up!"),
            customText: qsTr("Backup"),
            message: qsTr("Your files will be saved to:<br/>"
                        + "<font color=\"%1\">%2</font><br/><br/>"
                        + "Firmware, apps, and updates won't be saved.")
                     .arg(Theme.color.lightgreen).arg(baseName(target))
        };

        confirmationDialog.openWithMessage(function() { Nikita.backupSdCard(target); }, messageObj);
    }

    // No result dialogs here: the operation already asked for confirmation
    // before starting, and it reports its outcome on the finish screen, the
    // same one a firmware install uses. A second popup was asking the user to
    // approve something they had already approved.
    Connections {
        target: Nikita

        function onFormatFinished() {
            confirmationDialog.openWithMessage(function() { Backend.factoryReset(); }, {
                title : qsTr("Card emptied"),
                customText: qsTr("Reset settings"),
                message: qsTr("The SD card is now empty.<br/><br/>Reset the device settings as well?")
            });
        }
    }



    // The mirror of backupDevice(): puts a backup folder back onto /ext. The
    // stock action restored a /int tarball, which is settings; it could never
    // bring back a capture or a script.
    function restoreDevice() {
        SystemFileDialog.accepted.connect(function() {
            const folder = SystemFileDialog.fileUrl;
            const messageObj = {
                title : qsTr("Restore your files?"),
                customText: qsTr("Restore"),
                message: qsTr("Contents of<br/><font color=\"%1\">%2</font><br/>will be written to the SD card.<br/><br/>"
                            + "Anything with the same name is overwritten; nothing else on the card is touched.")
                         .arg(Theme.color.lightgreen).arg(baseName(folder))
            };

            confirmationDialog.openWithMessage(function() { Nikita.restoreSdCard(folder); }, messageObj);
        });

        // Picks the archive. A folder from an older backup still restores --
        // restoreSdCard() takes either, but this matches what Backup makes now.
        SystemFileDialog.beginOpenFile(SystemFileDialog.LastLocation,
                                       ["Flipper backup (*.tgz)", "All files (*.*)"]);
    }

    // Empties the card first; the /int reset is offered afterwards, from
    // onFormatFinished(). The two live in different storage and there is no
    // single RPC that clears both.
    function formatDevice() {
        const messageObj = {
            title : qsTr("Format Your Flipper?"),
            message: qsTr("<font color=\"%1\">Everything on the SD card will be deleted:</font><br/>"
                        + "Captures, scripts, dumps, and apps.<br/><br/>This can't be undone. Back up first if needed.")
                     .arg(Theme.color.lightred1),
            suggestedRole: ConfirmationDialog.RejectRole,
            customText: qsTr("Format")
        };

        confirmationDialog.openWithMessage(function() { Nikita.formatSdCard(); }, messageObj);
    }

    function reinstallFirmware() {
        // Backend.mainAction installs the OFFICIAL channel. On a fork that is
        // not a reinstall at all; it swaps the firmware for a different one.
        // When the running build has a known source, re-flash from there.
        const viaStore = Firmware.installedReady && Firmware.installedName !== "Official";

        const messageObj = {
            title : qsTr("Reinstall firmware?"),
            customText: qsTr("Reinstall"),
            message: viaStore
                     ? qsTr("%1 <font color=\"%2\">%3</font><br/>will be flashed over itself")
                       .arg(Firmware.installedName).arg(Theme.color.lightgreen).arg(Firmware.installedLatest)
                     : qsTr("Current firmware version will be reinstalled")
        };

        const canReinstall = deviceInfo.storage.isExternalPresent ||
                             deviceState.isRecoveryMode ||
                             sdWarningDialog.result;
        if(canReinstall) {
            confirmationDialog.openWithMessage(
                viaStore ? function() { Firmware.reinstallInstalled(); } : Backend.mainAction,
                messageObj);
        } else {
            sdWarningDialog.open();
        }
    }

    function installWirelessStack() {
        SystemFileDialog.accepted.connect(function() {
            const messageObj = {
                title : qsTr("Install wireless stack?"),
                customText: qsTr("Install"),
                suggestedRole: ConfirmationDialog.RejectRole,
                message: qsTr("WARNING! This operaton can break your Flipper!")
            };

            const actionFunc = function() {
                Backend.installWirelessStack(SystemFileDialog.fileUrl);
            }

            confirmationDialog.openWithMessage(actionFunc, messageObj);
        });

        SystemFileDialog.beginOpenFile(SystemFileDialog.LastLocation, ["Firmware files (*.bin)", "All files (*.*)"]);
    }

    function installFUSDangerDanger() {
        SystemFileDialog.accepted.connect(function() {
            const messageObj = {
                title : qsTr("Install FUS?"),
                customText: qsTr("Install"),
                suggestedRole: ConfirmationDialog.RejectRole,
                message: qsTr("LAST WARNING! This will invalidate your encryption keys! Please reconsider.")
            };

            const actionFunc = function() {
                Backend.installFUS(SystemFileDialog.fileUrl, 0x080ec00);
            }

            confirmationDialog.openWithMessage(actionFunc, messageObj);
        });

        SystemFileDialog.beginOpenFile(SystemFileDialog.LastLocation, ["Firmware files (*.bin)", "All files (*.*)"]);
    }

    function baseName(fileUrl) {
        const str = fileUrl.toString();
        return str.slice(str.lastIndexOf("/") + 1);
    }

    Component.onCompleted: {
        deviceActions.backupAction.triggered.connect(backupDevice);
        deviceActions.restoreAction.triggered.connect(restoreDevice);
        deviceActions.formatAction.triggered.connect(formatDevice);
        deviceActions.reinstallAction.triggered.connect(reinstallFirmware);
        deviceActions.selfUpdateAction.triggered.connect(selfUpdateRequested);
        developerActions.installRadioAction.triggered.connect(installWirelessStack);
        developerActions.installFusAction.triggered.connect(installFUSDangerDanger);

        if(App.isDeveloperMode) {
            tabs.addItem(developerTab);
        }

        // Close dialog windows when Flipper was PIN locked/disconnected
        Backend.currentDeviceChanged.connect(function() {
            if(messageDialog.visible) {
                messageDialog.close();
            }

            if(confirmationDialog.visible) {
                confirmationDialog.close();
            }

            if(sdWarningDialog.visible) {
                sdWarningDialog.close();
            }
        });
    }
}
