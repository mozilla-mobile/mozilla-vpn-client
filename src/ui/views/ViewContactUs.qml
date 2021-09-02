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
    id: contactUsRoot

    function tryAgain() {
        contactUsStackView.pop();
        contactUsStackView.pop();
    }


    function createSupportTicket(email, subject, issueText, category) {
        contactUsStackView.push("../components/VPNLoader.qml", {
            footerLinkIsVisible: false
        });
        VPN.createSupportTicket(email, subject, issueText, category);
    }

    VPNMenu {
        id: menu
        //% "Contact support"
        title: VPNl18n.tr(VPNl18n.InAppSupportWorkflowSupportNavLinkText)

        // this view gets pushed to mainStackView from backend always
        // and so should be removed from mainStackView (even in settings flow) on back clicks
        isMainView: true
    }

    StackView {
        initialItem: contactUsView
        id: contactUsStackView
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.top: menu.bottom

        Connections {
            target: VPN
            function onTicketCreationAnswer(successful) {
                if(successful) {
                    contactUsStackView.push(thankYouView);
                } else {
                    contactUsStackView.push("../views/ViewErrorFullScreen.qml", {
                        //% "Error submitting your support request..."
                        headlineText: VPNl18n.tr(VPNl18n.InAppSupportWorkflowSupportErrorHeader),

                        //% "An unexpected error has occured, please try again."
                        errorMessage: VPNl18n.tr(VPNl18n.InAppSupportWorkflowSupportErrorText),

                        //% "Try again"
                        buttonText: VPNl18n.tr(VPNl18n.InAppSupportWorkflowSupportErrorButton),
                        buttonOnClick: contactUsRoot.tryAgain,
                        buttonObjectName: "errorTryAgainButton"
                        }
                    );
                }
            }
        }
    }

    Component {
        id: contactUsView
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
                anchors.topMargin: window.fullscreenRequired() ? Theme.contentTopMarginMobile : Theme.contentTopMarginDesktop


                ColumnLayout {
                    Layout.alignment: Qt.AlignTop
                    Layout.preferredWidth: parent.width

                    spacing: 24

                    ColumnLayout {
                        Layout.fillHeight: true
                        spacing: 24
                        visible: !VPN.userAuthenticated

                        ColumnLayout {
                            spacing: 10
                            VPNBoldLabel {
                                //% "Enter your email"
                                property string enterEmailAddress: VPNl18n.tr(VPNl18n.InAppSupportWorkflowSupportEmailFieldLabel)

                                text: enterEmailAddress
                                lineHeight: 10
                                lineHeightMode: Text.FixedHeight
                                wrapMode: Text.WordWrap
                                verticalAlignment: Text.AlignVCenter
                                Layout.fillWidth: true
                            }

                            VPNTextField {
                                id: emailInput

                                width: parent.width
                                verticalAlignment: Text.AlignVCenter
                                Layout.fillWidth: true
                                hasError: !VPNAuthInApp.validateEmailAddress(emailInput.text)
                                //% "Email address"
                                placeholderText: VPNl18n.tr(VPNl18n.InAppSupportWorkflowSupportFieldPlaceholder)
                            }
                        }

                        VPNTextField {
                            id: confirmEmailInput

                            width: parent.width
                            verticalAlignment: Text.AlignVCenter
                            Layout.fillWidth: true
                            hasError: !VPNAuthInApp.validateEmailAddress(confirmEmailInput.text) || emailInput.text != confirmEmailInput.text
                            //% "Confirm email address"
                            placeholderText: VPNl18n.tr(VPNl18n.InAppSupportWorkflowSupportConfirmEmailPlaceholder)
                        }
                    }

                    ColumnLayout {
                        RowLayout {
                            visible: VPN.userAuthenticated
                            spacing: 15
                            Layout.fillWidth: true
                            Layout.bottomMargin: 15

                            Rectangle {
                                Layout.preferredWidth: 40
                                Layout.preferredHeight: 40
                                color: "transparent"

                                VPNAvatar {
                                    id: avatar

                                    avatarUrl: VPNUser.avatar
                                    anchors.fill: parent
                                }
                            }

                            ColumnLayout {

                                VPNBoldLabel {
                                    //% "VPN User"
                                    readonly property var textVpnUser: qsTrId("vpn.settings.user")
                                    text: VPNUser.displayName ? VPNUser.displayName : textVpnUser

                                }


                                VPNLightLabel {
                                    id: serverLocation
                                    text: VPNUser.email
                                    Accessible.ignored: true
                                    Layout.alignment: Qt.AlignLeft
                                    elide: Text.ElideRight
                                }
                            }
                        }

                        spacing: 10

                        VPNBoldLabel {
                            //% "How can we help you with Mozilla VPN?"
                            property string enterEmailAddress: VPNl18n.tr(VPNl18n.InAppSupportWorkflowSupportFieldHeader)

                            text: enterEmailAddress
                            lineHeight: 10
                            lineHeightMode: Text.FixedHeight
                            wrapMode: Text.WordWrap
                            verticalAlignment: Text.AlignVCenter
                            Layout.fillWidth: true
                        }

                        VPNComboBox {
                            id: dropDown
                            placeholderText: VPNl18n.tr(VPNl18n.InAppSupportWorkflowDropdownLabel)
                            model: VPNSupportCategoryModel
                        }
                    }

                    VPNTextField {
                        id: subjectInput

                        width: parent.width
                        verticalAlignment: Text.AlignVCenter
                        Layout.fillWidth: true
                        //% "Subject (optional)"
                        placeholderText: VPNl18n.tr(VPNl18n.InAppSupportWorkflowSubjectFieldPlaceholder)
                    }

                    VPNTextArea {
                        id: textArea
                        //% "Describe issue..."
                        placeholderText: VPNl18n.tr(VPNl18n.InAppSupportWorkflowIssueFieldPlaceholder)
                    }
                }

                ColumnLayout {
                    Layout.fillHeight: true
                    spacing: 24

                    VPNVerticalSpacer {
                        Layout.fillWidth: true
                        Layout.minimumHeight: 16
                        Layout.fillHeight: !window.fullscreenRequired()
                    }

                    Column {
                        spacing: 0
                        Layout.fillWidth: true


                        VPNTextBlock {
                            font.pixelSize: Theme.fontSize
                            horizontalAlignment: Text.AlignHCenter
                            //% "When you submit, Mozilla VPN will collect technical and interaction data with your email to help our support team understand your issue."
                            text: VPNl18n.tr(VPNl18n.InAppSupportWorkflowDisclaimerText)
                            width:parent.width
                        }

                        VPNLinkButton {
                            //% "Mozilla VPN Privacy Notice"
                            labelText: VPNl18n.tr(VPNl18n.InAppSupportWorkflowPrivacyNoticeLinkText)
                            Layout.alignment: Qt.AlignHCenter
                            onClicked: VPN.openLink(VPN.LinkPrivacyNotice)
                            width: parent.width
                        }
                    }

                    ColumnLayout {
                        spacing: Theme.windowMargin

                        VPNButton {
                             //% "Submit"
                            text: VPNl18n.tr(VPNl18n.InAppSupportWorkflowSupportPrimaryButtonText)
                            onClicked: contactUsRoot.createSupportTicket(emailInput.text, subjectInput.text, textArea.userEntry, dropDown.currentValue);
                            enabled: dropDown.currentValue != null && textArea.userEntry != "" &&
                                     (VPN.userAuthenticated ? true :
                                        (VPNAuthInApp.validateEmailAddress(emailInput.text) && emailInput.text == confirmEmailInput.text)
                                     )
                            opacity: enabled ? 1 : .5
                            Layout.preferredHeight: Theme.rowHeight
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
                            //% "Cancel"
                            labelText: VPNl18n.tr(VPNl18n.InAppSupportWorkflowSupportSecondaryActionText)
                            Layout.preferredHeight: Theme.rowHeight
                            Layout.alignment: Qt.AlignHCenter
                            onClicked: mainStackView.pop()
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
                id: col
                anchors.top: parent.top
                anchors.topMargin: window.height * .10
                anchors.horizontalCenter: parent.horizontalCenter
                width: Math.min(Theme.maxHorizontalContentWidth, parent.width - Theme.windowMargin * 4)
                VPNPanel {
                    id: panel
                    logo: "../resources/heart-check.svg"
                    //% "Thank you!"
                    logoTitle: VPNl18n.tr(VPNl18n.InAppSupportWorkflowSupportResponseHeader)
                    //% "We appreciate your feedback. You’re helping us improve Mozilla VPN."
                    logoSubtitle: VPNl18n.tr(VPNl18n.InAppSupportWorkflowSupportResponseBody)
                    anchors.horizontalCenter: undefined
                    Layout.fillWidth: true
                }
            }
            VPNButton {
                //% "Done"
               text: VPNl18n.tr(VPNl18n.InAppSupportWorkflowSupportResponseButton)
               anchors.top: col.bottom
               anchors.topMargin: Theme.vSpacing
               anchors.horizontalCenter: parent.horizontalCenter
               onClicked: mainStackView.pop()
               Component.onCompleted: {
                 if (window.fullscreenRequired()) {
                     anchors.top = undefined;
                     anchors.topMargin = undefined;
                     anchors.bottom= parent.bottom
                     anchors.bottomMargin = Theme.windowMargin * 4
                 }
               }
            }
        }
    }
}
