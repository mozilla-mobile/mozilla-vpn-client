/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1

VPNViewBase {
    _menuTitle: VPNI18n.SettingsDevTitle
    _viewContentData: ColumnLayout {
        id: root
        Layout.fillWidth: true

        spacing: VPNTheme.theme.windowMargin

        VPNCheckBoxRow {
            id: developerUnlock

            Layout.fillWidth: true
            Layout.rightMargin: VPNTheme.theme.windowMargin
            labelText:  VPNI18n.SettingsDevShowOptionTitle
            subLabelText: VPNI18n.SettingsDevShowOptionSubtitle
            isChecked: VPNSettings.developerUnlock
            onClicked: VPNSettings.developerUnlock = !VPNSettings.developerUnlock
        }

        VPNCheckBoxRow {
            id: checkBoxRowStagingServer

            Layout.fillWidth: true
            Layout.rightMargin: VPNTheme.theme.windowMargin
            labelText: VPNI18n.SettingsDevUseStagingTitle
            subLabelText: VPNI18n.SettingsDevUseStagingSubtitle
            isChecked: VPNSettings.stagingServer
            showDivider: false
            onClicked: {
                VPNSettings.stagingServer = !VPNSettings.stagingServer
            }
        }

        VPNTextField {
            id: serverAddressInput

            Layout.fillWidth: true
            Layout.rightMargin: VPNTheme.theme.windowMargin * 2
            Layout.leftMargin: VPNTheme.theme.windowMargin * 3

            Layout.alignment: Qt.AlignHCenter
            enabled: VPNSettings.stagingServer
            _placeholderText: "Staging server address"
            Layout.preferredHeight: VPNTheme.theme.rowHeight

            PropertyAnimation on opacity {
                duration: 200
            }

            onTextChanged: text => {
                               if (VPNSettings.stagingServerAddress !== serverAddressInput.text) {
                                   VPNSettings.stagingServerAddress = serverAddressInput.text;
                               }
                           }

            Component.onCompleted: {
                serverAddressInput.text = VPNSettings.stagingServerAddress;
            }
        }

        VPNCheckBoxRow {
            Layout.fillWidth: true
            Layout.topMargin: VPNTheme.theme.windowMargin
            Layout.rightMargin: VPNTheme.theme.windowMargin

            labelText: "Custom Add-on URL"
            subLabelText: "Load add-ons from an alternative URL address"

            isChecked: VPNSettings.addonCustomServer
            showDivider: false
            onClicked: {
                VPNSettings.addonCustomServer = !VPNSettings.addonCustomServer
            }
        }

        VPNTextField {
            id: addonCustomServerInput

            Layout.rightMargin: VPNTheme.theme.windowMargin * 2
            Layout.leftMargin: VPNTheme.theme.windowMargin * 3
            Layout.alignment: Qt.AlignRight
            Layout.fillWidth: true

            enabled: VPNSettings.addonCustomServer
            _placeholderText: "Addon Custom Server Address"
            height: 40

            PropertyAnimation on opacity {
                duration: 200
            }

            onTextChanged: text => {
                               if (VPNSettings.addonCustomServerAddress !== addonCustomServerInput.text) {
                                   VPNSettings.addonCustomServerAddress = addonCustomServerInput.text;
                               }
                           }

            Component.onCompleted: {
                addonCustomServerInput.text = VPNSettings.addonCustomServerAddress;
            }
        }

        VPNCheckBoxRow {
            id: checkBoxRowProdKeyInStaging

            Layout.rightMargin: VPNTheme.theme.windowMargin
            labelText: "Add-on production signature key in staging"
            subLabelText: "Use the add-on production signature key in staging"
            isChecked: VPNSettings.addonProdKeyInStaging
            showDivider: false
            onClicked: {
                VPNSettings.addonProdKeyInStaging = !VPNSettings.addonProdKeyInStaging
            }
        }

        Rectangle {
            id: divider
            Layout.preferredHeight: 1
            Layout.fillWidth: true
            Layout.topMargin: VPNTheme.theme.windowMargin / 2
            Layout.leftMargin: VPNTheme.theme.windowMargin * 3
            Layout.rightMargin: VPNTheme.theme.windowMargin
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
                    title: "Messages - REMOVE ME"
                    viewQrc: "qrc:/ui/screens/getHelp/developerMenu/ViewMessages.qml"
                }
                ListElement {
                    title: "Animations playground"
                    viewQrc: "qrc:/ui/screens/getHelp/developerMenu/ViewAnimationsPlayground.qml"
                }
            }

            delegate: VPNSettingsItem {
               settingTitle:  title
               imageLeftSrc: "qrc:/ui/resources/settings/questionMark.svg"
               imageRightSrc: "qrc:/nebula/resources/chevron.svg"
               imageRightMirror: VPNLocalizer.isRightToLeft
               onClicked: getHelpStackView.push(viewQrc)
               Layout.leftMargin: VPNTheme.theme.windowMargin / 2
               Layout.rightMargin: VPNTheme.theme.windowMargin / 2
            }
        }

        //Need to wrap VPNExternalLinkListItem in an item since it is not written to work in a layout
        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: VPNTheme.theme.rowHeight

            visible: checkBoxRowStagingServer.isChecked && !restartRequired.isVisible

            VPNExternalLinkListItem {
                id: inspectorLink

                anchors.left: parent.left
                anchors.right: parent.right

                objectName: "openInspector"
                title: "Open Inspector"
                accessibleName: "Open Inspector"
                iconSource:  "qrc:/nebula/resources/externalLink.svg"
                backgroundColor: VPNTheme.theme.clickableRowBlue
                onClicked: {
                    VPNUrlOpener.openUrlLabel("inspector");
                }
            }
        }

        VPNContextualAlerts {
            id: restartRequired

            property bool isVisible: false

            Layout.topMargin: VPNTheme.theme.listSpacing
            Layout.leftMargin: VPNTheme.theme.windowMargin/2

            messages: [
                {
                    type: "warning",
                    message: VPNI18n.SettingsDevRestartRequired,
                    visible: isVisible
                }
            ]

            Connections {
                target: VPNSettings
                function onStagingServerAddressChanged() {
                    restartRequired.isVisible = true;
                }
                function onStagingServerChanged() {
                    restartRequired.isVisible = true;
                }
            }
        }

        VPNButton {
            id: reinstateMessages

            Layout.topMargin: VPNTheme.theme.listSpacing * 2

            text: "Reinstate messages"
            onClicked: {
                VPNAddonManager.reinstateMessages()
            }
        }

        VPNButton {
            id: crashApp
            property int clickNeeded: 5


            text: "Test Crash Reporter"
            onClicked: {
                if (!VPNSettings.stagingServer){
                    text = "Test Crash Reporter (Staging only!)";
                    return;
                }
                if (clickNeeded) {
                    text = "Test Crash Reporter (" + clickNeeded + ")";
                    --clickNeeded;
                    return;
                }
                VPN.crashTest()
            }
        }

        VPNButton {
            id: resetAndQuit
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
            Layout.topMargin: VPNTheme.theme.vSpacingSmall

            VPNTextBlock {
                Layout.fillWidth: true

                text: VPN.devVersion
            }

            VPNTextBlock {
                Layout.fillWidth: true

                text: "Installation time: " + VPNSettings.installationTime
            }

            VPNTextBlock {
                Layout.fillWidth: true

                text: "Update time: " + VPNSettings.updateTime
            }
        }
    }
}

