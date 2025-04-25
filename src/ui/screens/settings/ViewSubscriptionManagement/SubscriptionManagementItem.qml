/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1

ColumnLayout {
    id: root
    objectName: _objectName + "-parent"

    property string telemetryScreenId

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
            PropertyChanges {
                target: upgradeToAnnualSub
                visible: false
            }
        },
        State {
            when: type === "text-upgrade"

            PropertyChanges {
                target: rowText
                visible: true
            }
            PropertyChanges {
                target: rowPill
                visible: false
            }
            PropertyChanges {
                target: upgradeToAnnualSub
                visible: true
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
            PropertyChanges {
                target: upgradeToAnnualSub
                visible: false
            }
        },
        State {
            when: type === "payment"

            PropertyChanges {
                target: rowText
                visible: true
            }
            PropertyChanges {
                target: rowPill
                visible: false
            }
            PropertyChanges {
                target: upgradeToAnnualSub
                visible: false
            }
        }
    ]

    RowLayout {
        objectName: _objectName + "-container"
        spacing: MZTheme.theme.listSpacing

        Layout.alignment: Qt.AlignVCenter
        Layout.bottomMargin: MZTheme.theme.listSpacing
        Layout.fillWidth: true
        Layout.preferredHeight: MZTheme.theme.rowHeight
        Layout.topMargin: MZTheme.theme.listSpacing

        MZInterLabel {
            id: rowLabel
            objectName: _objectName + "-labelText"

            horizontalAlignment: Text.AlignLeft
            font.pixelSize: MZTheme.theme.fontSizeSmall
            visible: !paymentMethod.visible
            text: labelText
            wrapMode: Text.WordWrap

            Layout.alignment: Qt.AlignLeft
        }

        MZPaymentMethod {
            id: paymentMethod
            objectName: _objectName + "-paymentMethod"
            Layout.alignment: Qt.AlignLeft
            paymentMethod: labelText
            visible: type === "payment"
        }

        MZInterLabel {
            id: rowText
            objectName: _objectName + "-valueText"

            color: MZTheme.colors.fontColorDark
            horizontalAlignment: Text.AlignRight
            font.pixelSize: MZTheme.theme.fontSizeSmall
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

            MZPill {
                objectName: _objectName + "-pill"
                property bool isActive:
                    VPNSubscriptionData.status === VPNSubscriptionData.Active

                color: isActive
                    ? MZTheme.colors.successText
                    : MZTheme.colors.errorText
                background: isActive
                    ? MZTheme.colors.successBackground
                    : MZTheme.colors.errorBackground
                text: valueText

                anchors {
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                }
            }
        }
    }

    RowLayout {
        id: upgradeToAnnualSub
        objectName: _objectName + "-upgradeToAnnualSub-layout"

        visible: false
        Layout.fillWidth: true
        Layout.topMargin: MZTheme.theme.listSpacing
        Layout.bottomMargin: MZTheme.theme.windowMargin

        MZBoldInterLabel {
            color: MZTheme.colors.primaryBrighter
            font.pixelSize: MZTheme.theme.fontSizeSmall
            text: MZI18n.SubscriptionManagementUpgradeToAnnual // "Save 50%..."
            wrapMode: Text.WordWrap
            lineHeight: MZTheme.theme.labelLineHeight
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
        }

        MZButton {
            objectName: _objectName + "-upgradeToAnnualSub-upgradeButton"
            
            onClicked: {
                Glean.interaction.changePlanSelected.record({
                    screen: root.telemetryScreenId,
                });
                MZUrlOpener.openUrlLabel("upgradeToAnnualUrl");
            }
            text: MZI18n.SubscriptionManagementUpgradeToAnnualButton// "Change plan"
            fontSize: MZTheme.theme.fontSizeSmall
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            Layout.preferredHeight: -1
            Layout.minimumHeight : MZTheme.theme.windowMargin * 2
            Layout.preferredWidth: 111
            Layout.maximumWidth: parent.width / 2
            label.wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        }
    }

    Rectangle {
        id: divider

        color: MZTheme.colors.divider

        Layout.fillWidth: true
        Layout.leftMargin: 0
        Layout.preferredHeight: 1
        Layout.rightMargin: 0
    }
}
