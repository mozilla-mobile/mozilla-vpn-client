/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1

Item {
    property string _menuTitle: VPNl18n.InAppSupportWorkflowSupportNavLinkText
    property bool addSafeAreaMargin: false

    // This property is used to cache the emailAddress between the sub-views.
    property string emailAddress: ""

    id: contactUsRoot
    objectName: "contactUs"

    function tryAgain() {
        mainStackView.pop();
    }

    function createSupportTicket(email, subject, issueText, category) {
        mainStackView.push("qrc:/nebula/components/VPNLoader.qml", {
            footerLinkIsVisible: false
        });
        VPN.createSupportTicket(email, subject, issueText, category);
    }

    function fxaBrowserLink() {
        VPN.openLink(VPN.LinkHelpSupport);
        contactUsRoot.tryAgain();
    }

    VPNMenu {
        id: menu
        objectName: "supportTicketScreen"
        title: VPNl18n.InAppSupportWorkflowSupportNavLinkText
        anchors.top: parent.top
        visible: VPN.state !== VPN.StateMain
    }

    StackView {
        initialItem: contactUsView
        id: contactUsStackView
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.top: menu.visible ? menu.bottom : parent.top

        Connections {
            target: VPN
            function onTicketCreationAnswer(successful) {
                if(successful) {
                    mainStackView.replace(thankYouView, StackView.Immediate);
                } else {
                    mainStackView.replace("qrc:/ui/views/ViewErrorFullScreen.qml", {
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
    }

    Component {
        id: contactUsView
        VPNFlickable {
            id: vpnFlickable
            property var appRating
            property var feedbackCategory
            flickContentHeight: col.height + col.anchors.topMargin

            Rectangle {
                anchors.fill: parent
                color:  VPNTheme.theme.bgColor
            }

            ColumnLayout {
                id: col

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.topMargin: window.fullscreenRequired() ? VPNTheme.theme.contentTopMarginMobile : VPNTheme.theme.contentTopMarginDesktop
                anchors.leftMargin: VPNTheme.theme.windowMargin * 2
                anchors.rightMargin: VPNTheme.theme.windowMargin * 2

                ColumnLayout {
                    objectName: "contactUs-unauthedUserInputs"
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignHCenter
                    spacing: 0

                    visible: VPN.userState !== VPN.UserAuthenticated

                    VPNBoldLabel {
                        property string enterEmailAddress: VPNl18n.InAppSupportWorkflowSupportEmailFieldLabel

                        Layout.fillWidth: true

                        text: enterEmailAddress
                        lineHeight: VPNTheme.theme.labelLineHeight
                        lineHeightMode: Text.FixedHeight
                        wrapMode: Text.WordWrap
                        verticalAlignment: Text.AlignVCenter
                        width: undefined

                    }

                    VPNTextField {
                        id: emailInput

                        Layout.topMargin: 10
                        Layout.fillWidth: true

                        verticalAlignment: Text.AlignVCenter
                        hasError: !VPNAuthInApp.validateEmailAddress(emailInput.text)
                        _placeholderText: VPNl18n.InAppSupportWorkflowSupportEmailFieldPlaceholder
                    }

                    VPNTextField {
                        id: confirmEmailInput

                        Layout.topMargin: VPNTheme.theme.vSpacing
                        Layout.fillWidth: true

                        width: parent.width
                        verticalAlignment: Text.AlignVCenter
                        hasError: !VPNAuthInApp.validateEmailAddress(confirmEmailInput.text) || emailInput.text != confirmEmailInput.text
                        _placeholderText: VPNl18n.InAppSupportWorkflowSupportConfirmEmailPlaceholder
                    }
                }

                VPNUserProfile {
                    objectName: "contactUs-userInfo"

                    Layout.preferredWidth: parent.width
                    Layout.topMargin: VPNTheme.theme.vSpacing

                    enabled: false
                    visible: VPN.userState === VPN.UserAuthenticated

                    anchors.left: undefined
                    anchors.right: undefined
                    anchors.leftMargin: undefined
                    anchors.rightMargin: undefined
                }

                VPNBoldLabel {
                    property string enterEmailAddress: VPNl18n.InAppSupportWorkflowSupportFieldHeader

                    Layout.topMargin: VPNTheme.theme.vSpacing
                    Layout.preferredWidth: parent.width

                    text: enterEmailAddress
                    lineHeight: VPNTheme.theme.labelLineHeight
                    lineHeightMode: Text.FixedHeight
                    wrapMode: Text.WordWrap
                    verticalAlignment: Text.AlignVCenter
                    width: undefined
                }

                VPNComboBox {
                    id: dropDown

                    Layout.topMargin: 10

                    placeholderText: VPNl18n.InAppSupportWorkflowDropdownLabel
                    model: VPNSupportCategoryModel
                    Layout.preferredWidth: parent.width
                }

                VPNTextField {
                    id: subjectInput

                    Layout.topMargin: VPNTheme.theme.vSpacing
                    Layout.alignment: Qt.AlignHCenter
                    Layout.fillWidth: true

                    verticalAlignment: Text.AlignVCenter
                    _placeholderText: VPNl18n.InAppSupportWorkflowSubjectFieldPlaceholder
                }

                VPNTextArea {
                    id: textArea

                    Layout.topMargin: VPNTheme.theme.vSpacing
                    Layout.alignment: Qt.AlignHCenter
                    Layout.fillWidth: true
                    Layout.preferredWidth: parent.width - VPNTheme.theme.windowMargin

                    placeholderText: VPNl18n.InAppSupportWorkflowIssueFieldPlaceholder
                }

                VPNTextBlock {
                    Layout.topMargin: VPNTheme.theme.vSpacingSmall * 3.5
                    Layout.fillWidth: true

                    font.pixelSize: VPNTheme.theme.fontSize
                    horizontalAlignment: Text.AlignHCenter
                    text: VPNl18n.InAppSupportWorkflowDisclaimerText
                    width:parent.width
                }

                VPNLinkButton {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignHCenter
                    labelText: VPNl18n.InAppSupportWorkflowPrivacyNoticeLinkText
                    onClicked: VPN.openLink(VPN.LinkPrivacyNotice)
                    width: parent.width
                }

                VPNButton {
                    text: VPNl18n.InAppSupportWorkflowSupportPrimaryButtonText
                    onClicked: {
                        contactUsRoot.emailAddress = (VPN.userState === VPN.UserAuthenticated ? VPNUser.email : emailInput.text);
                        contactUsRoot.createSupportTicket(contactUsRoot.emailAddress, subjectInput.text, textArea.userEntry, dropDown.currentValue);
                    }
                    enabled: dropDown.currentValue != null && textArea.userEntry != "" &&
                             (VPN.userState === VPN.UserAuthenticated ? true :
                                                                        (VPNAuthInApp.validateEmailAddress(emailInput.text) && emailInput.text == confirmEmailInput.text)
                              )
                    opacity: enabled ? 1 : .5
                    Layout.preferredHeight: VPNTheme.theme.rowHeight
                    Layout.fillWidth: true
                    Layout.topMargin: VPNTheme.theme.vSpacingSmall
                    width: undefined
                    height: undefined
                    Behavior on opacity {
                        PropertyAnimation {
                            duration: 100
                        }
                    }
                }

                VPNCancelButton {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredHeight: VPNTheme.theme.rowHeight
                    Layout.topMargin: VPNTheme.theme.vSpacingSmall

                    onClicked: VPN.state === VPN.StateMain ? settingsStack.pop() : mainStackView.pop()
                    implicitHeight: VPNTheme.theme.rowHeight
                }

                VPNVerticalSpacer {
                    Layout.preferredHeight: window.navBarVisible ? 0 : 40
                }
            }
        }
    }

    Component {
        id: thankYouView
        Item {
            ColumnLayout {
                id: col
                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: (VPNTheme.theme.rowHeight + VPNTheme.theme.vSpacing) * -1
                anchors.horizontalCenter: parent.horizontalCenter
                width: Math.min(VPNTheme.theme.maxHorizontalContentWidth, parent.width - VPNTheme.theme.windowMargin * 4)
                VPNPanel {
                    id: panel
                    logo: "qrc:/ui/resources/heart-check.svg"
                    logoTitle: VPNl18n.InAppSupportWorkflowSupportResponseHeader
                    logoSubtitle: VPNl18n.InAppSupportWorkflowSupportResponseBody.arg(contactUsRoot.emailAddress)
                    anchors.horizontalCenter: undefined
                    Layout.fillWidth: true
                }
            }
            VPNButton {
               text: VPNl18n.InAppSupportWorkflowSupportResponseButton
               anchors.top: col.bottom
               anchors.topMargin: VPNTheme.theme.vSpacing
               anchors.horizontalCenter: parent.horizontalCenter
               onClicked: {
                   const getHelpInStack = mainStackView.find((view) => { return view.objectName === "getHelp" });
                   console.log(getHelpInStack)
                   if (getHelpInStack) {
                       // Unwind mainStackView back to Get Help
                       mainStackView.pop(getHelpInStack, StackView.Immediate);
                       return
                   }
               }
               Component.onCompleted: {
                 if (window.fullscreenRequired()) {
                     anchors.top = undefined;
                     anchors.topMargin = undefined;
                     anchors.bottom= parent.bottom
                     anchors.bottomMargin = VPNTheme.theme.windowMargin * 4
                 }
               }
            }
        }
    }
}
