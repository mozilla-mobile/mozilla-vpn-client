/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme

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

        onClicked: {
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
        spacing: Theme.windowMargin
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: Theme.windowMargin

        anchors.top: menu.bottom

        VPNExternalLinkListItem {
            objectName: "settingsGiveFeedback"

            accessibleName: title
            title: qsTrId("vpn.settings.giveFeedback")
            onClicked: isSettingsView ? settingsStackView.push("../settings/ViewGiveFeedback.qml") : isMainView? mainStackView.push("../settings/ViewGiveFeedback.qml") : stackview.push("../settings/ViewGiveFeedback.qml", {isMainView: true})
            iconSource: "../resources/chevron.svg"
            backgroundColor: Theme.iconButtonLightBackground
            width: parent.width - Theme.windowMargin
            visible: VPN.userAuthenticated
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
                iconSource: externalLink ? "../resources/externalLink.svg" : "../resources/chevron.svg"
                backgroundColor: externalLink ? Theme.clickableRowBlue : Theme.iconButtonLightBackground
                onClicked: {
                    VPNHelpModel.open(id)
                }
            }
        }

        VPNSettingsItem {
            id: developer
            objectName: "developer"

            width: parent.width - Theme.windowMargin
            spacing: Theme.listSpacing
            anchors.horizontalCenter: parent.horizontalCenter

            //% "Developer Options"
            settingTitle: qsTrId("vpn.settings.developer")
            imageLeftSrc: "../resources/developer.svg"
            imageRightSrc: "../resources/chevron.svg"
            visible: VPNSettings.developerUnlock
            onClicked: {
                if (isSettingsView) {
                    settingsStackView.push("../developerMenu/ViewDeveloperMenu.qml", {isSettingsView: true})
                } else {
                    stackview.push("../developerMenu/ViewDeveloperMenu.qml", {isSettingsView: false})
                }
            }
        }
    }

}
