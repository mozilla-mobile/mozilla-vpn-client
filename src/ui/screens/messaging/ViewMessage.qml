/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1

VPNFlickable {
    id: vpnFlickable
    property var message

    property Component rightMenuButton: Component {
        VPNIconButton {
            id: editLink

            accessibleName: VPNl18n.InAppMessagingDeleteMessage

            VPNIcon {
                id: icon

                anchors.centerIn: parent
                source: "qrc:/nebula/resources/delete-gray.svg"
            }
            onClicked: deleteButtonClicked()
        }
    }

    signal deleteButtonClicked()

    flickContentHeight: messageLayout.implicitHeight

    Column {
        id: messageLayout

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        spacing: 0

        ColumnLayout {
            anchors {
                left: parent.left
                right: parent.right
                leftMargin: VPNTheme.theme.windowMargin * 1.5
                rightMargin: VPNTheme.theme.windowMargin * 1.5
            }

            spacing: 0

            RowLayout {
                Layout.topMargin: VPNTheme.theme.windowMargin
                spacing: 0

                Loader {
                    Layout.maximumWidth: parent.width - dateLabel.width - spacer.Layout.minimumWidth
                    active: message.badge !== AddonMessageBadge.None
                    sourceComponent: VPNBadge {
                        id: badge

                        Component.onCompleted: {
                            switch(message.badge) {
                            case AddonMessageBadge.NewUpdate:
                                badge.text = VPNl18n.InAppMessagingNewUpdateBadge
                                badge.theme = VPNTheme.theme.greenBadge
                                break;
                            case AddonMessageBadge.Critical:
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

            VPNBoldLabel {
                Layout.fillWidth: true
                Layout.topMargin: VPNTheme.theme.windowMargin

                text: message.title
                font.pixelSize: VPNTheme.theme.fontSizeLarge
                lineHeightMode: Text.FixedHeight
                lineHeight: 32
                wrapMode: Text.Wrap
            }

            VPNComposerView {
                Layout.fillWidth: true

                view: VPNComposerView.View.Message
                addon: message
            }
        }
    }
}
