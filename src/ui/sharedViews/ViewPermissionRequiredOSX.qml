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

            source: MZAssetLookup.getImageSource("MacosPermissionGeneric")
            fillMode: Image.PreserveAspectFit

            Text {
                id: imageTitle

                text: getImageTitle()
                horizontalAlignment: Text.AlignLeft

                color: MZTheme.colors.useDarkAssets ? MZTheme.colors.grey15 : MZTheme.colors.grey55
                font.pixelSize: 12
                font.family: MZTheme.theme.fontBoldFamily

                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right

                topPadding: 16
                leftPadding: 16

                Accessible.ignored: true

                function getImageTitle() {
                    if (MZFeatureList.get("networkExtension").isSupported) {
                        return MZI18n.PermissionMacosImageTitleNetworkExtension;
                    }
                    if (VPNMacOSUtils.getMacOSMajorVersion() > 25) {
                        return MZI18n.PermissionMacosImageTitleBackgroundMacOS26;
                    }
                    return MZI18n.PermissionMacosImageTitleBackground;
                }
            }

            Text {
                id: imageAppName

                text: MZI18n.ProductName
                horizontalAlignment: Text.AlignLeft

                color: MZTheme.colors.useDarkAssets ? MZTheme.colors.grey15 : MZTheme.colors.grey55
                font.pixelSize: 12
                font.family: MZTheme.theme.fontBoldFamily

                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right

                topPadding: 91
                leftPadding: 85

                Accessible.ignored: true
            }
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

            text: getButtonText()

            Layout.preferredHeight: MZTheme.theme.rowHeight
            loaderVisible: false
            onClicked: VPNMacOSUtils.openSystemSettingsLink()

            function getButtonText() {
                if (MZFeatureList.get("networkExtension").isSupported) {
                    return MZI18n.PermissionMacosOpenSettingsButtonLabelGeneric
                }
                if (VPNMacOSUtils.getMacOSMajorVersion() > 14) {
                    return MZI18n.PermissionMacosOpenSettingsButtonLabelMacOS15;
                }
                return MZI18n.PermissionMacosOpenSettingsButtonLabel
            }
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
