/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.5

import Mozilla.VPN 1.0

import "../components"
import "../themes/themes.js" as Theme

Flickable {
    VPNMenu {
        id: menu
        title: qsTr("Language")
        isSettingsView: true
    }

    ButtonGroup {
        id: radioButtonGroup
    }

    VPNBoldLabel {
        id: systemLabel
        anchors.top: menu.bottom
        anchors.topMargin: 20
        anchors.left: parent.left
        anchors.leftMargin: Theme.windowMargin
        width: parent.width
        text: qsTr("System")
    }

    VPNRadioDelegate {
        radioButtonLabelText: VPNLocalizer.systemLanguage

        id: systemLanguage
        anchors.top: systemLabel.bottom
        anchors.topMargin: 16
        anchors.left: parent.left
        anchors.leftMargin: 18

        checked: false // TODO : needs condition
        onClicked: {
            VPNLocalizer.setLanguage("")
            checked = !checked
        }

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

    ListView {
        height: parent.height - menu.height - systemLanguage.height - systemLabel.height
        width: parent.width
        anchors.top: additionalLabel.bottom
        anchors.topMargin: 16
        anchors.left: parent.left
        anchors.leftMargin: 18
        anchors.rightMargin: 18
        clip: true
        spacing: 26
        interactive: false
        model: VPNLocalizer

        delegate: VPNRadioDelegate {
            radioButtonLabelText: qsTr(language)

            anchors.leftMargin: 30
            anchors.topMargin: 30

            checked: false // TODO : needs condition
            onClicked: {
                VPNLocalizer.setLanguage(code)
                checked = !checked
            }

            VPNRadioSublabel {
                text: localizedLanguage
            }
        }
    }

    ScrollBar.vertical: ScrollBar {}
}
