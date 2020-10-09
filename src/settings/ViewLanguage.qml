/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.15

import Mozilla.VPN 1.0

import "../components"
import "../themes/themes.js" as Theme

Item {
    id: container
    VPNMenu {
        id: menu
        title: qsTr("Language")
        isSettingsView: true
    }

    ButtonGroup {
        id: radioButtonGroup
    }

    ListView {
        id: languageList
        clip: true
        anchors.top: menu.bottom
        height: parent.height - menu.height
        width: parent.width
        spacing: 26

        interactive: languageList.count > 3

        headerPositioning: ListView.InlineHeader
        header: Item {
            width: parent.width
            height: 150

            VPNBoldLabel {
                id: systemLabel
                anchors.top: parent.top
                anchors.topMargin: 20
                anchors.left: parent.left
                anchors.leftMargin: Theme.windowMargin
                width: parent.width
                text: qsTr("System")
            }

            VPNRadioDelegate {
                id: systemLanguage

                radioButtonLabelText: VPNLocalizer.systemLanguage
                checked: VPNSettings.languageCode === ""
                onClicked: VPNSettings.languageCode = ""

                anchors.top: systemLabel.bottom
                anchors.topMargin: 16
                anchors.left: parent.left
                anchors.leftMargin: 18

                VPNRadioSublabel {
                    text: VPNLocalizer.systemLocalizedLanguage
                }
            }

            VPNBoldLabel {
                id: additionalLabel
                anchors.top: systemLanguage.bottom
                anchors.topMargin: 28
                anchors.left: parent.left
                anchors.leftMargin: Theme.windowMargin
                width: parent.width
                text: qsTr("Additional")
            }
        }

        model: VPNLocalizer
        delegate: Item {
            id: languageOption
            width: languageList.width
            height: radioDel.height

            VPNRadioDelegate {
                id: radioDel

                radioButtonLabelText: qsTr(language)
                checked: VPNSettings.languageCode === code
                onClicked: VPNSettings.languageCode = code

                anchors.left: parent.left
                anchors.leftMargin: 18
                anchors.topMargin: Theme.windowMargin

                VPNRadioSublabel {
                    text: localizedLanguage
                }
            }
        }

        footer: Rectangle {
            height: 40
            color: "transparent"
            width: parent.width
        }

        ScrollBar.vertical: ScrollBar {}
    }
}
