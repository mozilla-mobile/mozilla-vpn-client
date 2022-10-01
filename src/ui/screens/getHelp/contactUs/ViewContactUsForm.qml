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
    objectName: "contactUs"
    _menuTitle: VPNl18n.InAppSupportWorkflowSupportNavLinkText
   _viewContentData: ColumnLayout {
       property string _emailAddress: ""

       id: contactUsRoot

       function createSupportTicket(email, subject, issueText, category) {
           getHelpStackView.push("qrc:/nebula/components/VPNLoader.qml", {
               footerLinkIsVisible: false
           });
           VPN.createSupportTicket(email, subject, issueText, category);
       }

       function fxaBrowserLink() {
           VPNUrlOpener.openLink(VPNUrlOpener.LinkHelpSupport);
           contactUsRoot.tryAgain();
       }

       Connections {
           target: VPN
           function onTicketCreationAnswer(successful) {
               if(successful) {
                   getHelpStackView.replace("qrc:/ui/screens/getHelp/contactUs/ViewContactUsThankYou.qml", {_emailAddress: contactUsRoot._emailAddress}, StackView.Immediate);
               } else {
                   // TODO: Navigator.GetTheThing
                   getHelpStackView.replace("qrc:/ui/sharedViews/ViewErrorFullScreen.qml", {
                       headlineText: VPNl18n.InAppSupportWorkflowSupportErrorHeader,
                       errorMessage: VPNl18n.InAppSupportWorkflowSupportErrorText,
                       primaryButtonText: VPNl18n.InAppSupportWorkflowSupportErrorButton,
                       primaryButtonOnClick: contactUsRoot.tryAgain,
                       primaryButtonObjectName: "errorTryAgainButton",
                       secondaryButtonIsSignOff: false,
                       secondaryButtonText: VPNl18n.InAppSupportWorkflowSupportErrorBrowserButton,
                       secondaryButtonObjectName: "errorFxALinkButton",
                       secondaryButtonOnClick: contactUsRoot.fxaBrowserLink
                       }
                   );
               }
           }
       }

        spacing: VPNTheme.theme.windowMargin
        Layout.leftMargin: VPNTheme.theme.windowMargin
        Layout.rightMargin: VPNTheme.theme.windowMargin

        ColumnLayout {
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: parent.width

            spacing: 24

            ColumnLayout {
                objectName: "contactUs-unauthedUserInputs"
                spacing: 24
                visible: VPN.userState !== VPN.UserAuthenticated
                Layout.alignment: Qt.AlignHCenter
                Layout.fillWidth: true
                Layout.maximumWidth: parent.width - VPNTheme.theme.windowMargin

                ColumnLayout {
                    spacing: 10
                    VPNBoldLabel {
                        property string enterEmailAddress: VPNl18n.InAppSupportWorkflowSupportEmailFieldLabel

                        text: enterEmailAddress
                        lineHeight: VPNTheme.theme.labelLineHeight
                        lineHeightMode: Text.FixedHeight
                        wrapMode: Text.WordWrap
                        verticalAlignment: Text.AlignVCenter
                        Layout.fillWidth: true
                        width: undefined

                    }

                    VPNTextField {
                        id: emailInput

                        verticalAlignment: Text.AlignVCenter
                        Layout.fillWidth: true
                        hasError: !VPNAuthInApp.validateEmailAddress(emailInput.text)
                        _placeholderText: VPNl18n.InAppSupportWorkflowSupportEmailFieldPlaceholder
                    }
                }

                VPNTextField {
                    id: confirmEmailInput

                    width: parent.width
                    verticalAlignment: Text.AlignVCenter
                    Layout.fillWidth: true
                    hasError: !VPNAuthInApp.validateEmailAddress(confirmEmailInput.text) || emailInput.text != confirmEmailInput.text
                    _placeholderText: VPNl18n.InAppSupportWorkflowSupportConfirmEmailPlaceholder
                }
            }

            VPNUserProfile {
                objectName: "contactUs-userInfo"
                enabled: false
                Layout.preferredWidth: parent.width
                visible: VPN.userState === VPN.UserAuthenticated
                Layout.topMargin: -VPNTheme.theme.windowMargin / 2
                Layout.bottomMargin: -VPNTheme.theme.windowMargin / 2
                anchors {
                    left: undefined
                    right: undefined
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.preferredWidth: parent.width - VPNTheme.theme.windowMargin
                Layout.maximumWidth: parent.width - VPNTheme.theme.windowMargin
                Layout.alignment: Qt.AlignHCenter
                spacing: 10

                VPNBoldLabel {
                    property string enterEmailAddress: VPNl18n.InAppSupportWorkflowSupportFieldHeader

                    text: enterEmailAddress
                    lineHeight: VPNTheme.theme.labelLineHeight
                    lineHeightMode: Text.FixedHeight
                    wrapMode: Text.WordWrap
                    verticalAlignment: Text.AlignVCenter
                    Layout.preferredWidth: parent.width
                    width: undefined
                }

                VPNComboBox {
                    id: dropDown
                    placeholderText: VPNl18n.InAppSupportWorkflowDropdownLabel
                    model: VPNSupportCategoryModel
                    Layout.preferredWidth: parent.width
                }
            }

            VPNTextField {
                id: subjectInput

                verticalAlignment: Text.AlignVCenter
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: parent.width - VPNTheme.theme.windowMargin
                _placeholderText: VPNl18n.InAppSupportWorkflowSubjectFieldPlaceholder
            }

            VPNTextArea {
                id: textArea
                placeholderText: VPNl18n.InAppSupportWorkflowIssueFieldPlaceholder
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: parent.width - VPNTheme.theme.windowMargin
            }
        }

        ColumnLayout {
            Layout.preferredWidth: parent.width - VPNTheme.theme.windowMargin * 2
            Layout.alignment: Qt.AlignHCenter
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
                    font.pixelSize: VPNTheme.theme.fontSize
                    horizontalAlignment: Text.AlignHCenter
                    text: VPNl18n.InAppSupportWorkflowDisclaimerText
                    width:parent.width

                }

                VPNLinkButton {
                    labelText: VPNl18n.InAppSupportWorkflowPrivacyNoticeLinkText
                    Layout.alignment: Qt.AlignHCenter
                    onClicked: VPNUrlOpener.openLink(VPNUrlOpener.LinkPrivacyNotice)
                    width: parent.width
                }
            }

            ColumnLayout {
                spacing: VPNTheme.theme.windowMargin

                VPNButton {
                    text: VPNl18n.InAppSupportWorkflowSupportPrimaryButtonText
                    onClicked: {
                      contactUsRoot._emailAddress = (VPN.userState === VPN.UserAuthenticated ? VPNUser.email : emailInput.text);
                      contactUsRoot.createSupportTicket(contactUsRoot._emailAddress, subjectInput.text, textArea.userEntry, dropDown.currentValue);
                    }
                    enabled: dropDown.currentValue != null && textArea.userEntry != "" &&
                             (VPN.userState === VPN.UserAuthenticated ? true :
                                (VPNAuthInApp.validateEmailAddress(emailInput.text) && emailInput.text == confirmEmailInput.text)
                             )
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

                VPNCancelButton {
                    Layout.minimumHeight: VPNTheme.theme.rowHeight
                    Layout.fillWidth: true
                    onClicked: getHelpStackView.pop()
                }
            }
        }
    }
}
