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
    objectName: "giveFeedbackView"

    _menuTitle: qsTrId("vpn.settings.giveFeedback")

    _viewContentData: ColumnLayout {
        Layout.fillHeight: true
        Layout.fillWidth: true
        Layout.leftMargin: VPNTheme.theme.windowMargin * 1.5
        Layout.rightMargin: VPNTheme.theme.windowMargin * 1.5

        ButtonGroup {
            id: btnGroup
            buttons: row.children
        }

        ColumnLayout {
            Layout.alignment: window.fullscreenRequired() ? Qt.AlignVCenter : Qt.AlignTop
            spacing: window.fullscreenRequired() ? VPNTheme.theme.windowMargin : VPNTheme.theme.windowMargin * 2

            VPNBoldLabel {
                //% "How would you describe your Mozilla VPN experience so far?"
                text: qsTrId("vpn.feedbackForm.ratingHeadline")
                lineHeight: 24
                lineHeightMode: Text.FixedHeight
                Layout.preferredWidth: parent.width
                wrapMode: Text.WordWrap
                verticalAlignment: Text.AlignBottom
            }

            ColumnLayout {
                spacing: 8

                RowLayout {
                    id: row
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
                        Accessible.name: VPNl18n.FeedbackFormExcellentLabel
                        iconSource: "qrc:/ui/resources/faces/veryGood.svg"
                        value: 5
                    }
                }
                RowLayout {
                    VPNInterLabel {
                        Layout.alignment: Qt.AlignLeft
                        horizontalAlignment: Qt.AlignLeft
                        text: qsTrId("vpn.feedbackForm.veryPoor")
                        Layout.fillWidth: true
                        color: VPNTheme.theme.fontColor
                    }
                    VPNInterLabel {
                        Layout.alignment: Qt.AlignRight
                        horizontalAlignment: Qt.AlignRight
                        text: VPNl18n.FeedbackFormExcellentLabel
                        Layout.fillWidth: true
                        color: VPNTheme.theme.fontColor
                    }
                }
            }
        }
        VPNButton {
            id: feebackContinueButton

            //% "Continue"
            text: qsTrId("vpn.feedbackForm.continue")
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


            Layout.alignment: window.fullscreenRequired() ? Qt.AlignTop : Qt.AlignBottom
            Layout.topMargin: VPNTheme.theme.vSpacing
            Layout.fillWidth: true
            enabled: btnGroup.checkedButton !== null
            opacity: enabled ? 1 : .5
            Behavior on opacity {
                PropertyAnimation {
                    duration: 100
                }
            }
        }
    }
}
