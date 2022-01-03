/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1

VPNFlickable {
    property string _menuTitle: VPNl18n.SettingsDevTitle
    property alias isSettingsView: menu.isSettingsView
    property bool vpnIsOff: (VPNController.state === VPNController.StateOff) ||
        (VPNController.state === VPNController.StateInitializing)

    id: vpnFlickable
    flickContentHeight: restartRequired.y + restartRequired.height

    VPNMenu {
        id: menu
        title: VPNl18n.SettingsDevTitle
        isSettingsView: false
    }

    VPNCheckBoxRow {
        id: developerUnlock

        anchors.top: menu.bottom
        anchors.topMargin: VPNTheme.theme.windowMargin
        width: parent.width - VPNTheme.theme.windowMargin

        labelText:  VPNl18n.SettingsDevShowOptionTitle
        subLabelText: VPNl18n.SettingsDevShowOptionSubtitle

        isChecked: VPNSettings.developerUnlock
        onClicked: VPNSettings.developerUnlock = !VPNSettings.developerUnlock
    }

    RowLayout {
        id: stagingServerRow
        spacing: VPNTheme.theme.windowMargin
        anchors.top: developerUnlock.bottom
        anchors.topMargin: VPNTheme.theme.windowMargin
        width: parent.width - VPNTheme.theme.windowMargin

        VPNCheckBox {
            id: stagingServerCheckBox
            Layout.leftMargin: 18
            checked: (VPNSettings.stagingServer)
            enabled: vpnFlickable.vpnIsOff
            opacity: vpnFlickable.vpnIsOff ? 1 : 0.5
            onClicked: {
                if (vpnFlickable.vpnIsOff) {
                    VPNSettings.stagingServer = !VPNSettings.stagingServer
                }
            }
        }

        ColumnLayout {
            id: labelWrapper

            Layout.fillWidth: true
            Layout.topMargin: 2
            spacing: 4
            Layout.alignment: Qt.AlignTop

            VPNInterLabel {
                id: label
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                Layout.fillWidth: true
                text: VPNl18n.SettingsDevUseStagingTitle
                color: VPNTheme.theme.fontColorDark
                horizontalAlignment: Text.AlignLeft
            }

            VPNTextBlock {
                id: subLabel

                Layout.fillWidth: true
                text: VPNl18n.SettingsDevUseStagingSubtitle
            }

            VPNVerticalSpacer {
                height: VPNTheme.theme.windowMargin
            }

            VPNTextField {
                Layout.fillWidth: true
                Layout.rightMargin: VPNTheme.theme.windowMargin

                id: serverAddressInput

                enabled: vpnFlickable.vpnIsOff && VPNSettings.stagingServer
                placeholderText: "Staging server address"
                text: VPNSettings.stagingServerAddress
                height: 40

                PropertyAnimation on opacity {
                    duration: 200
                }

                onTextChanged: text => {
                    if (vpnFlickable.vpnIsOff) {
                        VPNSettings.stagingServerAddress = serverAddressInput.text;
                    }
                }
            }

            Rectangle {
                id: divider

                Layout.topMargin: VPNTheme.theme.windowMargin
                Layout.preferredHeight: 1
                Layout.fillWidth: true
                color: "#E7E7E7"
                visible: true
            }
        }
    }

    VPNSettingsItem {
        id: themeListLink

        anchors.top: stagingServerRow.bottom
        anchors.topMargin: VPNTheme.theme.windowMargin
        anchors.left: stagingServerRow.left
        anchors.leftMargin: VPNTheme.theme.windowMargin/2
        width: parent.width - VPNTheme.theme.windowMargin

        // Do not translate this string!
        settingTitle: "Theme list"
        imageLeftSrc: "qrc:/ui/resources/settings/whatsnew.svg"
        imageRightSrc: "qrc:/nebula/resources/chevron.svg"
        onClicked: stackview.push("qrc:/ui/developerMenu/ViewThemeList.qml")
    }

    VPNSettingsItem {
        id: featureListLink
        objectName: "settingsFeatureList"

        anchors.top: themeListLink.bottom
        anchors.topMargin: VPNTheme.theme.windowMargin
        anchors.left: stagingServerRow.left
        anchors.leftMargin: VPNTheme.theme.windowMargin/2
        width: parent.width - VPNTheme.theme.windowMargin

        // Do not translate this string!
        settingTitle: "Feature list"
        imageLeftSrc: "qrc:/ui/resources/settings/whatsnew.svg"
        imageRightSrc: "qrc:/nebula/resources/chevron.svg"
        onClicked: stackview.push("qrc:/ui/developerMenu/ViewFeatureList.qml")
    }

    VPNExternalLinkListItem {
        id:inspectorLink
        visible: stagingServerCheckBox.checked && !restartRequired.visible
        anchors.top: featureListLink.bottom
        anchors.topMargin: VPNTheme.theme.windowMargin
        anchors.left: stagingServerRow.left
        anchors.leftMargin: VPNTheme.theme.windowMargin/2
        width: parent.width - VPNTheme.theme.windowMargin

        objectName: "openInspector"
        title: "Open Inspector"
        accessibleName: "Open Inspector"
        iconSource:  "qrc:/nebula/resources/externalLink.svg"
        backgroundColor: VPNTheme.theme.clickableRowBlue
        onClicked: {
            VPN.openLink(VPN.LinkInspector)
        }
    }

    VPNButton {
        id:resetAndQuit
        property int clickNeeded: 5

        anchors.top: stagingServerCheckBox.checked && !restartRequired.visible ? inspectorLink.bottom : featureListLink.bottom
        anchors.topMargin: VPNTheme.theme.windowMargin
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter

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

    VPNCheckBoxAlert {
        id: restartRequired
        anchors.top: resetAndQuit.bottom
        visible: false
        Connections {
            target: VPNSettings
            function onStagingServerAddressChanged() { restartRequired.visible = true; }
            function onStagingServerChanged() { restartRequired.visible = true; }
        }

        errorMessage: VPNl18n.SettingsDevRestartRequired
    }
}
