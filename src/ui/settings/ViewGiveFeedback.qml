

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
    anchors.fill: parent

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
            ButtonGroup {
                id: btnGroup
                buttons: row.children
            }

            ColumnLayout {
                id: col
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                spacing: 20
                anchors.topMargin: 20
                anchors.margins: Theme.windowMargin * 2

                VPNBoldLabel {
                    //% "How would you describe your Mozilla VPN experience so far?"
                    text: qsTrId("vpn.feedbackForm.ratingHeadline")
                    lineHeight: 24
                    lineHeightMode: Text.FixedHeight
                    Layout.preferredWidth: parent.width
                    wrapMode: Text.WordWrap
                    Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
                }

                RowLayout {
                    id: row
                    Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
                    Layout.fillWidth: true
                    Layout.preferredWidth: parent.width

                    VPNFeedbackRadioDelegate {
                        iconSource: "../resources/connection-info-dark.svg"
                        Accessible.name: "So bad"
                        value: 1
                    }
                    VPNFeedbackRadioDelegate {
                        iconSource: "../resources/connection-info-dark.svg"
                        Accessible.name: "Bad"
                        value: 2
                    }
                    VPNFeedbackRadioDelegate {
                        iconSource: "../resources/connection-info-dark.svg"
                        Accessible.name: "Average"
                        value: 3
                    }
                    VPNFeedbackRadioDelegate {
                        iconSource: "../resources/connection-info-dark.svg"
                        Accessible.name: "Good"
                        value: 4
                    }
                    VPNFeedbackRadioDelegate {
                        iconSource: "../resources/connection-info-dark.svg"
                        Accessible.name: "Very good"
                        value: 5
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
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                spacing: Theme.windowMargin
                anchors.margins: Theme.windowMargin * 2
                anchors.topMargin: 20
                width: parent.width - Theme.windowMargin * 4
                height: window.height - menu.height

                ColumnLayout {
                    Layout.alignment: Qt.AlignTop
                    Layout.preferredWidth: parent.width
                    Layout.fillWidth: true
                    spacing: Theme.windowMargin

                    VPNBoldLabel {
                        text: {
                            switch (appRating) {
                            case 1:
                            case 2:
                                "We’re sorry to hear you’ve had a poor experience! Please let us know how we can improve."
                                break
                            case 3:
                                "We’d love to know what we can do to improve Mozilla VPN. Please share any specific feedback here."
                                break
                            case 4:
                            case 5:
                            default:
                                "Do we have this string?"
                            }
                        }
                        lineHeight: 24
                        lineHeightMode: Text.FixedHeight
                        Layout.preferredWidth: parent.width
                        wrapMode: Text.WordWrap
                    }

                    VPNComboBox {
                        id: dropDown
                         //% "Choose a Category"
                        placeholderText: qsTrId("vpn.feedbackForm.chooseCategory")

                        model: ListModel {
                            ListElement {
                                //% "Product Bugs/Errors"
                                name: qsTrId("vpn.feedbackForm.category1")
//                                value: "Product Bugs/Errors"
                            }
                            ListElement {
                                //% "Network Connection/Speed"
                                name: qsTrId("vpn.feedbackForm.category2")
//                                value: "Network Connection/Speed"
                            }
                            ListElement {
                                //% "Product Quality"
                                name:qsTrId("vpn.feedbackForm.categor3")
//                                value: "Product Quality"
                            }
                            ListElement {
                                //% "Access to service"
                                name: qsTrId("vpn.feedbackForm.category4")
//                                value: "Access to service"
                            }
                            ListElement {
                                //% "Compatability"
                                name: qsTrId("vpn.feedbackForm.category5")
//                                value: "Compatability"
                            }
                            ListElement {
                                //% "Ease of Use"
                                name: qsTrId("vpn.feedbackForm.category6")
//                                value: "Ease of Use"
                            }
                            ListElement {
                                //% "Other"
                                name: qsTrId("vpn.feedbackForm.category7")
//                                value: "Other"
                            }
                        }
                    }

                    VPNTextArea {
                        id: textArea
                        //% "Enter feedback…"
                        placeholderText: qsTrId("vpn.feedbackForm.textAreaPlaceholder")
                    }
                }

                VPNVerticalSpacer {
                    Layout.preferredHeight: 1
                }

                ColumnLayout {
                    Layout.preferredWidth: col.width
                    Layout.fillHeight: true
                    spacing: Theme.windowMargin

                    VPNVerticalSpacer {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }

                    function sendFeedback() {
                        console.log(appRating + " " + dropDown.currentValue + " " + textArea.text);
                        feedbackStackView.push(thankYouView)
                    }

                    VPNButton {
                         //% "Submit"
                        text: qsTrId("vpn.feedbackform.submit")
                        onClicked: parent.sendFeedback()
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
                        onClicked: parent.sendFeedback()
                        implicitHeight: Theme.rowHeight * 2

                    }

                    VPNVerticalSpacer {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.maximumHeight: Theme.rowHeight * 2
                    }
                }
            }
        }
    }

    Component {
        id: thankYouView
        Item {
            VPNInterLabel {
                text: "WIP"
                anchors.centerIn: parent
            }
        }
    }
}
