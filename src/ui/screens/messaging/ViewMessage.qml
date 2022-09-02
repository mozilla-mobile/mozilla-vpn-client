/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import Mozilla.VPN.qmlcomponents 1.0

VPNViewBase {
    property var message

    property Component titleComponent: Component {
        RowLayout {
            spacing: 6

            VPNIcon {
                source: "qrc:/ui/resources/logo.svg"
                sourceSize.height: 20
                sourceSize.width: 20
                antialiasing: true
                Layout.alignment: Qt.AlignVCenter
            }

            VPNBoldLabel {
                //% "Mozilla VPN"
                text: qsTrId("MozillaVPN")
                color: "#000000"
                Layout.alignment: Qt.AlignVCenter
            }
        }
    }
    property Component rightMenuButton: Component {
        VPNIconButton {
            id: deleteButton

            accessibleName: VPNl18n.InAppMessagingDeleteMessage

            VPNIcon {
                id: icon

                anchors.centerIn: parent
                source: "qrc:/nebula/resources/delete-gray.svg"
            }
            onClicked: {
                stackview.pop()
                message.dismiss()
            }
        }
    }

    signal deleteButtonClicked()

    _viewContentData: ColumnLayout {
        Layout.fillHeight: true
        Layout.fillWidth: true
        Layout.leftMargin: VPNTheme.theme.windowMargin * 1.5
        Layout.rightMargin: VPNTheme.theme.windowMargin * 1.5

        spacing: 0

        RowLayout {
            spacing: 0

            Loader {
                Layout.maximumWidth: parent.width - dateLabel.width - spacer.Layout.minimumWidth
                active: message.badge !== VPNAddonMessage.None
                sourceComponent: VPNBadge {
                    id: badge

                    Component.onCompleted: {
                        switch(message.badge) {
                        case VPNAddonMessage.NewUpdate:
                            badge.text = VPNl18n.InAppMessagingNewUpdateBadge
                            badge.theme = VPNTheme.theme.greenBadge
                            break;
                        case VPNAddonMessage.Critical:
                            badge.text = VPNl18n.InAppMessagingCriticalBadge
                            badge.theme = VPNTheme.theme.redBadge
                            break;
                        }
                    }
                }
            }

            Item {
                id: spacer
                Layout.fillWidth: true
                Layout.minimumWidth: 20
            }

            VPNInterLabel {
                id: dateLabel
                Layout.fillWidth: true

                text: message.formattedDate
                color: VPNTheme.theme.fontColor
                font.pixelSize: VPNTheme.theme.fontSizeSmall
                lineHeight: 21
                horizontalAlignment: Text.AlignRight
                maximumLineCount: 1
                elide: Text.ElideRight
            }
        }

        Column {
            Layout.fillWidth: true
            Layout.topMargin: VPNTheme.theme.windowMargin

            VPNBoldLabel {
                anchors.left: parent.left
                anchors.right: parent.right

                text: message.title
                font.pixelSize: VPNTheme.theme.fontSizeLarge
                lineHeightMode: Text.FixedHeight
                lineHeight: 32
                wrapMode: Text.Wrap
            }

            VPNVerticalSpacer {
                height: VPNTheme.theme.vSpacingSmall
            }

            VPNInterLabel {
                anchors.left: parent.left
                anchors.right: parent.right

                text: message.subtitle
                font.pixelSize: VPNTheme.theme.fontSizeSmall
                color: VPNTheme.theme.fontColor
                horizontalAlignment: Text.AlignLeft
            }

            VPNComposerView {
                anchors.left: parent.left
                anchors.right: parent.right

                view: VPNComposerView.View.Message
                addon: message
            }
        }
    }
}

