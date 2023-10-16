/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1

MZFlickable {
    property var _menuButtonOnClick
    property bool _changeEmailLinkVisible: false
    property bool _disclaimersVisible: true
    property string _viewObjectName: ""

    property alias _menuButtonImageSource: menuButtonImage.source
    property alias _menuButtonImageMirror: menuButtonImage.mirror
    property alias _menuButtonAccessibleName: menuButton.accessibleName

    property alias _imgSource: img.source

    property alias _headlineText: headline.text
    property alias _subtitleText: subtitle.text
    property alias _inputLabel: inputLabel.text

    property alias _inputs: inputs.data
    property alias _disclaimers: disclaimers.data
    property alias _footerContent: footerContent.data

    id: authBase
    flickContentHeight: col.implicitHeight

    ColumnLayout {
        id: col
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        PropertyAnimation on opacity {
            from: 0
            to: 1
            duration: 400
        }

        RowLayout {
            id: authHeader
            Layout.preferredHeight: MZTheme.theme.rowHeight * 1.2
            Layout.preferredWidth: parent.width
            Layout.topMargin: MZTheme.theme.windowMargin / 2

            MZIconButton {
                id: menuButton
                objectName: _viewObjectName + "-backButton"
                onClicked: _menuButtonOnClick()
                Layout.preferredHeight: MZTheme.theme.rowHeight
                Layout.preferredWidth: MZTheme.theme.rowHeight
                Layout.leftMargin: MZTheme.theme.windowMargin / 2

                MZIcon {
                    id: menuButtonImage
                    anchors.centerIn: parent
                    sourceSize.height: MZTheme.theme.windowMargin
                    sourceSize.width: MZTheme.theme.windowMargin
                }
            }

            MZLinkButton {
                id: headerLink
                objectName: _viewObjectName + "-getHelpLink"
                Layout.preferredHeight: menuButton.height
                Layout.alignment: Qt.AlignRight
                Layout.rightMargin: MZTheme.theme.windowMargin
                labelText: MZI18n.GetHelpLinkTitle
                horizontalPadding: MZTheme.theme.windowMargin / 2
                onClicked: MZNavigator.requestScreen(VPN.ScreenGetHelp)
            }
        }

        MZVerticalSpacer {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.minimumHeight: MZTheme.theme.vSpacing / 2
            Layout.maximumHeight: MZTheme.theme.vSpacing * 2
        }

        ColumnLayout {
            spacing:  MZTheme.theme.windowMargin
            Layout.leftMargin: MZTheme.theme.vSpacing
            Layout.rightMargin: MZTheme.theme.vSpacing
            Layout.preferredWidth: col.width - MZTheme.theme.vSpacing * 2

            Rectangle {
                color: MZTheme.theme.transparent
                height: MZTheme.theme.rowHeight * 2
                Layout.preferredWidth: 100
                Layout.alignment: Qt.AlignHCenter

                Image {
                    id: img

                    anchors.fill: parent
                    sourceSize.height: MZTheme.theme.rowHeight * 2
                    fillMode: Image.PreserveAspectFit
                }
            }

            ColumnLayout {
                Layout.topMargin: MZTheme.theme.windowMargin / 2
                Layout.preferredWidth: parent.width

                spacing: MZTheme.theme.windowMargin / 2

                MZHeadline {
                    id: headline
                    width: undefined
                    Layout.fillWidth: true
                    Component.onCompleted: {
                        if (
                                MZAuthInApp.state === MZAuthInApp.StateSignIn ||
                                MZAuthInApp.state === MZAuthInApp.StateSigningIn ||
                                MZAuthInApp.state === MZAuthInApp.StateSignUp ||
                                MZAuthInApp.state === MZAuthInApp.StateSigningUp
                                ) {
                            fontSizeMode = Text.FixedSize
                            elide = Text.ElideMiddle
                        }
                    }
                }

                Loader {
                    Layout.alignment: Qt.AlignHCenter
                    active: _changeEmailLinkVisible
                    visible: _changeEmailLinkVisible
                    sourceComponent: MZLinkButton {
                        labelText: MZI18n.InAppAuthChangeEmailLink
                        visible: _changeEmailLinkVisible
                        onClicked: MZAuthInApp.reset()
                    }
                }

                MZSubtitle {
                    id: subtitle
                    visible: _subtitleText.length > 0
                    width: undefined
                    Layout.fillWidth: true
                }

            }

            MZBoldLabel {
                id: inputLabel
                visible: inputLabel.text.length > 0
            }

            ColumnLayout {
                id: inputs
                Layout.preferredWidth: parent.width
            }
        }

        ColumnLayout {
            id: disclaimers
            visible: _disclaimersVisible
            Layout.maximumWidth: col.width - MZTheme.theme.vSpacing * 2
            Layout.leftMargin: MZTheme.theme.vSpacing
            Layout.rightMargin: MZTheme.theme.vSpacing
            Layout.topMargin: MZTheme.theme.vSpacing
        }

        MZVerticalSpacer {
            Layout.fillHeight: true
            Layout.minimumHeight: MZTheme.theme.vSpacing
        }

        ColumnLayout {
            id: footerContent
            Layout.alignment: Qt.AlignBottom
            Layout.maximumWidth: col.width - MZTheme.theme.vSpacing * 2
            Layout.minimumWidth: col.width - MZTheme.theme.vSpacing * 2
            Layout.leftMargin: MZTheme.theme.vSpacing
            Layout.rightMargin: MZTheme.theme.vSpacing
            Layout.bottomMargin: navbar.visible ? 0 : 34


        }
    }

}
