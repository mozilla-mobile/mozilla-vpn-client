/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1
import themes 0.1

Item {
    property string _menuTitle:  VPNl18n.InAppSupportWorkflowSupportNavLinkText

    id: contactUsRoot

    function tryAgain() {
        mainStackView.pop();
    }

    function createSupportTicket(email, subject, issueText, category) {
        mainStackView.push("qrc:/components/components/VPNLoader.qml", {
            footerLinkIsVisible: false
        });
        VPN.createSupportTicket(email, subject, issueText, category);
    }

    VPNMenu {
        id: menu
        title: VPNl18n.InAppSupportWorkflowSupportNavLinkText

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
        anchors.topMargin: 0

        Connections {
            target: VPN
            function onTicketCreationAnswer(successful) {
                if(successful) {
                    mainStackView.replace(thankYouView);
                } else {
                    mainStackView.replace("qrc:/ui/views/ViewErrorFullScreen.qml", {
                        headlineText: VPNl18n.InAppSupportWorkflowSupportErrorHeader,
                        errorMessage: VPNl18n.InAppSupportWorkflowSupportErrorText,
                        buttonText: VPNl18n.InAppSupportWorkflowSupportErrorButton,
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
        objectName: "contactUsView"
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
                        Layout.fillWidth: true

                        ColumnLayout {
                            spacing: 10
                            VPNBoldLabel {
                                property string enterEmailAddress: VPNl18n.InAppSupportWorkflowSupportEmailFieldLabel

                                text: enterEmailAddress
                                lineHeight: Theme.labelLineHeight
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
                                placeholderText: VPNl18n.InAppSupportWorkflowSupportEmailFieldPlaceholder
                            }
                        }

                        VPNTextField {
                            id: confirmEmailInput

                            width: parent.width
                            verticalAlignment: Text.AlignVCenter
                            Layout.fillWidth: true
                            hasError: !VPNAuthInApp.validateEmailAddress(confirmEmailInput.text) || emailInput.text != confirmEmailInput.text
                            placeholderText: VPNl18n.InAppSupportWorkflowSupportConfirmEmailPlaceholder
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.preferredWidth: parent.width
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
                            lineHeight: Theme.labelLineHeight
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
                        placeholderText: VPNl18n.InAppSupportWorkflowSubjectFieldPlaceholder
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
                            font.pixelSize: Theme.fontSize
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
                        spacing: Theme.windowMargin

                        VPNButton {
                            text: VPNl18n.InAppSupportWorkflowSupportPrimaryButtonText
                            onClicked: contactUsRoot.createSupportTicket((VPN.userAuthenticated ? VPNUser.email : emailInput.text), subjectInput.text, textArea.userEntry, dropDown.currentValue);
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
                            labelText: VPNl18n.InAppSupportWorkflowSupportSecondaryActionText
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
                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: (Theme.rowHeight + Theme.vSpacing) * -1
                anchors.horizontalCenter: parent.horizontalCenter
                width: Math.min(Theme.maxHorizontalContentWidth, parent.width - Theme.windowMargin * 4)
                VPNPanel {
                    id: panel
                    logo: "qrc:/ui/resources/heart-check.svg"
                    logoTitle: VPNl18n.InAppSupportWorkflowSupportResponseHeader
                    logoSubtitle: VPNl18n.InAppSupportWorkflowSupportResponseBody.arg((VPN.userAuthenticated ? VPNUser.email : emailInput.text))
                    anchors.horizontalCenter: undefined
                    Layout.fillWidth: true
                }
            }
            VPNButton {
               text: VPNl18n.InAppSupportWorkflowSupportResponseButton
               anchors.top: col.bottom
               anchors.topMargin: Theme.vSpacing
               anchors.horizontalCenter: parent.horizontalCenter
               onClicked: {
                   mainStackView.pop();
                   mainStackView.pop();
               }
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
