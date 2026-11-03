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
    objectName: "appPermissionsDisabled"

    _menuTitle: MZI18n.SettingsAppExclusionTitle
    _viewContentData: ColumnLayout {
        id: col
        Layout.fillWidth: true
        Layout.leftMargin: MZTheme.theme.windowMargin
        Layout.rightMargin: MZTheme.theme.windowMargin
        spacing: 0

        Rectangle {
            Layout.topMargin: MZTheme.theme.vSpacing
            Layout.preferredHeight: 48
            Layout.preferredWidth: 48
            Layout.alignment: Qt.AlignHCenter
            color: MZTheme.colors.errorAccent
            radius: height / 2

            Image {
                source: MZAssetLookup.getImageSource("WarningWhite")
                antialiasing: true
                sourceSize.height: 20
                sourceSize.width: 20
                anchors.centerIn: parent
            }
        }

        MZTextBlock {
            Layout.topMargin: MZTheme.theme.vSpacing
            Layout.preferredWidth: col.width - (MZTheme.theme.windowMargin * 3)
            Layout.preferredHeight: paintedHeight
            Layout.alignment: Qt.AlignHCenter
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: MZTheme.theme.fontSize
            lineHeight: MZTheme.theme.labelLineHeight
            text: VPNController.splitTunnelSupport === VPNController.SplitTunnelDisabledConflict
                  ? MZI18n.SettingsAppExclusionDisabledConflict
                  : MZI18n.SettingsAppExclusionDisabledError
        }

        // A conflict is something the user can fix themselves, a failure
        // needs our support team.
        MZButton {
            readonly property bool isConflict: VPNController.splitTunnelSupport === VPNController.SplitTunnelDisabledConflict

            objectName: isConflict ? "appPermissionsDisabledLearnMore" : "appPermissionsDisabledContactSupport"
            text: isConflict ? MZI18n.GlobalLearnMore : MZI18n.InAppSupportWorkflowSupportNavLinkText
            Layout.topMargin: MZTheme.theme.vSpacing * 2
            Layout.preferredHeight: MZTheme.theme.rowHeight
            Layout.fillWidth: true
            onClicked: {
                if (isConflict) {
                    MZUrlOpener.openUrlLabel("sumoExcludedApps")
                } else {
                    vpnFlickable.StackView.view.push("qrc:/qt/qml/Mozilla/VPN/screens/getHelp/contactUs/ViewContactUsForm.qml")
                }
            }
        }
    }
}
