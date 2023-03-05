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
            PropertyChanges {
                target: relayUpsell
                visible: false
            }
        },
        State {
            when: type === "text-upgrade"

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
            PropertyChanges {
                target: relayUpsell
                visible: true
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
            PropertyChanges {
                target: relayUpsell
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
            PropertyChanges {
                target: relayUpsell
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
            text: labelText
            wrapMode: Text.WordWrap

            Layout.alignment: Qt.AlignLeft
        }

        MZPaymentMethod {
            id: paymentMethod
            objectName: _objectName + "-paymentMethod"
            paymentMethod: labelText
        }

        MZInterLabel {
            id: rowText
            objectName: _objectName + "-valueText"

            color: MZTheme.theme.fontColorDark
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
                    ? MZTheme.colors.green90
                    : MZTheme.colors.red70
                background: isActive
                    ? MZTheme.colors.green5
                    : MZTheme.colors.red5
                text: valueText

                anchors {
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                }
            }
        }
    }

    RowLayout {
        id: relayUpsell
        objectName: _objectName + "-relayUpsell-layout"

        visible: false
        Layout.fillWidth: true
        Layout.topMargin: MZTheme.theme.listSpacing
        Layout.bottomMargin: MZTheme.theme.windowMargin

        Component.onCompleted: {
            if (visible) {
                MZGleanDeprecated.recordGleanEvent("bundle_upsell_viewed");
                Glean.sample.bundleUpsellViewed.record();
            }
        }

        ColumnLayout {
            Layout.fillWidth: true

            MZInterLabel {
                color: MZTheme.theme.fontColorDark
                horizontalAlignment: Text.AlignLeft
                font.pixelSize: MZTheme.theme.fontSizeSmall
                text: MZI18n.SubscriptionManagementAddFirefoxRelay // "Add Firefox Relay"
                wrapMode: Text.WordWrap
                lineHeight: 13
                Layout.alignment: Qt.AlignLeft
                Layout.fillWidth: true
                elide: Text.ElideRight
            }

            MZLinkButton {
                objectName: _objectName + "-relayUpsell-learnMoreLink"
                linkColor: MZTheme.theme.blueButton
                fontSize: MZTheme.theme.fontSizeSmall
                labelText: MZI18n.SplittunnelInfoLinkText // "Learn more"
                Layout.alignment: Qt.AlignLeft
                padding: 4
                Layout.leftMargin: -4

                onClicked: {
                    MZGleanDeprecated.recordGleanEvent("bundle_upsell_link_clicked");
                    Glean.sample.bundleUpsellLinkClicked.record();
                    MZUrlOpener.openUrlLabel("relayPremium");
                }
            }
        }

        MZButtonBase {
            objectName: _objectName + "-relayUpsell-upgradeButton"

            onClicked: {
                MZGleanDeprecated.recordGleanEvent("bundle_upsell_upgrade_clicked");
                Glean.sample.bundleUpsellUpgradeClicked.record();
                MZUrlOpener.openUrlLabel("upgradeToBundle");
            }

            contentItem: Label {
                text: MZI18n.SubscriptionManagementUpgrade // "Upgrade"
                font.family: MZTheme.theme.fontInterFamily
                font.pixelSize: MZTheme.theme.fontSizeSmall
                color: MZTheme.theme.white
                anchors.centerIn: parent
                leftPadding: MZTheme.theme.windowMargin
                rightPadding: MZTheme.theme.windowMargin
            }

            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            Layout.preferredHeight: MZTheme.theme.windowMarginb * 2

            MZUIStates {
                colorScheme:MZTheme.theme.blueButton
                setMargins: -3
            }
            MZMouseArea {
                id: buttonMouseArea
            }
        }
    }

    Rectangle {
        id: divider

        color: MZTheme.colors.grey10

        Layout.fillWidth: true
        Layout.leftMargin: 0
        Layout.preferredHeight: 1
        Layout.rightMargin: 0
    }
}
