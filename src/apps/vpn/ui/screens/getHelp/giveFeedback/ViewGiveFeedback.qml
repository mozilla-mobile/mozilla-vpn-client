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
    property string _menuTitle: qsTrId("vpn.settings.giveFeedback")

    objectName: "giveFeedbackView"

    anchors.top: parent.top
    anchors.bottom: parent.bottom

    spacing: 0

    ButtonGroup {
        id: btnGroup
        buttons: row.children
    }

    VPNBoldLabel {
        Layout.fillWidth: true
        Layout.topMargin: window.fullscreenRequired() ? VPNTheme.theme.windowMargin * 3 : VPNTheme.theme.vSpacing
        Layout.leftMargin: VPNTheme.theme.windowMargin * 2
        Layout.rightMargin: VPNTheme.theme.windowMargin * 2

        //% "How would you describe your Mozilla VPN experience so far?"
        text: qsTrId("vpn.feedbackForm.ratingHeadline")
        lineHeight: 24
        lineHeightMode: Text.FixedHeight
        wrapMode: Text.WordWrap
        verticalAlignment: Text.AlignBottom
    }

    RowLayout {
        id: row

        Layout.topMargin: VPNTheme.theme.windowMargin * 2
        Layout.leftMargin: VPNTheme.theme.windowMargin * 2
        Layout.rightMargin: VPNTheme.theme.windowMargin * 2
        Layout.preferredHeight: VPNTheme.theme.rowHeight

        GiveFeedbackRadioDelegate {
            //% "Very poor"
            Accessible.name: qsTrId("vpn.feedbackForm.veryPoor")
            iconSource: "qrc:/ui/resources/faces/veryPoor.svg"
            value: 1
        }
        VPNVerticalSpacer {
            Layout.preferredHeight: 1
            Layout.fillWidth: true
        }

        GiveFeedbackRadioDelegate {
            //% "Poor"
            Accessible.name: qsTrId("vpn.feedbackForm.poor")
            iconSource: "qrc:/ui/resources/faces/poor.svg"
            value: 2
        }

        VPNVerticalSpacer {
            Layout.preferredHeight: 1
            Layout.fillWidth: true
        }

        GiveFeedbackRadioDelegate {
            //% "Average"
            Accessible.name: qsTrId("vpn.feedbackForm.average")
            iconSource: "qrc:/ui/resources/faces/average.svg"
            value: 3
        }

        VPNVerticalSpacer {
            Layout.preferredHeight: 1
            Layout.fillWidth: true
        }

        GiveFeedbackRadioDelegate {
            //% "Good"
            Accessible.name: qsTrId("vpn.feedbackForm.good")
            iconSource: "qrc:/ui/resources/faces/good.svg"
            value: 4
        }

        VPNVerticalSpacer {
            Layout.preferredHeight: 1
            Layout.fillWidth: true
        }

        GiveFeedbackRadioDelegate {
            Accessible.name: VPNI18n.FeedbackFormExcellentLabel
            iconSource: "qrc:/ui/resources/faces/veryGood.svg"
            value: 5
        }
    }

    RowLayout {
        Layout.topMargin: VPNTheme.theme.vSpacingSmall
        Layout.leftMargin: VPNTheme.theme.windowMargin * 2
        Layout.rightMargin: VPNTheme.theme.windowMargin * 2

        VPNInterLabel {
            horizontalAlignment: Qt.AlignLeft
            text: qsTrId("vpn.feedbackForm.veryPoor")
            Layout.fillWidth: true
            color: VPNTheme.theme.fontColor
        }

        Item {
            Layout.fillWidth: true
        }

        VPNInterLabel {
            horizontalAlignment: Qt.AlignRight
            text: VPNI18n.FeedbackFormExcellentLabel
            Layout.fillWidth: true
            color: VPNTheme.theme.fontColor
        }
    }

    Item {
        Layout.fillHeight: window.fullscreenRequired()
        Layout.preferredHeight: window.fullscreenRequired() ? undefined : VPNTheme.theme.windowMargin * 2
    }


    VPNButton {
        id: feebackContinueButton

        Layout.fillWidth: true
        Layout.leftMargin: VPNTheme.theme.windowMargin * 2
        Layout.rightMargin: VPNTheme.theme.windowMargin * 2
        Layout.bottomMargin: VPNTheme.theme.navBarHeightWithMargins

        //% "Continue"
        text: qsTrId("vpn.feedbackForm.continue")
        enabled: btnGroup.checkedButton !== null
        opacity: enabled ? 1 : .5

        onClicked: {
            if (btnGroup.checkedButton.value === null) {
                // This should not happen.
                return;
            }

            if (btnGroup.checkedButton.value >= 5) {
                VPN.submitFeedback("", btnGroup.checkedButton.value, 0);

                if (VPNFeatureList.get("appReview").isSupported) {
                    getHelpStackView.push("qrc:/ui/screens/getHelp/giveFeedback/ViewGiveFeedbackReview.qml");
                    return;
                }

                getHelpStackView.push("qrc:/ui/screens/getHelp/giveFeedback/ViewGiveFeedbackThankYou.qml");
                return;
            }

            getHelpStackView.push("qrc:/ui/screens/getHelp/giveFeedback/ViewGiveFeedbackForm.qml", { "appRating": btnGroup.checkedButton.value })
        }

        Behavior on opacity {
            PropertyAnimation {
                duration: 100
            }
        }
    }

    Item {
        Layout.fillHeight: !window.fullscreenRequired()
    }
}
