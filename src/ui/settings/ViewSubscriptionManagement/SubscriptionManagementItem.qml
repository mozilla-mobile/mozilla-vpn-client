/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1

ColumnLayout {
    id: root

    spacing: 0

    states: [
        State {
            when: type === "text"

            PropertyChanges {
                target: rowText
                visible: true
            }
            PropertyChanges {
                target: rowPill
                visible: false
            }
        },
        State {
            when: type === "pill"

            PropertyChanges {
                target: rowText
                visible: false
            }
            PropertyChanges {
                target: rowPill
                visible: true
            }
        }
    ]

    RowLayout {
        spacing: VPNTheme.theme.listSpacing

        Layout.alignment: Qt.AlignVCenter
        Layout.bottomMargin: VPNTheme.theme.listSpacing
        Layout.fillWidth: true
        Layout.preferredHeight: VPNTheme.theme.rowHeight
        Layout.topMargin: VPNTheme.theme.listSpacing

        VPNInterLabel {
            id: rowLabel

            horizontalAlignment: Text.AlignLeft
            font.pixelSize: VPNTheme.theme.fontSizeSmall
            text: labelText
            wrapMode: Text.WordWrap

            Layout.alignment: Qt.AlignLeft
        }

        // TODO: Create credit card icon component
        VPNIcon {
            id: labelIcon
            property string cardBrand: VPNSubscriptionData.creditCardBrand
            property var availableCardIcons: [
                "android",
                "apple",
                "paypal",
                "visa",
                "mastercard",
                "amex",
                "discover",
                "diners",
                "unionpay",
                "jcb"
            ]

            source: availableCardIcons.includes(cardBrand)
                ? `qrc:/ui/resources/logos/${cardBrand}.svg`
                : "qrc:/ui/resources/logos/unbranded.svg"
            sourceSize.height: VPNTheme.theme.iconSizeSmall * 1.5
            sourceSize.width: VPNTheme.theme.iconSizeSmall * 1.5

            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
        }

        VPNInterLabel {
            id: rowText

            color: VPNTheme.theme.fontColorDark
            horizontalAlignment: Text.AlignRight
            font.pixelSize: VPNTheme.theme.fontSizeSmall
            text: valueText
            wrapMode: Text.WordWrap

            Layout.alignment: Qt.AlignRight
            Layout.fillWidth: true
        }

        Item {
            height: rowPill.implicitHeight
            Layout.fillWidth: true

            VPNPill {
                id: rowPill

                color: valueText === "active"
                    ? VPNTheme.colors.green90
                    : VPNTheme.colors.red70
                background: valueText === "active"
                    ? VPNTheme.colors.green5
                    : VPNTheme.colors.red5
                text: valueText === "active"
                    ? VPNl18n.SubscriptionManagementStatusActive
                    : VPNl18n.SubscriptionManagementStatusInactive

                anchors {
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                }
            }
        }
    }

    Rectangle {
        id: divider

        color: VPNTheme.colors.grey10

        Layout.fillWidth: true
        Layout.leftMargin: 0
        Layout.preferredHeight: 1
        Layout.rightMargin: 0
    }
}
