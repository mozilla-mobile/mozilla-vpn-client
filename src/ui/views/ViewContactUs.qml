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
    property alias isMainView: menu.isMainView

    // This property is used to cache the emailAddress between the sub-views.
    property string emailAddress: ""

    id: contactUsRoot
    objectName: "contactUs"

    function stackView() {
      if (menu.isMainView) return mainStackView;
      return stackview;
    }

    function tryAgain() {
        stackView().pop();
    }

    function createSupportTicket(email, subject, issueText, category) {
        stackView().push("qrc:/nebula/components/VPNLoader.qml", {
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

        // this view gets pushed to stackView from backend always
        // and so should be removed from stackView (even in settings flow) on back clicks
    }

    StackView {
        initialItem: contactUsView
        id: contactUsStackView
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.top: menu.bottom
        anchors.topMargin: 0

        Connections {
            target: VPN
            function onTicketCreationAnswer(successful) {
                if(successful) {
                    stackView().replace(thankYouView);
                } else {
                    stackView().replace("qrc:/ui/views/ViewErrorFullScreen.qml", {
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
            flickContentHeight: col.childrenRect.height
            interactive: flickContentHeight > height

            Rectangle {
                anchors.fill: parent
                color: VPNTheme.theme.bgColor
            }

            ColumnLayout {
                id: col

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                spacing: VPNTheme.theme.windowMargin
                anchors.margins: VPNTheme.theme.windowMargin * 2
                anchors.topMargin: window.fullscreenRequired() ? VPNTheme.theme.contentTopMarginMobile : VPNTheme.theme.contentTopMarginDesktop


                ColumnLayout {
                    Layout.alignment: Qt.AlignTop
                    Layout.preferredWidth: parent.width

                    spacing: 24

                    ColumnLayout {
                        Layout.fillHeight: true
                        spacing: 24
                        visible: VPN.userState !== VPN.UserAuthenticated
                        Layout.fillWidth: true

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

                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.preferredWidth: parent.width
                        RowLayout {
                            visible: VPN.userState === VPN.UserAuthenticated
                            spacing: 15
                            Layout.fillWidth: true
                            Layout.bottomMargin: 15

                            Rectangle {
                                Layout.preferredWidth: 40
                                Layout.preferredHeight: 40
                                color: VPNTheme.theme.transparent

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
                                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                    Layout.fillWidth: true

                                }


                                VPNLightLabel {
                                    id: serverLocation
                                    text: VPNUser.email
                                    Accessible.ignored: true
                                    Layout.alignment: Qt.AlignLeft
                                    elide: Text.ElideRight
                                    Layout.fillWidth: true
                                }
                            }
                        }

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

                        width: parent.width
                        verticalAlignment: Text.AlignVCenter
                        Layout.fillWidth: true
                        _placeholderText: VPNl18n.InAppSupportWorkflowSubjectFieldPlaceholder
                    }

                    VPNTextArea {
                        id: textArea
                        placeholderText: VPNl18n.InAppSupportWorkflowIssueFieldPlaceholder
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
                            font.pixelSize: VPNTheme.theme.fontSize
                            horizontalAlignment: Text.AlignHCenter
                            text: VPNl18n.InAppSupportWorkflowDisclaimerText
                            width:parent.width
                        }

                        VPNLinkButton {
                            labelText: VPNl18n.InAppSupportWorkflowPrivacyNoticeLinkText
                            Layout.alignment: Qt.AlignHCenter
                            onClicked: VPN.openLink(VPN.LinkPrivacyNotice)
                            width: parent.width
                        }
                    }

                    ColumnLayout {
                        spacing: VPNTheme.theme.windowMargin

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
                            width: undefined
                            height: undefined
                            Behavior on opacity {
                                PropertyAnimation {
                                    duration: 100
                                }
                            }
                        }
                        VPNLinkButton {
                            labelText: VPNl18n.InAppSupportWorkflowSupportSecondaryActionText
                            Layout.preferredHeight: VPNTheme.theme.rowHeight
                            Layout.alignment: Qt.AlignHCenter
                            onClicked: stackView().pop()
                            implicitHeight: VPNTheme.theme.rowHeight

                        }
                    }
                    VPNVerticalSpacer {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.minimumHeight: VPNTheme.theme.rowHeight * 2
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
                   stackView().pop();
                   stackView().pop();
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
