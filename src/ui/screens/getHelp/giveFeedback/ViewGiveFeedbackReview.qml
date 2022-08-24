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
    id: root
    _viewContentData: ColumnLayout {

        Layout.minimumHeight: root.height
        Layout.leftMargin: VPNTheme.theme.windowMargin * 2
        Layout.rightMargin: VPNTheme.theme.windowMargin * 2

        ColumnLayout {
            spacing: VPNTheme.theme.vSpacing
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            Layout.fillWidth: true
            Layout.bottomMargin: VPNTheme.theme.rowHeight * 2

            Column {
                Layout.fillWidth: true
                VPNPanel {
                    id: panel
                    logo: "qrc:/ui/resources/app-rating.svg"
                    logoTitle: VPNl18n.FeedbackFormReviewHeader
                    logoSubtitle: VPNl18n.FeedbackFormReviewBody
                }
            }

            VPNButton {
                text: VPNl18n.FeedbackFormLeaveReviewButton
                width: undefined
                Layout.fillWidth: true
                onClicked: {
                    VPN.openAppStoreReviewLink();
                    getHelpStackView.push("qrc:/ui/screens/getHelp/giveFeedback/ViewGiveFeedbackThankYou.qml");
               }
            }

            VPNLinkButton {
                id: skipLink

                labelText: qsTrId("vpn.feedbackForm.skip")
                onClicked: getHelpStackView.pop(null, StackView.Immediate);
                implicitHeight: VPNTheme.theme.rowHeight
                Layout.alignment: Qt.AlignHCenter
            }
        }

    }
}

