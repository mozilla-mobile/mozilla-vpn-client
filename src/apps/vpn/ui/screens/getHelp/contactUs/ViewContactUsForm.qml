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
    id: vpnFlickable

    objectName: "contactUs"
    _menuTitle: VPNI18n.InAppSupportWorkflowSupportNavLinkText
   _viewContentData: ColumnLayout {
       property string _emailAddress: ""

       id: contactUsRoot

       function createSupportTicket(email, subject, issueText, category) {
           getHelpStackView.push("qrc:/nebula/components/VPNLoader.qml", {
               footerLinkIsVisible: false
           });
           VPN.createSupportTicket(email, subject, issueText, category);
       }

       Connections {
           target: VPN
           function onTicketCreationAnswer(successful) {
               if(successful) {
                   getHelpStackView.replace("qrc:/ui/screens/getHelp/contactUs/ViewContactUsThankYou.qml", {_emailAddress: contactUsRoot._emailAddress}, StackView.Immediate);
               } else {
                   // TODO: Navigator.GetTheThing
                   getHelpStackView.replace("qrc:/ui/sharedViews/ViewErrorFullScreen.qml", {
                       headlineText: VPNI18n.InAppSupportWorkflowSupportErrorHeader,
                       errorMessage: VPNI18n.InAppSupportWorkflowSupportErrorText,
                       primaryButtonText: VPNI18n.InAppSupportWorkflowSupportErrorButton,
                       primaryButtonOnClick: () => getHelpStackView.pop(),
                       primaryButtonObjectName: "errorTryAgainButton",
                       secondaryButtonIsSignOff: false,
                       secondaryButtonText: VPNI18n.InAppSupportWorkflowSupportErrorBrowserButton,
                       secondaryButtonObjectName: "errorFxALinkButton",
                       secondaryButtonOnClick: () => {
                               VPNUrlOpener.openUrlLabel("sumo");
                               getHelpStackView.pop();
                           },
                       }
                   );
               }
           }
       }

        spacing: VPNTheme.theme.windowMargin
        Layout.leftMargin: VPNTheme.theme.windowMargin * 2
        Layout.rightMargin: VPNTheme.theme.windowMargin * 2

        ColumnLayout {
            Layout.alignment: Qt.AlignTop

            spacing: 24

            ColumnLayout {
                objectName: "contactUs-unauthedUserInputs"
                spacing: 24
                visible: VPN.userState !== VPN.UserAuthenticated
                Layout.alignment: Qt.AlignHCenter

                ColumnLayout {
                    spacing: 10
                    VPNBoldLabel {
                        property string enterEmailAddress: VPNI18n.InAppSupportWorkflowSupportEmailFieldLabel

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
                        _placeholderText: VPNI18n.InAppSupportWorkflowSupportEmailFieldPlaceholder
                    }
                }

                VPNTextField {
                    id: confirmEmailInput

                    verticalAlignment: Text.AlignVCenter
                    Layout.fillWidth: true
                    hasError: !VPNAuthInApp.validateEmailAddress(confirmEmailInput.text) || emailInput.text != confirmEmailInput.text
                    _placeholderText: VPNI18n.InAppSupportWorkflowSupportConfirmEmailPlaceholder
                }
            }

            VPNUserProfile {
                objectName: "contactUs-userInfo"
                enabled: false
                Layout.fillWidth: true
                visible: VPN.userState === VPN.UserAuthenticated
                Layout.topMargin: -VPNTheme.theme.windowMargin / 2
                Layout.bottomMargin: -VPNTheme.theme.windowMargin / 2
                Layout.leftMargin: undefined
                Layout.rightMargin: undefined
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter
                spacing: 10

                VPNBoldLabel {
                    property string enterEmailAddress: VPNI18n.InAppSupportWorkflowSupportFieldHeader

                    text: enterEmailAddress
                    lineHeight: VPNTheme.theme.labelLineHeight
                    lineHeightMode: Text.FixedHeight
                    wrapMode: Text.WordWrap
                    verticalAlignment: Text.AlignVCenter
                    Layout.fillWidth: true
                    width: undefined
                }

                VPNComboBox {
                    id: dropDown
                    placeholderText: VPNI18n.InAppSupportWorkflowDropdownLabel
                    model: VPNSupportCategoryModel
                    Layout.fillWidth: true
                    Layout.preferredWidth: undefined
                }
            }

            VPNTextField {
                id: subjectInput

                verticalAlignment: Text.AlignVCenter
                Layout.alignment: Qt.AlignHCenter
                Layout.fillWidth: true
                _placeholderText: VPNI18n.InAppSupportWorkflowSubjectFieldPlaceholder
            }

            VPNTextArea {
                id: textArea
                placeholderText: VPNI18n.InAppSupportWorkflowIssueFieldPlaceholder
                Layout.alignment: Qt.AlignHCenter
                Layout.fillWidth: true
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
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
                    text: VPNI18n.InAppSupportWorkflowDisclaimerText
                    anchors.left: parent.left
                    anchors.right: parent.right

                }

                VPNLinkButton {
                    anchors.horizontalCenter: parent.horizontalCenter
                    labelText: VPNI18n.InAppSupportWorkflowPrivacyNoticeLinkText
                    onClicked: VPNUrlOpener.openUrlLabel("privacyNotice")
                }
            }

            ColumnLayout {
                spacing: VPNTheme.theme.windowMargin

                VPNButton {
                    text: VPNI18n.InAppSupportWorkflowSupportPrimaryButtonText
                    onClicked: {
                      VPNGleanDeprecated.recordGleanEvent("supportCaseSubmitted");
                      Glean.sample.supportCaseSubmitted.record();
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
                    Layout.preferredWidth: width
                    Layout.alignment: Qt.AlignHCenter
                    onClicked: getHelpStackView.pop()
                }
            }
        }
    }
}
