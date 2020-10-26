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

    Flickable {
        id: wrapper
        anchors.top: menu.bottom
        anchors.topMargin: 20
        width: parent.width
        height: parent.height - menu.height
        contentWidth: parent.width
        contentHeight: wrapper.childrenRect.height
        boundsBehavior: Flickable.StopAtBounds

        VPNBoldLabel {
            id: systemLabel

            anchors.left: parent.left
            anchors.leftMargin: Theme.windowMargin
            width: parent.width
            //% "System"
            //: The system language
            text: qsTrId("vpn.settings.system")
            Accessible.role: Accessible.Heading
        }

        VPNRadioDelegate {
            id: systemLanguage

            radioButtonLabelText: VPNLocalizer.systemLanguage
            checked: VPNSettings.languageCode === ""
            onClicked: VPNSettings.languageCode = ""
            anchors.top: systemLabel.bottom
            anchors.topMargin: 16
            anchors.left: parent.left
            anchors.leftMargin: defaultMargin
            anchors.rightMargin: defaultMargin
            width: parent.width - defaultMargin * 2
            activeFocusOnTab: true
            //% "%1 %2"
            accessibleName: qsTrId("vpn.settings.languageAccessibleName")
                .arg(VPNLocalizer.systemLanguage)
                .arg(VPNLocalizer.systemLocalizedLanguage)

            VPNFocus {
                itemToFocus: systemLanguage
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
            //% "Additional"
            //: List of the additional languages
            text: qsTrId("vpn.settings.additional")
            Accessible.role: Accessible.Heading
        }

        VPNList {
            id: additionalLanguageList

            clip: true
            anchors.top: additionalLabel.bottom
            anchors.topMargin: 16
            anchors.left: parent.left
            anchors.leftMargin: defaultMargin
            anchors.rightMargin: defaultMargin
            width: parent.width - defaultMargin * 2
            height: contentItem.childrenRect.height + 40
            spacing: 26
            listName: additionalLabel.text
            model: VPNLocalizer

            delegate: VPNRadioDelegate {
                radioButtonLabelText: language
                checked: VPNSettings.languageCode === code
                onClicked: VPNSettings.languageCode = code

                width: additionalLanguageList.width
                anchors.left: parent.left
                anchors.topMargin: Theme.windowMargin
                //% "%1 %2"
                accessibleName: qsTrId("vpn.settings.languageAccessibleName")
                    .arg(language)
                    .arg(localizedLanguage)

                VPNRadioSublabel {
                    text: localizedLanguage
                }

            }

        }

        ScrollBar.vertical: ScrollBar {
            Accessible.ignored: true
        }

    }

}
