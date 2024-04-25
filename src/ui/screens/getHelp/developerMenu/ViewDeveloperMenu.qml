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
                if (MZSettings.stagingServerAddress !== serverAddressInput.text) {
                    MZSettings.stagingServerAddress = serverAddressInput.text;
                }
            }

            Component.onCompleted: {
                serverAddressInput.text = MZSettings.stagingServerAddress;
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
                               if (MZSettings.addonCustomServerAddress !== addonCustomServerInput.text) {
                                   MZSettings.addonCustomServerAddress = addonCustomServerInput.text;
                               }
                           }

            Component.onCompleted: {
                addonCustomServerInput.text = MZSettings.addonCustomServerAddress;
            }
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
            color: "#E7E7E7"
        }

        Repeater {
            model: ListModel {
                id: devMenu

                ListElement {
                    title: "Feature list"
                    viewQrc: "qrc:/ui/screens/getHelp/developerMenu/ViewFeatureList.qml"
                }
                ListElement {
                    title: "Theme list"
                    viewQrc: "qrc:/ui/screens/getHelp/developerMenu/ViewThemeList.qml"
                }
                ListElement {
                    title: "Animations playground"
                    viewQrc: "qrc:/ui/screens/getHelp/developerMenu/ViewAnimationsPlayground.qml"
                }
                ListElement {
                    title: "UI Testing"
                    viewQrc: "qrc:/ui/screens/getHelp/developerMenu/ViewUiTesting.qml"
                }
                ListElement {
                    title: "Telemetry Debugging"
                    viewQrc: "qrc:/ui/screens/getHelp/developerMenu/ViewTelemetryDebugging.qml"
                }
            }

            delegate: MZSettingsItem {
               settingTitle:  title
               imageLeftSrc: "qrc:/ui/resources/settings/questionMark.svg"
               imageRightSrc: "qrc:/nebula/resources/chevron.svg"
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

            visible: checkBoxRowStagingServer.isChecked && !restartRequired.isVisible

            MZExternalLinkListItem {
                id: inspectorLink

                anchors.left: parent.left
                anchors.right: parent.right

                objectName: "openInspector"
                title: "Open Inspector"
                accessibleName: "Open Inspector"
                iconSource:  "qrc:/nebula/resources/externalLink.svg"
                backgroundColor: MZTheme.theme.clickableRowBlue
                onClicked: {
                    MZUrlOpener.openUrlLabel("inspector");
                }
            }
        }

        MZContextualAlerts {
            id: restartRequired

            property bool isVisible: false

            Layout.topMargin: MZTheme.theme.listSpacing
            Layout.leftMargin: MZTheme.theme.windowMargin/2

            messages: [
                {
                    type: "warning",
                    message: MZI18n.SettingsDevRestartRequired,
                    visible: isVisible
                }
            ]

            Connections {
                target: MZSettings
                function onStagingServerAddressChanged() {
                    restartRequired.isVisible = true;
                }
                function onStagingServerChanged() {
                    restartRequired.isVisible = true;
                }
            }
        }

        MZButton {
            id: reinstateMessages

            Layout.topMargin: MZTheme.theme.listSpacing * 2

            text: "Reinstate messages"
            onClicked: {
                MZAddonManager.reinstateMessages()
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
            visible: Qt.platform.os === "ios"

            text: "iOS daemon: Silent server switch"
            onClicked: {
                VPNController.forceDaemonSilentServerSwitch();
            }
        }

        MZButton {
            id: resetAndQuit
            objectName: "resetAndQuitButton"
            property int clickNeeded: 5

            text: "Reset and Quit"
            onClicked: {
                if (clickNeeded) {
                    text = "Reset and Quit (" + clickNeeded + ")";
                    --clickNeeded;
                    return;
                }

                VPN.hardResetAndQuit()
            }
        }

        ColumnLayout {
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            Layout.maximumWidth: resetAndQuit.width
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
