/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import themes 0.1

VPNFlickable {
    id: licenses

    property alias isSettingsView: menu.isSettingsView
    property alias isMainView: menu.isMainView
    property string _menuTitle:  qsTrId("vpn.aboutUs.license")

    flickContentHeight: menu.height + body.height + (Theme.windowMargin * 4)
    width: parent.width

    VPNMenu {
        id: menu
        objectName: "aboutUsBackButton"
        title: qsTrId("vpn.aboutUs.license")
    }

    Column {
        id: body

        anchors.right: parent.right
        anchors.left: parent.left
        anchors.top: menu.bottom

        width: licenses.width

        VPNTextBlock {
            id: mainLicense

            anchors.horizontalCenter: parent.horizontalCenter

            textFormat: Text.MarkdownText
            text: VPNLicenseModel.contentLicense

            onLinkActivated: link => VPN.openLinkUrl(link)
        }

        Repeater {
            model: VPNLicenseModel

            VPNExternalLinkListItem {
                title: licenseTitle
                accessibleName: licenseTitle
                onClicked: {
                    if (isSettingsView) {
                        settingsStackView.push("qrc:/ui/views/ViewLicense.qml", { isSettingsView, isMainView, licenseTitle, licenseContent })
                    } else if (isMainView) {
                        mainStackView.push("qrc:/ui/views/ViewLicense.qml", { isSettingsView, isMainView, licenseTitle, licenseContent })
                    } else {
                        stackview.push("qrc:/ui/views/ViewLicense.qml", { isSettingsView, isMainView, licenseTitle, licenseContent })
                    }
                }
                iconSource: "qrc:/ui/resources/chevron.svg"
            }
        }
    }

    Component.onCompleted: VPNLicenseModel.initialize()
}
