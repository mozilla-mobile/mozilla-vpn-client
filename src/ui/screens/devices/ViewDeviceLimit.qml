/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import "qrc:/nebula/utils/MZAssetLookup.js" as MZAssetLookup

MZViewBase {
    id: vpnFlickable

    property Component rightMenuButton: Component {
        MZLinkButton {
            id: getHelpLink

            property bool skipEnsureVisible: true

            labelText: MZI18n.GetHelpLinkText
            onClicked: MZNavigator.requestScreen(VPN.ScreenGetHelp);
        }
    }

    //% "Devices"
    _menuTitle: MZI18n.DevicesSectionTitle

    _viewContentData: ColumnLayout {
        spacing: MZTheme.theme.vSpacingSmall

        ColumnLayout {
            Layout.leftMargin: 40
            Layout.rightMargin: 40

            spacing: 0

            Image {
                id: logo

                Layout.alignment: Qt.AlignHCenter

                source: MZAssetLookup.getImageSource("DevicesLimit")
                sourceSize.height: 80
                sourceSize.width: 80
                fillMode: Image.PreserveAspectFit
            }

            ColumnLayout {
                spacing: MZTheme.theme.vSpacingSmall / 2

                MZHeadline {
                    id: logoTitle

                    Layout.fillWidth: true

                    text: MZI18n.DevicesLimitTitle
                    wrapMode: Text.WordWrap
                }

                MZSubtitle {
                    id: logoSubtitle

                    Layout.fillWidth: true

                    text: MZI18n.DevicesLimitDescription2
                }
            }
        }

        VPNDeviceList {
            Layout.fillWidth: true

            isDeviceLimit: true

            onRemoveItem: (name, publicKey) => { removePopup.initializeAndOpen(name, publicKey) }
        }

        MZLinkButton {
            Layout.leftMargin: 40
            Layout.rightMargin: 40
            Layout.fillWidth: true
            Layout.preferredHeight: MZTheme.theme.rowHeight

            labelText: MZI18n.GlobalSignOut
            fontName: MZTheme.theme.fontBoldFamily
            linkColor: MZTheme.colors.destructiveButton
            onClicked: VPN.logout();
        }
    }

    VPNRemoveDevicePopup {
        id: removePopup

        function initializeAndOpen(name, publicKey) {
            removePopup.deviceName = name;
            removePopup.devicePublicKey = publicKey;
            removePopup.open();
        }
    }

    Connections {
        target: VPN
        function onDeviceRemoving(devPublicKey) {
            MZNavigator.requestScreen(VPN.ScreenRemovingDevice)
        }
    }
}
