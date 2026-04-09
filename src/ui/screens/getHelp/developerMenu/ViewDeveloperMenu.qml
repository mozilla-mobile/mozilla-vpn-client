/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1
import "qrc:/nebula/utils/MZAssetLookup.js" as MZAssetLookup

MZViewBase {
    objectName: "developerScreen"
    _menuTitle: MZI18n.SettingsDevTitle
    _viewContentData: ColumnLayout {
        id: root
        Layout.preferredWidth: parent.width

        spacing: MZTheme.theme.windowMargin

        MZCheckBoxRow {
            id: developerUnlock

            Layout.fillWidth: true
            Layout.rightMargin: MZTheme.theme.windowMargin
            labelText:  MZI18n.SettingsDevShowOptionTitle
            subLabelText: MZI18n.SettingsDevShowOptionSubtitle
            isChecked: MZSettings.developerUnlock
            onClicked: MZSettings.developerUnlock = !MZSettings.developerUnlock
        }

        MZCheckBoxRow {
            id: checkBoxRowStagingServer

            Layout.fillWidth: true
            Layout.rightMargin: MZTheme.theme.windowMargin
            labelText: MZI18n.SettingsDevUseStagingTitle
            subLabelText: MZI18n.SettingsDevUseStagingSubtitle
            isChecked: MZSettings.stagingServer
            showDivider: false
            onClicked: {
                MZSettings.stagingServer = !MZSettings.stagingServer
            }
        }

        MZTextField {
            id: serverAddressInput

            Layout.fillWidth: true
            Layout.rightMargin: MZTheme.theme.windowMargin * 2
            Layout.leftMargin: MZTheme.theme.windowMargin * 3

            Layout.alignment: Qt.AlignHCenter
            enabled: MZSettings.stagingServer
            _placeholderText: "Staging server address"
            Layout.preferredHeight: MZTheme.theme.rowHeight

            PropertyAnimation on opacity {
                duration: 200
            }

            onTextChanged: text => {
                var serverAddressText = serverAddressInput.text.trim();
                if (serverAddressText.endsWith('/')) {
                  serverAddressText = serverAddressText.slice(0, -1);
                }
                if (MZSettings.stagingServerAddress !== serverAddressText) {
                    MZSettings.stagingServerAddress = serverAddressText;
                }
            }

            Component.onCompleted: {
                serverAddressInput.text = MZSettings.stagingServerAddress;
            }
        }

        function maybeTurnOffStartOnBoot() {
            if (MZSettings.startAtBoot) {
              startOnBootDeactivated.visible = true;
              // This settings change fixes a crash (VPN-7523)
              MZSettings.startAtBoot = false;
            }
        }

        MZContextualAlerts {
            id: restartRequired
            visible: false
            Layout.leftMargin: MZTheme.theme.windowMargin/2

            messages: [
                {
                    type: MZContextualAlert.AlertType.Warning,
                    message: MZI18n.SettingsDevRestartRequired,
                }
            ]

            Connections {
                target: MZSettings
                function onStagingServerAddressChanged() {
                    restartRequired.visible = true;
                }
                function onStagingServerChanged() {
                    restartRequired.visible = true;
                }
            }
        }

        MZContextualAlerts {
            id: startOnBootDeactivated
            visible: false
            Layout.leftMargin: MZTheme.theme.windowMargin/2

            messages: [
                {
                    type: MZContextualAlert.AlertType.Warning,
                    message: "'Connect VPN on startup' setting has been disabled",
                }
            ]

            Connections {
                target: MZSettings
                function onStagingServerAddressChanged() {
                    root.maybeTurnOffStartOnBoot();
                }
                function onStagingServerChanged() {
                    root.maybeTurnOffStartOnBoot();
                }
            }
        }

        MZCheckBoxRow {
            Layout.fillWidth: true
            Layout.topMargin: MZTheme.theme.windowMargin
            Layout.rightMargin: MZTheme.theme.windowMargin

            labelText: "Custom Add-on URL"
            subLabelText: "Load add-ons from an alternative URL address"

            isChecked: MZSettings.addonCustomServer
            showDivider: false
            onClicked: {
                MZSettings.addonCustomServer = !MZSettings.addonCustomServer

                if (MZSettings.addonCustomServer && MZFeatureList.get("addonSignature").isSupported) {
                  reminderForAddonsSignature.visible = true
                }
            }
        }

        MZTextField {
            id: addonCustomServerInput

            Layout.rightMargin: MZTheme.theme.windowMargin * 2
            Layout.leftMargin: MZTheme.theme.windowMargin * 3
            Layout.alignment: Qt.AlignRight
            Layout.fillWidth: true

            enabled: MZSettings.addonCustomServer
            _placeholderText: "Addon Custom Server Address"
            height: 40

            PropertyAnimation on opacity {
                duration: 200
            }

            onTextChanged: text => {
                               var addonAddressText = addonCustomServerInput.text.trim();
                               if (!addonAddressText.endsWith('/')) {
                                 addonAddressText = addonAddressText + '/';
                               }
                               if (MZSettings.addonCustomServerAddress !== addonAddressText) {
                                   MZSettings.addonCustomServerAddress = addonAddressText;
                               }

                               if (addonAddressText.includes("localhost")) {
                                  reminderForLocalhostHttps.visible = addonAddressText.startsWith("https")
                               }
                           }

            Component.onCompleted: {
                addonCustomServerInput.text = MZSettings.addonCustomServerAddress;
            }
        }

        MZContextualAlerts {
            id: reminderForLocalhostHttps
            visible: false
            Layout.leftMargin: MZTheme.theme.windowMargin/2

            messages: [
                {
                    type: MZContextualAlert.AlertType.Warning,
                    message: "Did you mean to use `https` with `localhost`?",
                }
            ]
        }

        MZContextualAlerts {
            id: reminderForAddonsSignature
            visible: false
            Layout.leftMargin: MZTheme.theme.windowMargin/2

            messages: [
                {
                    type: MZContextualAlert.AlertType.Warning,
                    message: "'Addon signature' feature is active, which may break custom addon URL",
                }
            ]
        }

        MZCheckBoxRow {
            id: checkBoxRowProdKeyInStaging

            Layout.rightMargin: MZTheme.theme.windowMargin
            labelText: "Add-on production signature key in staging"
            subLabelText: "Use the add-on production signature key in staging"
            isChecked: MZSettings.addonProdKeyInStaging
            showDivider: false
            onClicked: {
                MZSettings.addonProdKeyInStaging = !MZSettings.addonProdKeyInStaging
            }
        }

        Rectangle {
            id: divider
            Layout.preferredHeight: 1
            Layout.fillWidth: true
            Layout.topMargin: MZTheme.theme.windowMargin / 2
            Layout.leftMargin: MZTheme.theme.windowMargin * 3
            Layout.rightMargin: MZTheme.theme.windowMargin
            color: MZTheme.colors.divider
        }

        Repeater {
            model: ListModel {
                id: devMenu

                ListElement {
                    title: "Feature list"
                    viewQrc: "qrc:/qt/qml/Mozilla/VPN/screens/getHelp/developerMenu/ViewFeatureList.qml"
                }
                ListElement {
                    title: "UI Debugging"
                    viewQrc: "qrc:/qt/qml/Mozilla/VPN/screens/getHelp/developerMenu/ViewUiTesting.qml"
                }
                ListElement {
                    title: "Telemetry Debugging"
                    viewQrc: "qrc:/qt/qml/Mozilla/VPN/screens/getHelp/developerMenu/ViewTelemetryDebugging.qml"
                }
            }

            delegate: MZSettingsItem {
               settingTitle:  title
               imageLeftSrc: MZAssetLookup.getImageSource("MenuIconQuestion")
               imageRightSrc: MZAssetLookup.getImageSource("Chevron")
               imageRightMirror: MZLocalizer.isRightToLeft
               onClicked: getHelpStackView.push(viewQrc)
               Layout.leftMargin: MZTheme.theme.windowMargin / 2
               Layout.rightMargin: MZTheme.theme.windowMargin / 2
            }
        }

        //Need to wrap MZExternalLinkListItem in an item since it is not written to work in a layout
        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: MZTheme.theme.rowHeight

            visible: checkBoxRowStagingServer.isChecked && !restartRequired.visible

            MZExternalLinkListItem {
                id: inspectorLink

                anchors.left: parent.left
                anchors.right: parent.right

                objectName: "openInspector"
                title: "Open Inspector"
                accessibleName: "Open Inspector"
                iconSource:  MZAssetLookup.getImageSource("ExternalLink")
                backgroundColor: MZTheme.colors.clickableRow
                onClicked: {
                    MZUrlOpener.openUrlLabel("inspector");
                }
            }
        }

        MZContextualAlerts {
            id: restartRequiredForButtons
            visible: false

            Layout.topMargin: MZTheme.theme.listSpacing
            Layout.leftMargin: MZTheme.theme.windowMargin/2

            messages: [
                {
                    type: MZContextualAlert.AlertType.Warning,
                    message: MZI18n.SettingsDevRestartRequired,
                }
            ]
        }

        MZButton {
            id: resetMessages

            Layout.topMargin: MZTheme.theme.listSpacing * 2

            text: "Reset messages"
            onClicked: {
                MZAddonManager.reset()
            }
        }

        MZButton {
            id: resetOnboarding

            text: "View onboarding at next launch"
            onClicked: {
                MZSettings.onboardingCompleted = false
                restartRequiredForButtons.visible = true
            }
        }

        MZButton {
            id: crashApp
            property int clickNeeded: 5


            text: "Test Crash Reporter"
            onClicked: {
                if (!MZSettings.stagingServer){
                    text = "Test Crash Reporter (Staging only!)";
                    return;
                }
                if (clickNeeded) {
                    text = "Test Crash Reporter (" + clickNeeded + ")";
                    --clickNeeded;
                    return;
                }
                MZUtils.crashTest()
            }
        }

        MZButton {
            id: silentServerSwitch

            text: "Silent server switch (from app)"
            onClicked: {
                VPN.silentSwitch();
            }
        }

        MZButton {
            id: unstableNetworkExtension
            visible: Qt.platform.os === "ios" || Qt.platform.os === "android"

            text: "Silent server switch (from daemon)"
            onClicked: {
                VPNController.forceDaemonSilentServerSwitch();
            }
        }

        ColumnLayout {
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            Layout.maximumWidth: unstableNetworkExtension.width
            Layout.topMargin: MZTheme.theme.vSpacingSmall

            MZTextBlock {
                Layout.fillWidth: true

                text: MZEnv.devVersion
            }

            MZTextBlock {
                Layout.fillWidth: true

                text: "Installation time: " + MZSettings.installationTime
            }

            MZTextBlock {
                Layout.fillWidth: true

                text: "Update time: " + MZSettings.updateTime
            }
        }
    }
}
