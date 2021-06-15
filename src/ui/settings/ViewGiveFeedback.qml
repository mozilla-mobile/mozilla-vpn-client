

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
import QtQuick 2.5

import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../components"
import "../components/forms"
import "../themes/themes.js" as Theme

Item {
    property var rating: 0
    id: feedbackRoot

    VPNMenu {
        id: menu
        title: qsTrId("vpn.settings.giveFeedback")
        isSettingsView: true
    }

    StackView {
        id: feedbackStackView
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.top: menu.bottom
        initialItem: appRatingView
    }

    Component {
        id: appRatingView

        Item {
            id: root
            ButtonGroup {
                id: btnGroup
                buttons: row.children
            }

            ColumnLayout {
                id: col
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                spacing: Theme.windowMargin
                anchors.margins: Theme.windowMargin * 2
                anchors.topMargin: 20

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
                        Layout.preferredHeight: Theme.rowHeight
                        VPNFeedbackRadioDelegate {
                            //% "Very poor"
                            Accessible.name: qsTrId("vpn.feedbackForm.veryPoor")
                            iconSource: "../resources/connection-info-dark.svg"
                            value: 1
                        }
                        VPNVerticalSpacer {
                           Layout.preferredHeight: 1
                           Layout.fillWidth: true
                        }
                        VPNFeedbackRadioDelegate {
                            //% "Poor"
                            Accessible.name: qsTrId("vpn.feedbackForm.poor")
                            iconSource: "../resources/connection-info-dark.svg"
                            value: 2
                        }
                        VPNVerticalSpacer {
                           Layout.preferredHeight: 1
                           Layout.fillWidth: true
                        }
                        VPNFeedbackRadioDelegate {
                            //% "Average"
                            Accessible.name: qsTrId("vpn.feedbackForm.average")
                            iconSource: "../resources/connection-info-dark.svg"
                            value: 3
                        }
                        VPNVerticalSpacer {
                           Layout.preferredHeight: 1
                           Layout.fillWidth: true
                        }
                        VPNFeedbackRadioDelegate {
                            //% "Good"
                            Accessible.name: qsTrId("vpn.feedbackForm.good")
                            iconSource: "../resources/connection-info-dark.svg"
                            value: 4
                        }
                        VPNVerticalSpacer {
                           Layout.preferredHeight: 1
                           Layout.fillWidth: true
                        }
                        VPNFeedbackRadioDelegate {
                            //% "Very good"
                            Accessible.name: qsTrId("vpn.feedbackForm.veryGood")
                            iconSource: "../resources/connection-info-dark.svg"
                            value: 5
                        }
                    }
                    RowLayout {
                        VPNInterLabel {
                            Layout.alignment: Qt.AlignLeft
                            horizontalAlignment: Qt.AlignLeft
                            text: qsTrId("vpn.feedbackForm.poor")
                            Layout.fillWidth: true
                            color: Theme.fontColor
                        }
                        VPNInterLabel {
                            Layout.alignment: Qt.AlignRight
                            horizontalAlignment: Qt.AlignRight
                            text: qsTrId("vpn.feedbackForm.veryGood")
                            Layout.fillWidth: true
                            color: Theme.fontColor
                        }
                    }
                }
            }

            VPNButton {
                //% "Continue"
                text: qsTrId("vpn.feedbackForm.continue")
                onClicked: {
                    feedbackStackView.push(feedbackFormView, {
                                               "appRating": btnGroup.checkedButton.value
                                           })
                }
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottomMargin: Theme.windowMargin * 4
                enabled: btnGroup.checkedButton !== null
                opacity: enabled ? 1 : .5
                width: col.width
                Behavior on opacity {
                    PropertyAnimation {
                        duration: 100
                    }
                }
            }
        }
    }

    Component {
        id: feedbackFormView
        VPNFlickable {
            id: vpnFlickable
            property var appRating
            property var feedbackCategory
            flickContentHeight: col.childrenRect.height
            interactive: flickContentHeight > height

            Rectangle {
                anchors.fill: parent
                color: Theme.bgColor
            }

            ColumnLayout {
                id: col

                function sendFeedback() {
                    console.log("app_rating: "+ appRating + " /   category_index: " + dropDown.currentIndex + "  " + "/   comments:" + textArea.userEntry);
                    feedbackStackView.push(thankYouView)
                }

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                spacing: Theme.windowMargin
                anchors.margins: Theme.windowMargin * 2
                anchors.topMargin: 20
                width: Math.min(Theme.maxHorizontalContentWidth, parent.width - Theme.windowMargin * 4)
                height: window.height - menu.height

                ColumnLayout {
                    Layout.alignment: Qt.AlignTop
                    Layout.preferredWidth: parent.width
                    Layout.fillWidth: true
                    spacing: 24

                    VPNBoldLabel {
                        //% "We’re sorry to hear you’ve had a poor experience! Please let us know how we can improve."
                        property string lowRatingResponse: qsTrId("vpn.feedbackForm.lowRatingResponse")

                        //% "We’d love to know what we can do to improve Mozilla VPN. Please share any specific feedback here."
                        property string averageToHighRatingResponse: qsTrId("vpn.feedbackForm.averageToHighRatingResponse")
                        text: appRating >= 3 ? averageToHighRatingResponse : lowRatingResponse
                        lineHeight: 24
                        lineHeightMode: Text.FixedHeight
                        Layout.preferredWidth: parent.width
                        wrapMode: Text.WordWrap
                        verticalAlignment: Text.AlignVCenter
                    }

                    VPNComboBox {
                        id: dropDown
                         //% "Choose a Category"
                        placeholderText: qsTrId("vpn.feedbackForm.chooseCategory")

                        model: ListModel {
                            ListElement {
                                //% "Product Bugs/Errors"
                                name: qsTrId("vpn.feedbackForm.category1")
                            }
                            ListElement {
                                //% "Network Connection/Speed"
                                name: qsTrId("vpn.feedbackForm.category2")
                            }
                            ListElement {
                                //% "Product Quality"
                                name:qsTrId("vpn.feedbackForm.categor3")
                            }
                            ListElement {
                                //% "Access to service"
                                name: qsTrId("vpn.feedbackForm.category4")
                            }
                            ListElement {
                                //% "Compatability"
                                name: qsTrId("vpn.feedbackForm.category5")
                            }
                            ListElement {
                                //% "Ease of Use"
                                name: qsTrId("vpn.feedbackForm.category6")
                            }
                            ListElement {
                                //% "Other"
                                name: qsTrId("vpn.feedbackForm.category7")
                            }
                        }
                    }
                    VPNTextArea {
                        id: textArea
                        //% "Enter feedback…"
                        placeholderText: qsTrId("vpn.feedbackForm.textAreaPlaceholder")
                    }
                }

                ColumnLayout {
                    Layout.preferredWidth: col.width
                    Layout.fillHeight: true
                    spacing: 24

                    VPNVerticalSpacer {
                        Layout.fillWidth: true
                        Layout.minimumHeight: 16
                        Layout.fillHeight: !window.fullscreenRequired()
                    }
                    ColumnLayout {
                        spacing: 0

                        VPNTextBlock {
                            width: undefined
                            Layout.fillWidth: true
                            Layout.preferredWidth: parent.width
                            font.pixelSize: Theme.fontSize
                            horizontalAlignment: Text.AlignHCenter
                            //% "When you submit feedback, Mozilla VPN will send interaction data to Mozilla. This data isn't associated with your identity."
                            text: qsTrId("vpn.feedback.privacyBlurb")
                        }

                        VPNLinkButton {
                            //% "Mozilla VPN Privacy Notice"
                            labelText: qsTrId("vpn.feedbackForm.privacyNoticeLink")
                            Layout.alignment: Qt.AlignHCenter
                            onClicked: VPN.openLink(VPN.LinkPrivacyNotice)
                            Layout.preferredHeight: Theme.rowHeight
                        }
                    }

                    ColumnLayout {
                        spacing: Theme.windowMargin
                        Layout.preferredWidth: parent.width

                        VPNButton {
                             //% "Submit"
                            text: qsTrId("vpn.feedbackform.submit")
                            onClicked: col.sendFeedback()
                            Layout.preferredHeight: Theme.rowHeight
                            Layout.preferredWidth: parent.width
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
                            Layout.preferredHeight: Theme.rowHeight
                            Layout.alignment: Qt.AlignHCenter
                            onClicked: col.sendFeedback()
                            implicitHeight: Theme.rowHeight
                        }
                    }
                    VPNVerticalSpacer {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.minimumHeight: Theme.rowHeight * 2
                        Layout.maximumHeight: Layout.minimumHeight
                    }
                }
            }
        }
    }

    Component {
        id: thankYouView
        Item {
            ColumnLayout {
                anchors.top: parent.top
                anchors.topMargin: window.height * .13
                anchors.horizontalCenter: parent.horizontalCenter
                width: Math.min(Theme.maxHorizontalContentWidth, parent.width - Theme.windowMargin * 4)
                spacing: 24
                VPNPanel {
                    logo: "../resources/heart-check.svg"
                    //% Thank you!
                    logoTitle: qsTrId("vpn.feedbackForm.thankyou")
                    //% "We appreciate your feedback. You’re helping us improve Mozilla VPN."
                    logoSubtitle: qsTrId("vpn.feedbackForm.thankyouSubtitle")
                    anchors.horizontalCenter: undefined
                    Layout.fillWidth: true
                }
                VPNButton {
                    //% "Done"
                   text: qsTrId("vpn.feedbackform.done")
                   Layout.fillWidth: true
                   Layout.preferredHeight: Theme.rowHeight
                   onClicked: stackview.pop(StackView.Immediate)
                }
            }
        }
    }
}
