/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1

VPNViewBase {
    property string _emailAddress: ""
    id: base

    _viewContentData: ColumnLayout {
        Layout.minimumHeight: base.height

        ColumnLayout {
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: Math.min(VPNTheme.theme.maxHorizontalContentWidth, parent.width - VPNTheme.theme.windowMargin * 4)
            spacing: VPNTheme.theme.vSpacing

            VPNPanel {
                id: panel
                logo: "qrc:/ui/resources/heart-check.svg"
                logoTitle: VPNl18n.InAppSupportWorkflowSupportResponseHeader
                logoSubtitle: VPNl18n.InAppSupportWorkflowSupportResponseBody.arg(base._emailAddress)
                anchors.horizontalCenter: undefined
                Layout.fillWidth: true
            }

            VPNButton {
                text: VPNl18n.InAppSupportWorkflowSupportResponseButton
                Layout.alignment: Qt.AlignHCenter
                onClicked: {
                    getHelpStackView.pop();
                    getHelpStackView.pop();
                }
            }

            VPNVerticalSpacer {
                Layout.preferredHeight: VPNTheme.theme.rowHeight
                Layout.fillWidth: true
            }
        }
    }
}
