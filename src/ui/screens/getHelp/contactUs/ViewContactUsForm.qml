/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1

MZViewBase {
    id: vpnFlickable

    objectName: "contactUs"
    _menuTitle: MZI18n.InAppSupportWorkflowSupportNavLinkText
   _viewContentData: ColumnLayout {
       property string _emailAddress: ""
       Layout.preferredWidth: parent.width

       id: contactUsRoot

       function createSupportTicket(email, subject, issueText, category, shareLogs) {
           getHelpStackView.push("qrc:/nebula/components/MZLoader.qml", {
               footerLinkIsVisible: false
           });
           VPN.createSupportTicket(email, subject, issueText, category, shareLogs);
       }

       Connections {
           target: VPN
           function onTicketCreationAnswer(successful) {
               if(successful) {
                   getHelpStackView.replace("qrc:/qt/qml/Mozilla/VPN/screens/getHelp/contactUs/ViewContactUsThankYou.qml", {_emailAddress: contactUsRoot._emailAddress}, StackView.Immediate);
               } else {
                   // TODO: Navigator.GetTheThing
                   getHelpStackView.replace("qrc:/qt/qml/Mozilla/VPN/sharedViews/ViewErrorFullScreen.qml", {
                       headlineText: MZI18n.InAppSupportWorkflowSupportErrorHeader,
                       errorMessage: MZI18n.InAppSupportWorkflowSupportErrorText,
                       primaryButtonText: MZI18n.InAppSupportWorkflowSupportErrorButton,
                       primaryButtonOnClick: () => getHelpStackView.pop(),
                       primaryButtonObjectName: "errorTryAgainButton",
                       secondaryButtonIsSignOff: false,
                       secondaryButtonText: MZI18n.InAppSupportWorkflowSupportErrorBrowserButton,
                       secondaryButtonObjectName: "errorFxALinkButton",
                       secondaryButtonOnClick: () => {
                               MZUrlOpener.openUrlLabel("sumo");
                               getHelpStackView.pop();
                           },
                       }
                   );
               }
           }
       }

        spacing: MZTheme.theme.windowMargin
        Layout.leftMargin: MZTheme.theme.windowMargin * 2
        Layout.rightMargin: MZTheme.theme.windowMargin * 2

        ColumnLayout {
            Layout.alignment: Qt.AlignTop

            spacing: 24

            ColumnLayout {
                objectName: "contactUs-unauthedUserInputs"
                spacing: 10
                visible: !VPN.userAuthenticated
                Layout.alignment: Qt.AlignHCenter

                MZBoldLabel {
                    property string enterEmailAddress: MZI18n.InAppSupportWorkflowSupportEmailFieldLabel

                    text: enterEmailAddress
                    lineHeight: MZTheme.theme.labelLineHeight
                    lineHeightMode: Text.FixedHeight
                    wrapMode: Text.WordWrap
                    verticalAlignment: Text.AlignVCenter
                    Layout.fillWidth: true
                    width: undefined

                }

                MZTextField {
                    id: emailInput
                    objectName: "contactUs-emailInput"

                    verticalAlignment: Text.AlignVCenter
                    Layout.fillWidth: true
                    hasError: !MZValidator.validateEmailAddress(emailInput.text)
                    _placeholderText: MZI18n.InAppSupportWorkflowSupportEmailFieldPlaceholder
                }
            }

            MZUserProfile {
                objectName: "contactUs-userInfo"
                enabled: false
                Layout.fillWidth: true
                visible: VPN.userAuthenticated
                Layout.topMargin: -MZTheme.theme.windowMargin / 2
                Layout.bottomMargin: -MZTheme.theme.windowMargin / 2
                Layout.leftMargin: undefined
                Layout.rightMargin: undefined
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter
                spacing: 10

                MZBoldLabel {
                    property string enterEmailAddress: MZI18n.InAppSupportWorkflowSupportFieldHeader

                    text: enterEmailAddress
                    lineHeight: MZTheme.theme.labelLineHeight
                    lineHeightMode: Text.FixedHeight
                    wrapMode: Text.WordWrap
                    verticalAlignment: Text.AlignVCenter
                    Layout.fillWidth: true
                    width: undefined
                }

                MZComboBox {
                    id: dropDown
                    objectName: "contactUs-dropDown"
                    placeholderText: MZI18n.InAppSupportWorkflowDropdownLabel
                    model: VPNSupportCategoryModel
                    Layout.fillWidth: true
                    Layout.preferredWidth: undefined
                    onCurrentValueChanged: {
                        shareLogsCheckBox.isChecked = currentValue === 'account' || currentValue === 'technical';
                    }
                }
            }

            MZTextField {
                id: subjectInput
                objectName: "contactUs-subjectInput"
                verticalAlignment: Text.AlignVCenter
                Layout.alignment: Qt.AlignHCenter
                Layout.fillWidth: true
                _placeholderText: MZI18n.InAppSupportWorkflowSubjectFieldPlaceholder
            }

            MZTextArea {
                id: textArea
                objectName: "contactUs-textArea"
                placeholderText: MZI18n.InAppSupportWorkflowIssueFieldPlaceholder
                Layout.alignment: Qt.AlignHCenter
                Layout.fillWidth: true
            }

            MZCheckBoxRow {
                id: shareLogsCheckBox
                objectName: "contactUs-shareLogsCheckBox"
                labelText: MZI18n.InAppSupportWorkflowShareLogsCheckBoxTitle
                subLabelText: MZI18n.InAppSupportWorkflowShareLogsCheckBoxSubtitle
                Layout.alignment: Qt.AlignHCenter
                Layout.fillWidth: true
                Layout.rightMargin: MZTheme.theme.windowMargin
                leftMargin: 0
                showDivider: false
                onClicked: () => isChecked = !isChecked
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            Layout.fillHeight: true
            spacing: 24

            MZLinkButton {
                anchors.left: parent.left
                anchors.right: parent.right
                labelText: MZI18n.InAppSupportWorkflowPrivacyNoticeLinkText
                onClicked: MZUrlOpener.openUrlLabel("privacyNotice")
            }

            ColumnLayout {
                spacing: MZTheme.theme.windowMargin

                MZButton {
                    objectName: "contactUs-submitButton"
                    text: MZI18n.InAppSupportWorkflowSupportPrimaryButtonText
                    onClicked: {
                      contactUsRoot._emailAddress = (VPN.userAuthenticated ? VPNUser.email : emailInput.text);
                      contactUsRoot.createSupportTicket(contactUsRoot._emailAddress, subjectInput.text, textArea.userEntry, dropDown.currentValue, shareLogsCheckBox.isChecked);
                    }
                    enabled: dropDown.currentValue != null && textArea.userEntry != "" &&
                             (VPN.userAuthenticated  ? true :
                                MZValidator.validateEmailAddress(emailInput.text)
                             )
                    opacity: enabled ? 1 : .5
                    Layout.preferredHeight: MZTheme.theme.rowHeight
                    Layout.fillWidth: true
                    width: undefined
                    height: undefined
                    Behavior on opacity {
                        PropertyAnimation {
                            duration: 100
                        }
                    }
                }

                MZCancelButton {
                    Layout.minimumHeight: MZTheme.theme.rowHeight
                    Layout.preferredWidth: width
                    Layout.alignment: Qt.AlignHCenter
                    onClicked: getHelpStackView.pop()
                }
            }
        }
    }
}
