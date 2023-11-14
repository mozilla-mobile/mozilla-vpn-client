/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.Shared.qmlcomponents 1.0
import components 0.1

MZViewBase {
    id: root
    property var message
    objectName: "messageView-" + message.id

    property Component titleComponent: Component {
        RowLayout {
            id: row
            spacing: 6

            Item {
                Layout.fillWidth: true
            }

            MZIcon {
                id: logo
                source: "qrc:/ui/resources/logo.svg"
                sourceSize.height: 20
                sourceSize.width: 20
                antialiasing: true
            }

            MZBoldLabel {
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
        MZIconButton {
            id: deleteButton

            accessibleName: MZI18n.InAppMessagingDeleteMessage
            anchors.rightMargin: 32

            MZIcon {
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
        Layout.leftMargin: MZTheme.theme.windowMargin * 1.5
        Layout.rightMargin: MZTheme.theme.windowMargin * 1.5

        spacing: 0

        RowLayout {
            id: badgeAndTimestampRow

            Layout.preferredWidth: parent.width

            spacing: 20

            Loader {
                Layout.preferredWidth: item.implicitWidth
                active: message.badge !== MZAddonMessage.None
                sourceComponent: MZBadge {

                    badgeLabel.width: Math.min(badgeLabel.implicitWidth, badgeAndTimestampRow.width - dateLabel.width - badgeAndTimestampRow.spacing)
                    badgeType: {
                        switch(message.badge) {
                        case MZAddonMessage.Warning:
                            return badgeInfo.warningBadge
                        case MZAddonMessage.Critical:
                            return badgeInfo.criticalBadge
                        case MZAddonMessage.NewUpdate:
                            return badgeInfo.newUpdateBadge
                        case MZAddonMessage.WhatsNew:
                            return badgeInfo.whatsNewBadge
                        case MZAddonMessage.Survey:
                            return badgeInfo.surveyBadge
                        case MZAddonMessage.Subscription:
                            return badgeInfo.subscriptionBadge
                        }
                    }

                    QtObject {
                        id: badgeInfo

                        property var warningBadge: {
                            'badgeText': MZI18n.InAppMessagingWarningBadge,
                            'badgeTheme': MZTheme.theme.orangeBadge
                        };
                        property var criticalBadge: {
                            'badgeText': MZI18n.InAppMessagingCriticalBadge,
                            'badgeTheme': MZTheme.theme.redBadge
                        };
                        property var newUpdateBadge: {
                            'badgeText': MZI18n.InAppMessagingNewUpdateBadge,
                            'badgeTheme': MZTheme.theme.greenBadge
                        };
                        property var whatsNewBadge: {
                            'badgeText': MZI18n.InAppMessagingWhatsNewBadge,
                            'badgeTheme': MZTheme.theme.blueBadge
                        };
                        property var surveyBadge: {
                            'badgeText': MZI18n.InAppMessagingSurveyBadge,
                            'badgeTheme': MZTheme.theme.blueBadge
                        };
                        property var subscriptionBadge: {
                            'badgeText': MZI18n.InAppMessagingSubscriptionBadge,
                            'badgeTheme': MZTheme.theme.blueBadge
                        };
                    }
                }
            }

            MZInterLabel {
                id: dateLabel

                Layout.preferredWidth: implicitWidth
                Layout.alignment: Qt.AlignRight

                text: message.formattedDate
                color: MZTheme.theme.fontColor
                font.pixelSize: MZTheme.theme.fontSizeSmall
                lineHeight: 21
                horizontalAlignment: Text.AlignRight
                maximumLineCount: 1
                elide: Text.ElideRight
            }
        }

        Column {
            Layout.fillWidth: true
            Layout.topMargin: MZTheme.theme.windowMargin

            MZBoldLabel {
                anchors.left: parent.left
                anchors.right: parent.right

                text: message.title
                font.pixelSize: MZTheme.theme.fontSizeLarge
                lineHeightMode: Text.FixedHeight
                lineHeight: 32
                wrapMode: Text.Wrap
            }

            MZVerticalSpacer {
                height: MZTheme.theme.vSpacingSmall
            }

            MZInterLabel {
                anchors.left: parent.left
                anchors.right: parent.right

                text: message.subtitle
                font.pixelSize: MZTheme.theme.fontSizeSmall
                color: MZTheme.theme.fontColor
                horizontalAlignment: Text.AlignLeft
            }

            MZComposerView {
                anchors.left: parent.left
                anchors.right: parent.right

                view: MZComposerView.View.Message
                addon: message
            }
        }
    }

    Connections {
        target: message
        function onAboutToDisable() {
            if (stackview.currentItem === root) {
                stackview.pop();
            }
        }
    }
}

