/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1

ColumnLayout {
    id: layout

    spacing: 0

    Item {
        Layout.fillHeight: window.fullscreenRequired()
    }

    VPNPanel {
        anchors.horizontalCenter: undefined
        Layout.topMargin: window.fullscreenRequired() ? 0 : layout.height * 0.2
        Layout.preferredHeight: height
        Layout.preferredWidth: width
        Layout.alignment: Qt.AlignHCenter

        logo: "qrc:/ui/resources/app-rating.svg"
        logoTitle: VPNl18n.FeedbackFormReviewHeader
        logoSubtitle: VPNl18n.FeedbackFormReviewBody
    }

    Item {
        Layout.fillHeight: window.fullscreenRequired()
    }

    VPNButton {
        id: leaveReviewBtn

        Layout.fillWidth: true
        Layout.topMargin: VPNTheme.theme.vSpacing
        Layout.leftMargin: VPNTheme.theme.windowMargin * 2
        Layout.rightMargin: VPNTheme.theme.windowMargin * 2

        text: VPNl18n.FeedbackFormLeaveReviewButton
        onClicked: {
            VPN.openAppStoreReviewLink();
            getHelpStackView.push("qrc:/ui/screens/getHelp/giveFeedback/ViewGiveFeedbackThankYou.qml");
        }
    }

    VPNLinkButton {
        id: skipLink

        Layout.fillWidth: true
        Layout.topMargin: VPNTheme.theme.vSpacingSmall
        Layout.leftMargin: VPNTheme.theme.windowMargin * 2
        Layout.rightMargin: VPNTheme.theme.windowMargin * 2
        Layout.bottomMargin: navbar.visible ? VPNTheme.theme.navBarHeightWithMargins : 34

        labelText: qsTrId("vpn.feedbackForm.skip")
        onClicked: getHelpStackView.push("qrc:/ui/screens/getHelp/giveFeedback/ViewGiveFeedbackThankYou.qml");
        implicitHeight: VPNTheme.theme.rowHeight
    }

    Item {
        Layout.fillHeight: !window.fullscreenRequired()
    }
}


