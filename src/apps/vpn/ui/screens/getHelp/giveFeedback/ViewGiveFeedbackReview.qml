/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import components 0.1
import components.forms 0.1

ColumnLayout {
    id: layout

    spacing: 0

    Item {
        Layout.fillHeight: window.fullscreenRequired()
    }

    MZPanel {
        anchors.horizontalCenter: undefined
        Layout.topMargin: window.fullscreenRequired() ? 0 : layout.height * 0.2
        Layout.preferredHeight: height
        Layout.preferredWidth: width
        Layout.alignment: Qt.AlignHCenter

        logo: "qrc:/ui/resources/app-rating.svg"
        logoTitle: MZI18n.FeedbackFormReviewHeader
        logoSubtitle: MZI18n.FeedbackFormReviewBody
    }

    Item {
        Layout.fillHeight: window.fullscreenRequired()
    }

    MZButton {
        id: leaveReviewBtn

        Layout.fillWidth: true
        Layout.topMargin: MZTheme.theme.vSpacing
        Layout.leftMargin: MZTheme.theme.windowMargin * 2
        Layout.rightMargin: MZTheme.theme.windowMargin * 2

        text: MZI18n.FeedbackFormLeaveReviewButton
        onClicked: {
            getHelpStackView.push("qrc:/ui/screens/getHelp/giveFeedback/ViewGiveFeedbackThankYou.qml", StackView.Immediate);
            MZUtils.openAppStoreReviewLink();
        }
    }

    MZLinkButton {
        id: skipLink

        Layout.fillWidth: true
        Layout.topMargin: MZTheme.theme.vSpacingSmall
        Layout.leftMargin: MZTheme.theme.windowMargin * 2
        Layout.rightMargin: MZTheme.theme.windowMargin * 2
        Layout.bottomMargin: navbar.visible ? MZTheme.theme.navBarHeightWithMargins : 34

        labelText: qsTrId("vpn.feedbackForm.skip")
        onClicked: getHelpStackView.push("qrc:/ui/screens/getHelp/giveFeedback/ViewGiveFeedbackThankYou.qml");
        implicitHeight: MZTheme.theme.rowHeight
    }

    Item {
        Layout.fillHeight: !window.fullscreenRequired()
    }
}


