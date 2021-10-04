/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../components"
import "../components/forms"
import "../themes/themes.js" as Theme

VPNFlickable {
    property string _menuTitle: VPNl18n.SettingsDevTitle
    property alias isSettingsView: menu.isSettingsView
    property bool vpnIsOff: (VPNController.state === VPNController.StateOff) ||
        (VPNController.state === VPNController.StateInitializing)

    id: vpnFlickable
    VPNMenu {
        id: menu
        title: VPNl18n.SettingsDevTitle
        isSettingsView: false
    }

    VPNCheckBoxRow {
        id: developerUnlock

        anchors.top: menu.bottom
        anchors.topMargin: Theme.windowMargin
        width: parent.width - Theme.windowMargin

        labelText:  VPNl18n.SettingsDevShowOptionTitle
        subLabelText: VPNl18n.SettingsDevShowOptionSubtitle

        isChecked: VPNSettings.developerUnlock
        onClicked: VPNSettings.developerUnlock = !VPNSettings.developerUnlock
    }

    RowLayout {
        id: stagingServerRow
        spacing: Theme.windowMargin
        anchors.top: developerUnlock.bottom
        anchors.topMargin: Theme.windowMargin
        width: parent.width - Theme.windowMargin

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
                color: Theme.fontColorDark
                horizontalAlignment: Text.AlignLeft
            }

            VPNTextBlock {
                id: subLabel

                Layout.fillWidth: true
                text: VPNl18n.SettingsDevUseStagingSubtitle
            }

            VPNVerticalSpacer {
                height: Theme.windowMargin
            }

            VPNTextField {
                Layout.fillWidth: true
                Layout.rightMargin: Theme.windowMargin

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

                Layout.topMargin: Theme.windowMargin
                Layout.preferredHeight: 1
                Layout.fillWidth: true
                color: "#E7E7E7"
                visible: true
            }
        }
    }
    VPNSettingsItem {
        id: featureListLink
        objectName: "settingsFeatureList"

        anchors.top: stagingServerRow.bottom
        anchors.topMargin: Theme.windowMargin
        anchors.left: stagingServerRow.left
        anchors.leftMargin: Theme.windowMargin/2
        width: parent.width - Theme.windowMargin

        // Do not translate this string!
        settingTitle: "Feature list"
        imageLeftSrc: "../resources/settings/whatsnew.svg"
        imageRightSrc: "../resources/chevron.svg"
        onClicked: stackview.push("../developerMenu/ViewFeatureList.qml")
    }
    VPNExternalLinkListItem {
        id:inspectorLink
        visible: stagingServerCheckBox.checked && !restartRequired.visible
        anchors.top: featureListLink.bottom
        anchors.topMargin: Theme.windowMargin
        anchors.left: featureListLink.left
        anchors.leftMargin: Theme.windowMargin/2
        width: parent.width - Theme.windowMargin

        objectName: "openInspector"
        title: "Open Inspector"
        accessibleName: "Open Inspector"
        iconSource:  "../resources/externalLink.svg"
        backgroundColor: Theme.clickableRowBlue
        onClicked: {
            VPN.openLink(VPN.LinkInspector)
        }
    }

    VPNInputMessages {
        id: restartMessage

        property bool isVisible: false

        anchors.left: inspectorLink.left
        anchors.top: inspectorLink.bottom
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
                restartMessage.isVisible = true;
            }
            function onStagingServerChanged() {
                restartMessage.isVisible = true;
            }
        }
    }
}
