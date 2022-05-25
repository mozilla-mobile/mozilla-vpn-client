/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1

Item {
    property string _menuTitle: VPNl18n.SettingsDevTitle
    property bool vpnIsOff: (VPNController.state === VPNController.StateOff) ||
                            (VPNController.state === VPNController.StateInitializing)

    id: root

    VPNMenu {
        id: menu
        title: VPNl18n.SettingsDevTitle
        anchors.top: parent.top
    }

    VPNFlickable {
        id: flickableContent
        flickContentHeight: layout.implicitHeight

        anchors.top: menu.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        ColumnLayout {
            id: layout

            anchors.right: parent.right
            anchors.left: parent.left

            spacing: 0

            VPNCheckBoxRow {
                id: developerUnlock

                Layout.fillWidth: true
                Layout.topMargin: VPNTheme.theme.windowMargin
                Layout.rightMargin: VPNTheme.theme.windowMargin

                labelText:  VPNl18n.SettingsDevShowOptionTitle
                subLabelText: VPNl18n.SettingsDevShowOptionSubtitle

                isChecked: VPNSettings.developerUnlock
                onClicked: VPNSettings.developerUnlock = !VPNSettings.developerUnlock
            }

            VPNCheckBoxRow {
                id: checkBoxRowStagingServer

                Layout.fillWidth: true
                Layout.topMargin: VPNTheme.theme.windowMargin
                Layout.rightMargin: VPNTheme.theme.windowMargin

                labelText: VPNl18n.SettingsDevUseStagingTitle
                subLabelText: VPNl18n.SettingsDevUseStagingSubtitle

                isChecked: VPNSettings.stagingServer
                isEnabled: root.vpnIsOff
                showDivider: false
                onClicked: {
                    if (root.vpnIsOff) {
                        VPNSettings.stagingServer = !VPNSettings.stagingServer
                    }
                }
            }

            VPNTextField {
                id: serverAddressInput

                Layout.topMargin: VPNTheme.theme.windowMargin
                Layout.rightMargin: VPNTheme.theme.windowMargin * 2
                implicitWidth: checkBoxRowStagingServer.labelWidth - VPNTheme.theme.windowMargin
                Layout.alignment: Qt.AlignRight

                enabled: root.vpnIsOff && VPNSettings.stagingServer
                _placeholderText: "Staging server address"
                height: 40

                PropertyAnimation on opacity {
                    duration: 200
                }

                onTextChanged: text => {
                                   if (root.vpnIsOff && VPNSettings.stagingServerAddress !== serverAddressInput.text) {
                                       VPNSettings.stagingServerAddress = serverAddressInput.text;
                                   }
                               }

                Component.onCompleted: {
                    serverAddressInput.text = VPNSettings.stagingServerAddress;
                }
            }

            Rectangle {
                id: divider

                Layout.topMargin: VPNTheme.theme.listSpacing * 2.5
                Layout.rightMargin: VPNTheme.theme.windowMargin
                Layout.preferredHeight: 1
                Layout.preferredWidth: checkBoxRowStagingServer.labelWidth
                Layout.alignment: Qt.AlignRight

                color: "#E7E7E7"
            }

            VPNSettingsItem {
                id: themeListLink

                Layout.topMargin: VPNTheme.theme.listSpacing
                Layout.leftMargin: VPNTheme.theme.windowMargin/2
                Layout.rightMargin: VPNTheme.theme.windowMargin/2
                Layout.fillWidth: true

                // Do not translate this string!
                settingTitle: "Theme list"
                imageLeftSrc: "qrc:/ui/resources/settings/whatsnew.svg"
                imageRightSrc: "qrc:/nebula/resources/chevron.svg"
                onClicked: mainStackView.push("qrc:/ui/developerMenu/ViewThemeList.qml")
            }

            VPNSettingsItem {
                id: featureListLink
                objectName: "settingsFeatureList"

                Layout.topMargin: VPNTheme.theme.listSpacing
                Layout.leftMargin: VPNTheme.theme.windowMargin/2
                Layout.rightMargin: VPNTheme.theme.windowMargin/2
                Layout.fillWidth: true

                // Do not translate this string!
                settingTitle: "Feature list"
                imageLeftSrc: "qrc:/ui/resources/settings/whatsnew.svg"
                imageRightSrc: "qrc:/nebula/resources/chevron.svg"
                onClicked: mainStackView.push("qrc:/ui/developerMenu/ViewFeatureList.qml")
            }

            VPNSettingsItem {
                id: tutorialLink

                Layout.topMargin: VPNTheme.theme.listSpacing
                Layout.leftMargin: VPNTheme.theme.windowMargin/2
                Layout.rightMargin: VPNTheme.theme.windowMargin/2
                Layout.fillWidth: true

                // Do not translate this string!
                settingTitle: "Tutorial - REMOVE ME!"
                imageLeftSrc: "qrc:/ui/resources/settings/whatsnew.svg"
                imageRightSrc: "qrc:/nebula/resources/chevron.svg"
                onClicked: mainStackView.push("qrc:/ui/developerMenu/ViewTutorials.qml")
            }

            VPNSettingsItem {
                id: animationsPlaygroundLink
                objectName: "settingsAnimationsPlayground"

                Layout.topMargin: VPNTheme.theme.listSpacing
                Layout.leftMargin: VPNTheme.theme.windowMargin/2
                Layout.rightMargin: VPNTheme.theme.windowMargin/2
                Layout.fillWidth: true

                // Do not translate this string!
                settingTitle: "Animations playground"
                imageLeftSrc: "qrc:/ui/resources/settings/whatsnew.svg"
                imageRightSrc: "qrc:/nebula/resources/chevron.svg"
                onClicked: mainStackView.push("qrc:/ui/developerMenu/ViewAnimationsPlayground.qml")
            }

            //Need to wrap VPNExternalLinkListItem in an item since it is not written to work in a layout
            Item {
                Layout.topMargin: VPNTheme.theme.listSpacing
                Layout.fillWidth: true
                implicitHeight: inspectorLink.height

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
                        VPN.openLink(VPN.LinkInspector)
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
                        message: VPNl18n.SettingsDevRestartRequired,
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
                id: crashApp
                property int clickNeeded: 5

                Layout.topMargin: VPNTheme.theme.listSpacing * 2

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

                Layout.topMargin: VPNTheme.theme.listSpacing * 2

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

            VPNTextBlock {
                id: qtVersionText

                Layout.topMargin: VPNTheme.theme.listSpacing
                Layout.leftMargin: 31
                Layout.rightMargin: 31
                Layout.bottomMargin: VPNTheme.theme.listSpacing
                Layout.fillWidth: true

                text: VPN.devVersion
            }

            VPNVerticalSpacer {
                height: VPNTheme.theme.windowMargin
            }
        }
    }
}

