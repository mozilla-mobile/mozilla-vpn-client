/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


import QtQuick 2.5
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1

VPNFlickable {
    property var _menuButtonOnClick
    property bool _changeEmailLinkVisible: false

    property alias _menuButtonImageSource: menuButtonImage.source
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
        height: Math.max(parent.height, col.implicitHeight)
        anchors.left: parent.left
        anchors.right: parent.right
        width: Math.min(authBase.width, 200)

        PropertyAnimation on opacity {
            from: 0
            to: 1
            duration: 400
        }

        RowLayout {
            id: authHeader
            Layout.preferredHeight: VPNTheme.theme.rowHeight * 1.2
            Layout.preferredWidth: parent.width
            Layout.topMargin: VPNTheme.theme.windowMargin / 2

            VPNIconButton {
                id: menuButton
                onClicked: _menuButtonOnClick()
                Layout.preferredHeight: VPNTheme.theme.rowHeight
                Layout.preferredWidth: VPNTheme.theme.rowHeight
                Layout.leftMargin: VPNTheme.theme.windowMargin / 2

                VPNIcon {
                    id: menuButtonImage
                    anchors.centerIn: parent
                    sourceSize.height: VPNTheme.theme.windowMargin
                    sourceSize.width: VPNTheme.theme.windowMargin
                }
            }

            VPNLinkButton {
                id: headerLink
                objectName: "getHelpLink"
                Layout.preferredHeight: menuButton.height
                Layout.alignment: Qt.AlignRight
                Layout.rightMargin: VPNTheme.theme.windowMargin
                labelText: qsTrId("vpn.main.getHelp2")
                horizontalPadding: VPNTheme.theme.windowMargin / 2
                onClicked: {
                    stackview.push("qrc:/ui/views/ViewGetHelp.qml", {isSettingsView: false});
                }
            }
        }

        VPNVerticalSpacer {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.minimumHeight: VPNTheme.theme.vSpacing / 2
            Layout.maximumHeight: VPNTheme.theme.vSpacing * 2
        }

        ColumnLayout {
            spacing:  VPNTheme.theme.windowMargin
            Layout.leftMargin: VPNTheme.theme.vSpacing
            Layout.rightMargin: VPNTheme.theme.vSpacing
            Layout.preferredWidth: col.width - VPNTheme.theme.vSpacing * 2

            Rectangle {
                color: "transparent"
                height: VPNTheme.theme.rowHeight * 2
                Layout.preferredWidth: 100
                Layout.alignment: Qt.AlignHCenter

                Image {
                    id: img

                    anchors.fill: parent
                    sourceSize.height: VPNTheme.theme.rowHeight * 2
                    fillMode: Image.PreserveAspectFit
                }
            }

            ColumnLayout {
                Layout.topMargin: VPNTheme.theme.windowMargin / 2
                spacing: VPNTheme.theme.windowMargin / 2

                VPNHeadline {
                    id: headline
                    width: undefined
                    Layout.maximumWidth: col.width - VPNTheme.theme.vSpacing * 2
                    Layout.minimumWidth: col.width - VPNTheme.theme.vSpacing * 2
                }

                VPNLinkButton {
                    Layout.alignment: Qt.AlignHCenter
                    labelText: VPNl18n.InAppAuthChangeEmailLink
                    visible: _changeEmailLinkVisible
                    onClicked: VPNAuthInApp.reset()
                }

                VPNSubtitle {
                    id: subtitle
                    horizontalAlignment: Text.AlignHCenter
                    Layout.maximumWidth: col.width - VPNTheme.theme.vSpacing * 2
                    Layout.minimumWidth: col.width - VPNTheme.theme.vSpacing * 2
                }

            }

            VPNBoldLabel {
                id: inputLabel
            }

            ColumnLayout {
                id: inputs
                Layout.maximumWidth: col.width - VPNTheme.theme.vSpacing * 2
            }
        }

        ColumnLayout {
            id: disclaimers
            Layout.maximumWidth: col.width - VPNTheme.theme.vSpacing * 2
            Layout.leftMargin: VPNTheme.theme.vSpacing
            Layout.rightMargin: VPNTheme.theme.vSpacing
            Layout.topMargin: VPNTheme.theme.vSpacing
        }

        VPNVerticalSpacer {
            Layout.fillHeight: true
            Layout.minimumHeight: VPNTheme.theme.vSpacing
        }

        ColumnLayout {
            id: footerContent
            Layout.alignment: Qt.AlignBottom
            Layout.maximumWidth: col.width - VPNTheme.theme.vSpacing * 2
            Layout.minimumWidth: col.width - VPNTheme.theme.vSpacing * 2
            Layout.leftMargin: VPNTheme.theme.vSpacing
            Layout.rightMargin: VPNTheme.theme.vSpacing

        }

        VPNVerticalSpacer {
            Layout.minimumHeight: VPNTheme.theme.vSpacing * 2
            Layout.fillWidth: true
        }
    }

}
