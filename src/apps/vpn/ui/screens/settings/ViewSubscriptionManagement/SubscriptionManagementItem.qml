/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14

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

    RowLayout {
        id: relayUpsell
        objectName: _objectName + "-relayUpsell-layout"

        visible: false
        Layout.fillWidth: true
        Layout.topMargin: VPNTheme.theme.listSpacing
        Layout.bottomMargin: VPNTheme.theme.windowMargin

        Component.onCompleted: {
            if (visible) {
                MZGleanDeprecated.recordGleanEvent("bundle_upsell_viewed");
                Glean.sample.bundleUpsellViewed.record();
            }
        }

        ColumnLayout {
            Layout.fillWidth: true

            VPNInterLabel {
                color: VPNTheme.theme.fontColorDark
                horizontalAlignment: Text.AlignLeft
                font.pixelSize: VPNTheme.theme.fontSizeSmall
                text: VPNI18n.SubscriptionManagementAddFirefoxRelay // "Add Firefox Relay"
                wrapMode: Text.WordWrap
                lineHeight: 13
                Layout.alignment: Qt.AlignLeft
                Layout.fillWidth: true
                elide: Text.ElideRight
            }

            VPNLinkButton {
                objectName: _objectName + "-relayUpsell-learnMoreLink"
                linkColor: VPNTheme.theme.blueButton
                fontSize: VPNTheme.theme.fontSizeSmall
                labelText: VPNI18n.SplittunnelInfoLinkText // "Learn more"
                Layout.alignment: Qt.AlignLeft
                padding: 4
                Layout.leftMargin: -4

                onClicked: {
                    MZGleanDeprecated.recordGleanEvent("bundle_upsell_link_clicked");
                    Glean.sample.bundleUpsellLinkClicked.record();
                    VPNUrlOpener.openUrlLabel("relayPremium");
                }
            }
        }

        VPNButtonBase {
            objectName: _objectName + "-relayUpsell-upgradeButton"

            onClicked: {
                MZGleanDeprecated.recordGleanEvent("bundle_upsell_upgrade_clicked");
                Glean.sample.bundleUpsellUpgradeClicked.record();
                VPNUrlOpener.openUrlLabel("upgradeToBundle");
            }

            contentItem: Label {
                text: VPNI18n.SubscriptionManagementUpgrade // "Upgrade"
                font.family: VPNTheme.theme.fontInterFamily
                font.pixelSize: VPNTheme.theme.fontSizeSmall
                color: VPNTheme.theme.white
                anchors.centerIn: parent
                leftPadding: VPNTheme.theme.windowMargin
                rightPadding: VPNTheme.theme.windowMargin
            }

            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            Layout.preferredHeight: VPNTheme.theme.windowMarginb * 2

            VPNUIStates {
                colorScheme:VPNTheme.theme.blueButton
                setMargins: -3
            }
            VPNMouseArea {
                id: buttonMouseArea
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
