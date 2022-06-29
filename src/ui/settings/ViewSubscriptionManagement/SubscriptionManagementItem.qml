/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1

ColumnLayout {
    id: root
    objectName: _objectName + "-parent"

    spacing: 0

    states: [
        State {
            when: type === "text"

            PropertyChanges {
                target: rowLabel
                visible: true
            }
            PropertyChanges {
                target: rowText
                visible: true
            }
            PropertyChanges {
                target: rowPill
                visible: false
            }
            PropertyChanges {
                target: paymentMethod
                visible: false
            }
        },
        State {
            when: type === "pill"

            PropertyChanges {
                target: rowLabel
                visible: true
            }
            PropertyChanges {
                target: rowText
                visible: false
            }
            PropertyChanges {
                target: rowPill
                visible: true
            }
            PropertyChanges {
                target: paymentMethod
                visible: false
            }
        },
        State {
            when: type === "payment"

            PropertyChanges {
                target: rowLabel
                visible: false
            }
            PropertyChanges {
                target: rowText
                visible: true
            }
            PropertyChanges {
                target: rowPill
                visible: false
            }
            PropertyChanges {
                target: paymentMethod
                visible: true
            }
        }
    ]

    RowLayout {
        objectName: _objectName + "-container"
        spacing: VPNTheme.theme.listSpacing

        Layout.alignment: Qt.AlignVCenter
        Layout.bottomMargin: VPNTheme.theme.listSpacing
        Layout.fillWidth: true
        Layout.preferredHeight: VPNTheme.theme.rowHeight
        Layout.topMargin: VPNTheme.theme.listSpacing

        VPNInterLabel {
            id: rowLabel
            objectName: _objectName + "-labelText"

            horizontalAlignment: Text.AlignLeft
            font.pixelSize: VPNTheme.theme.fontSizeSmall
            text: labelText
            wrapMode: Text.WordWrap

            Layout.alignment: Qt.AlignLeft
        }

        VPNPaymentMethod {
            id: paymentMethod
            objectName: _objectName + "-paymentMethod"
            paymentMethod: labelText
        }

        VPNInterLabel {
            id: rowText
            objectName: _objectName + "-valueText"

            color: VPNTheme.theme.fontColorDark
            horizontalAlignment: Text.AlignRight
            font.pixelSize: VPNTheme.theme.fontSizeSmall
            text: valueText
            wrapMode: Text.WordWrap

            Layout.alignment: Qt.AlignRight
            Layout.fillWidth: true
        }

        Item {
            id: rowPill
            objectName: _objectName + "-pillWrapper"

            height: rowPill.implicitHeight
            Layout.fillWidth: true

            VPNPill {
                objectName: _objectName + "-pill"
                property bool isActive:
                    VPNSubscriptionData.status === VPNSubscriptionData.Active

                color: isActive
                    ? VPNTheme.colors.green90
                    : VPNTheme.colors.red70
                background: isActive
                    ? VPNTheme.colors.green5
                    : VPNTheme.colors.red5
                text: valueText

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
