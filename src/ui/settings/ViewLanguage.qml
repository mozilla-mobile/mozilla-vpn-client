/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme

Item {
    id: container

    readonly property int defaultMargin: 18

    VPNMenu {
        id: menu

        //% "Language"
        title: qsTrId("vpn.settings.language")
        isSettingsView: true
    }

    ButtonGroup {
        id: radioButtonGroup
    }

    VPNList {
        id: languageList

        anchors.top: menu.bottom
        anchors.left: parent.left
        anchors.leftMargin: defaultMargin
        anchors.rightMargin: defaultMargin
        width: parent.width - defaultMargin * 2
        height: parent.height - menu.height
        spacing: 26
        listName: menu.title
        interactive: languageList.childrenRect.height > height
        model: VPNLocalizer

        header: Rectangle {
            height: defaultMargin
            color: "transparent"
            width: parent.width
        }

        delegate: VPNRadioDelegate {
            radioButtonLabelText: localizedLanguage
            checked: VPNLocalizer.code === code
            onClicked: VPNLocalizer.code = code

            width: languageList.width
            anchors.left: parent.left
            anchors.topMargin: Theme.windowMargin
            //% "%1 %2"
            //: This string is read by accessibility tools.
            //: %1 is the language name, %2 is the localized language name.
            accessibleName: qsTrId("vpn.settings.languageAccessibleName")
                .arg(language)
                .arg(localizedLanguage)

            VPNRadioSublabel {
                text: language
            }

        }

    }

}
