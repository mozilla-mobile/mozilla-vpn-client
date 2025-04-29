/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import "qrc:/nebula/utils/MZAssetLookup.js" as MZAssetLookup

MZFlickable {
    id: vpnFlickable

    property var primaryButtonObjectName

    width: parent.width
    flickContentHeight: headerLink.implicitHeight + col.implicitHeight + col.anchors.topMargin

    MZHeaderLink {
        id: headerLink
        objectName: "getHelpLink"

        labelText: MZI18n.GetHelpLinkText
        onClicked: MZNavigator.requestScreen(VPN.ScreenGetHelp)
    }

    ColumnLayout {
        id: col

        anchors.top: headerLink.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: MZTheme.theme.vSpacingSmall

        Image {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: MZTheme.theme.contentTopMarginDesktop

            source: MZAssetLookup.getLocalizedImageSource("MacosPermissionBackground")
            fillMode: Image.PreserveAspectFit
        }

        MZHeadline {
            id: headline

            text: MZI18n.PermissionMacosTitle
            horizontalAlignment: Text.AlignLeft

            Layout.fillWidth: true
            Layout.leftMargin: MZTheme.theme.windowMargin * 1.5
            Layout.rightMargin: MZTheme.theme.windowMargin * 1.5
        }

        ColumnLayout {
            id: textbox

            Layout.fillWidth: true
            Layout.leftMargin: MZTheme.theme.windowMargin * 1.5
            Layout.rightMargin: MZTheme.theme.windowMargin * 1.5
            
            spacing: MZTheme.theme.listSpacing

            MZInterLabel {
                Layout.topMargin: 8
                Layout.fillWidth: true

                text: MZI18n.PermissionMacosBody
                horizontalAlignment: Text.AlignLeft
            }

            MZInterLabel {
                Layout.topMargin: 8
                Layout.fillWidth: true

                text: MZI18n.PermissionMacosInstructions
                horizontalAlignment: Text.AlignLeft
            }
        }

        Item {
            Layout.fillHeight: !window.fullscreenRequired()
        }

        MZButton {
            id: primaryButton

            objectName: primaryButtonObjectName
            text: MZI18n.PermissionMacosOpenSettingsButtonLabel
            Layout.preferredHeight: MZTheme.theme.rowHeight
            loaderVisible: false
            onClicked: VPNMacOSUtils.openSystemSettingsLoginItems()
        }

        MZSignOut {
            id: signOff

            Layout.preferredHeight: MZTheme.theme.rowHeight
            Layout.alignment: Qt.AlignHCenter
            anchors.horizontalCenter: undefined
            anchors.bottom: undefined
            anchors.bottomMargin: undefined
            height: undefined
        }
    }
}
