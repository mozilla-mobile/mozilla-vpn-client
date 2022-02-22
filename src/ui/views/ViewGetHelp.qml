/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1

Item {
    property alias isSettingsView: menu.isSettingsView
    property alias isMainView: menu.isMainView
    //% "Get help"
    property string _menuTitle: qsTrId("vpn.main.getHelp2")
    property int unlockCounter: 0

    id: getHelp
    objectName: "getHelp"

    Timer {
        id: unlockTimeout
        repeat: false
        running: false
        interval: 10000
        onTriggered: unlockCounter = 0
    }

    VPNMenu {
        id: menu
        objectName: "getHelpBack"

        //% "Get help"
        title: qsTrId("vpn.main.getHelp2")
        visible: !isSettingsView
    }

    VPNMouseArea {
        anchors.fill: menu
        hoverEnabled: true
        onMouseAreaClicked: function() {
            if (unlockCounter >= 5) {
                unlockCounter = 0
                VPNSettings.developerUnlock = true
            }
            else if (!VPNSettings.developerUnlock) {
                unlockTimeout.restart()
                unlockCounter = unlockCounter + 1
            }
        }
    }

    Column {
        objectName: "getHelpLinks"
        spacing: VPNTheme.theme.windowMargin
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: VPNTheme.theme.windowMargin

        anchors.top: menu.bottom

        VPNExternalLinkListItem {
            objectName: "settingsGiveFeedback"

            accessibleName: title
            title: qsTrId("vpn.settings.giveFeedback")
            onClicked: isSettingsView ? settingsStackView.push("qrc:/ui/settings/ViewGiveFeedback.qml") : isMainView? mainStackView.push("qrc:/ui/settings/ViewGiveFeedback.qml") : stackview.push("qrc:/ui/settings/ViewGiveFeedback.qml", {isMainView: true})
            iconSource: "qrc:/nebula/resources/chevron.svg"
            backgroundColor: VPNTheme.theme.iconButtonLightBackground
            width: parent.width - VPNTheme.theme.windowMargin
            visible: VPN.userState === VPN.UserAuthenticated
        }

        Repeater {
            id: getHelpList
            objectName: "getHelpBackList"
            anchors.left: parent.left
            anchors.right: parent.right

            model: VPNHelpModel
            delegate: VPNExternalLinkListItem {
                objectName: "getHelpBackList-" + id
                title: name
                accessibleName: name
                iconSource: externalLink ? "qrc:/nebula/resources/externalLink.svg" : "qrc:/nebula/resources/chevron.svg"
                backgroundColor: externalLink ? VPNTheme.theme.clickableRowBlue : VPNTheme.theme.iconButtonLightBackground
                onClicked: {
                    VPNHelpModel.open(id)
                }
            }
        }

        VPNSettingsItem {
            id: developer
            objectName: "developer"

            width: parent.width - VPNTheme.theme.windowMargin
            spacing: VPNTheme.theme.listSpacing
            anchors.horizontalCenter: parent.horizontalCenter

            //% "Developer Options"
            settingTitle: qsTrId("vpn.settings.developer")
            imageLeftSrc: "qrc:/ui/resources/developer.svg"
            imageRightSrc: "qrc:/nebula/resources/chevron.svg"
            visible: VPNSettings.developerUnlock
            onClicked: {
                if (isSettingsView) {
                    settingsStackView.push("qrc:/ui/developerMenu/ViewDeveloperMenu.qml", {isSettingsView: true})
                } else {
                    stackview.push("qrc:/ui/developerMenu/ViewDeveloperMenu.qml", {isSettingsView: false})
                }
            }
        }
    }

}
