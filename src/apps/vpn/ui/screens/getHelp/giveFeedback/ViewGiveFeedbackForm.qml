/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1


// This is the second view in the Give Feedback flow

VPNViewBase {
    property var appRating
    property var feedbackCategory

    id: vpnFlickable

    _menuTitle: qsTrId("vpn.settings.giveFeedback")
    _viewContentData: ColumnLayout {
        id: col

        Layout.fillWidth: true
        Layout.leftMargin: VPNTheme.theme.windowMargin * 1.5
        Layout.rightMargin: VPNTheme.theme.windowMargin * 1.5
        spacing: VPNTheme.theme.windowMargin

        ColumnLayout {
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: parent.width

            spacing: VPNTheme.theme.windowMargin * 1.5

            VPNBoldLabel {
                //% "We’re sorry to hear you’ve had a poor experience. Please let us know how we can improve."
                property string lowRatingResponse: qsTrId("vpn.feedbackForm.lowRatingResponse")

                //% "We’d love to know what we can do to improve Mozilla VPN. Please share any specific feedback here."
                property string averageRatingResponse: qsTrId("vpn.feedbackForm.averageRatingResponse")

                //% "We’re glad you’re enjoying Mozilla VPN! Please let us know how we can make your experience even better."
                property string goodRatingResponse: qsTrId("vpn.feedbackForm.goodRatingResponse")

                text: appRating < 3 ? lowRatingResponse : (appRating == 3 ? averageRatingResponse : goodRatingResponse)
                lineHeight:VPNTheme.theme.windowMargin * 1.5
                lineHeightMode: Text.FixedHeight
                wrapMode: Text.WordWrap
                verticalAlignment: Text.AlignVCenter
                Layout.fillWidth: true
            }

            VPNComboBox {
                id: dropDown
                placeholderText: VPNl18n.FeedbackFormChooseCategory
                model: VPNFeedbackCategoryModel
            }

            VPNTextArea {
                id: textArea
                //% "Enter feedback…"
                placeholderText: qsTrId("vpn.feedbackForm.textAreaPlaceholder")
            }
        }

        ColumnLayout {
            Layout.fillHeight: true
            spacing: VPNTheme.theme.windowMargin * 1.5

            VPNVerticalSpacer {
                Layout.fillWidth: true
                Layout.minimumHeight: VPNTheme.theme.windowMargin
                Layout.fillHeight: !window.fullscreenRequired()
            }

            Column {
                spacing: 0
                Layout.fillWidth: true

                VPNTextBlock {
                    font.pixelSize: VPNTheme.theme.fontSize
                    horizontalAlignment: Text.AlignHCenter
                    //% "When you submit feedback, Mozilla VPN will collect technical and interaction data, including your IP address, to help us improve. This data won’t be associated with your Firefox account."
                    text: qsTrId("vpn.feedback.privacyDisclaimer")
                    width:parent.width
                }

                VPNLinkButton {
                    //% "Mozilla VPN Privacy Notice"
                    labelText: qsTrId("vpn.feedbackForm.privacyNoticeLink")
                    Layout.alignment: Qt.AlignHCenter
                    onClicked: VPNUrlOpener.openLink(VPNUrlOpener.LinkPrivacyNotice)
                    width: parent.width
                }
            }

            ColumnLayout {
                spacing: VPNTheme.theme.windowMargin

                VPNButton {
                     //% "Submit"
                    text: qsTrId("vpn.feedbackform.submit")
                    onClicked: sendFeedback(appRating, dropDown.currentValue, textArea.userEntry);
                    enabled: dropDown.currentValue != null
                    opacity: enabled ? 1 : .5
                    Layout.preferredHeight: VPNTheme.theme.rowHeight
                    Layout.fillWidth: true
                    width: undefined
                    height: undefined
                    Behavior on opacity {
                        PropertyAnimation {
                            duration: 100
                        }
                    }
                }
                VPNLinkButton {
                    //% "Skip"
                    labelText: qsTrId("vpn.feedbackForm.skip")
                    Layout.preferredHeight: VPNTheme.theme.rowHeight
                    Layout.alignment: Qt.AlignHCenter
                    onClicked: sendFeedback(appRating, 0, "");
                    implicitHeight: VPNTheme.theme.rowHeight

                }
            }
        }
    }
    function sendFeedback(appRating, category, message) {
        VPN.submitFeedback(message, appRating, category);
        getHelpStackView.push("qrc:/ui/screens/getHelp/giveFeedback/ViewGiveFeedbackThankYou.qml")
    }
}
