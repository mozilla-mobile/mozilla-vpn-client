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
            id: row
            spacing: 6

            Item {
                Layout.fillWidth: true
            }

            VPNIcon {
                id: logo
                source: "qrc:/ui/resources/logo.svg"
                sourceSize.height: 20
                sourceSize.width: 20
                antialiasing: true
            }

            VPNBoldLabel {
                text: qsTrId("MozillaVPN")
                color: "#000000"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
                Layout.preferredWidth: Math.min(implicitWidth, row.width - row.spacing - logo.sourceSize.width)
            }

            Item {
                Layout.fillWidth: true
            }
        }
    }
    property Component rightMenuButton: Component {
        VPNIconButton {
            id: deleteButton

            accessibleName: VPNl18n.InAppMessagingDeleteMessage
            anchors.rightMargin: 32

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
        Layout.leftMargin: VPNTheme.theme.windowMargin * 1.5
        Layout.rightMargin: VPNTheme.theme.windowMargin * 1.5

        spacing: 0

        RowLayout {
            id: badgeAndTimestampRow

            Layout.preferredWidth: parent.width

            spacing: 20

            Loader {
                Layout.preferredWidth: item.width
                active: message.badge !== VPNAddonMessage.None
                sourceComponent: VPNBadge {

                    badgeLabel.width: Math.min(badgeLabel.implicitWidth, badgeAndTimestampRow.width - dateLabel.width - badgeAndTimestampRow.spacing)
                    badgeType: {
                        switch(message.badge) {
                        case VPNAddonMessage.Warning:
                            return badgeInfo.warningBadge
                        case VPNAddonMessage.Critical:
                            return badgeInfo.criticalBadge
                        case VPNAddonMessage.NewUpdate:
                            return badgeInfo.newUpdateBadge
                        case VPNAddonMessage.WhatsNew:
                            return badgeInfo.whatsNewBadge
                        case VPNAddonMessage.Survey:
                            return badgeInfo.surveyBadge
                        }
                    }

                    QtObject {
                        id: badgeInfo

                        property var warningBadge: {
                            'badgeText': VPNl18n.InAppMessagingWarningBadge,
                            'badgeTheme': VPNTheme.theme.orangeBadge
                        };
                        property var criticalBadge: {
                            'badgeText': VPNl18n.InAppMessagingCriticalBadge,
                            'badgeTheme': VPNTheme.theme.redBadge
                        };
                        property var newUpdateBadge: {
                            'badgeText': VPNl18n.InAppMessagingNewUpdateBadge,
                            'badgeTheme': VPNTheme.theme.greenBadge
                        };
                        property var whatsNewBadge: {
                            'badgeText': VPNl18n.InAppMessagingWhatsNewBadge,
                            'badgeTheme': VPNTheme.theme.blueBadge
                        };
                        property var surveyBadge: {
                            'badgeText': VPNl18n.InAppMessagingSurveyBadge,
                            'badgeTheme': VPNTheme.theme.blueBadge
                        };
                    }
                }
            }

            VPNInterLabel {
                id: dateLabel

                Layout.preferredWidth: implicitWidth
                Layout.alignment: Qt.AlignRight

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

