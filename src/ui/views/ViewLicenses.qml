/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import themes 0.1

Item {
    id: root

    property alias isSettingsView: menu.isSettingsView
    property alias isMainView: menu.isMainView
    property string _menuTitle: qsTrId("vpn.aboutUs.license")

    VPNMenu {
        id: menu
        objectName: "aboutUsBackButton"
        title: qsTrId("vpn.aboutUs.license")
    }

    VPNFlickable {
        id: licenses

        anchors {
            left: parent.left
            right: parent.right
            top: menu.bottom
        }
        height: root.height - menu.height
        flickContentHeight: body.height + Theme.windowMargin * 2
        width: root.width

        Column {
            id: body

            spacing: Theme.listSpacing
            width: licenses.width

            VPNVerticalSpacer {
                height: Theme.listSpacing
            }

            VPNTextBlock {
                id: mainLicense

                anchors.horizontalCenter: parent.horizontalCenter
                text: VPNLicenseModel.contentLicense
                textFormat: Text.MarkdownText
                width: parent.width - Theme.windowMargin * 2

                onLinkActivated: link => VPN.openLinkUrl(link)
            }

            VPNVerticalSpacer {
                height: Theme.listSpacing * 0.5
            }

            Repeater {
                model: VPNLicenseModel

                VPNExternalLinkListItem {
                    accessibleName: licenseTitle
                    title: licenseTitle
                    iconSource: "qrc:/ui/resources/chevron.svg"

                    onClicked: {
                        if (isSettingsView) {
                            settingsStackView.push("qrc:/ui/views/ViewLicense.qml", { isSettingsView, isMainView, licenseTitle, licenseContent })
                        } else if (isMainView) {
                            mainStackView.push("qrc:/ui/views/ViewLicense.qml", { isSettingsView, isMainView, licenseTitle, licenseContent })
                        } else {
                            stackview.push("qrc:/ui/views/ViewLicense.qml", { isSettingsView, isMainView, licenseTitle, licenseContent })
                        }
                    }
                }
            }
        }

        Component.onCompleted: VPNLicenseModel.initialize()
    }
}
