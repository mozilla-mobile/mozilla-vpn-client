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
    id: authThirdParty

    property string _viewObjectName
    property string _telemetryScreenId

    property alias _imgSource: img.source
    property alias _headlineText: headline.text
    property alias _description: description.data

    flickContentHeight: col.implicitHeight

    ColumnLayout {
        id: col
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        height: implicitHeight > window.height ? implicitHeight : window.height
        width: parent.width

        RowLayout {
            id: authHeader
            Layout.preferredHeight: MZTheme.theme.rowHeight * 1.2
            Layout.preferredWidth: parent.width
            Layout.topMargin: MZTheme.theme.windowMargin / 2

            MZLinkButton {
                id: headerLink
                objectName: _viewObjectName + "-getHelpLink"
                Layout.alignment: Qt.AlignRight
                Layout.rightMargin: MZTheme.theme.windowMargin
                labelText: MZI18n.GetHelpLinkTitle
                horizontalPadding: MZTheme.theme.windowMargin / 2
                onClicked: {
                    Glean.interaction.getHelpSelected.record({
                        screen: _telemetryScreenId,
                    })

                    MZNavigator.requestScreen(VPN.ScreenGetHelp)
                }
            }
        }

        ColumnLayout {
            spacing:  MZTheme.theme.windowMargin

            Rectangle {
                color: MZTheme.theme.transparent
                Layout.preferredHeight: MZTheme.theme.rowHeight * 2
                Layout.preferredWidth: 117
                Layout.alignment: Qt.AlignHCenter

                Image {
                    id: img

                    anchors.fill: parent
                    sourceSize.height: MZTheme.theme.rowHeight * 2
                    fillMode: Image.PreserveAspectFit
                }
            }

            MZHeadline {
                id: headline
                objectName: _viewObjectName + "-headline"
                width: undefined
                Layout.fillWidth: true
            }

            ColumnLayout {
                id: description
                spacing: MZTheme.theme.windowMargin
                Layout.leftMargin: MZTheme.theme.vSpacing
                Layout.rightMargin: MZTheme.theme.vSpacing
                Layout.preferredWidth: col.width - MZTheme.theme.vSpacing * 2
            }

        }

        ColumnLayout {
            id: footerContent
            Layout.alignment: Qt.AlignBottom
            Layout.maximumWidth: col.width - MZTheme.theme.vSpacing * 2
            Layout.minimumWidth: col.width - MZTheme.theme.vSpacing * 2
            Layout.leftMargin: MZTheme.theme.vSpacing
            Layout.rightMargin: MZTheme.theme.vSpacing
            Layout.bottomMargin: navbar.visible ? 0 : 34
            spacing: 24

            MZButton {
                objectName: _viewObjectName + "-buttonSignIn"
                text: MZI18n.InAppAuthContinueToSignIn
                Layout.fillWidth: true
                onClicked: {
                    MZAuthInApp.checkAccount(MZAuthInApp.emailAddress);
                }
            }

            MZCancelButton {
                objectName: _viewObjectName + "-cancel"
                Layout.alignment: Qt.AlignHCenter
                onClicked: {
                    VPN.cancelAuthentication();
                }
            }
        }

        //Manual padding for views without a navbar - not ideal, but modifying
        //MZFlickable causes a lot of churn, and is a separate issue
        MZVerticalSpacer {
            Layout.preferredHeight: MZTheme.theme.navBarBottomMargin
        }
    }
}
