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
    id: osxPermissionRequired

    width: parent.width
    flickContentHeight: col.implicitHeight + col.anchors.topMargin

    ColumnLayout {
        id: col

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: MZTheme.theme.vSpacingSmall

        Image {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: MZTheme.theme.contentTopMarginDesktop

            source: VPNMacOSUtils.getMacOSMajorVersion() > 25 ? MZAssetLookup.getLocalizedImageSource("MacosPermissionBackgroundMacOS26") : MZAssetLookup.getLocalizedImageSource("MacosPermissionBackground")
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

                text: VPNMacOSUtils.getMacOSMajorVersion() > 25 ? MZI18n.PermissionMacosInstructionsMacOS26 :
                      VPNMacOSUtils.getMacOSMajorVersion() > 14 ? MZI18n.PermissionMacosInstructionsMacOS15 : MZI18n.PermissionMacosInstructions
                horizontalAlignment: Text.AlignLeft
            }
        }

        Item {
            Layout.fillHeight: !window.fullscreenRequired()
        }

        MZButton {
            id: openSettingsButton

            text: VPNMacOSUtils.getMacOSMajorVersion() > 14 ? MZI18n.PermissionMacosOpenSettingsButtonLabelMacOS15 : MZI18n.PermissionMacosOpenSettingsButtonLabel
            Layout.preferredHeight: MZTheme.theme.rowHeight
            loaderVisible: false
            onClicked: VPNMacOSUtils.openSystemSettingsLoginItems()
        }

        MZLinkButton {
            id: learnMoreLink

            labelText:  MZI18n.GlobalLearnMore
            Layout.preferredHeight: MZTheme.theme.rowHeight
            Layout.alignment: Qt.AlignHCenter
            onClicked: MZUrlOpener.openUrlLabel("sumoAllowBackgroundMacos")
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
