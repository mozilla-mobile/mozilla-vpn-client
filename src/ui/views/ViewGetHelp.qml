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
    id: getHelp
    objectName: "getHelp"
    property alias isSettingsView: menu.isSettingsView

    property int unlockCounter: 0
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
        isSettingsView: true

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

    VPNSettingsItem {
        id: developer
        objectName: "developer"

        anchors.top: menu.bottom
        anchors.topMargin: Theme.windowMargin
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
                settingsStackView.push("../views/ViewDeveloper.qml", {isSettingsView: true})
            } else {
                stackview.push("../views/ViewDeveloper.qml", {isSettingsView: false})
            }
        }
    }

    VPNList {
        objectName: "getHelpBackList"
        height: parent.height - menu.height - (developer.visible ? developer.height : 0)
        width: parent.width
        anchors.top: developer.visible ? developer.bottom : menu.bottom
        spacing: Theme.listSpacing
        anchors.topMargin: Theme.windowMargin
        listName: menu.title

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

        ScrollBar.vertical: ScrollBar {
            Accessible.ignored: true
        }

    }

}
