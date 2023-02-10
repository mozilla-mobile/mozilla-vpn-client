/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import compat 0.1

RadioDelegate {
    id: radioDelegate

    // used by the `Subscribe Now` to pass productIdentifier to VPNPurchase.subscribe()
    property var productId: productIdentifier

    activeFocusOnTab: true
    checked: productFeatured
    ButtonGroup.group: subscriptionOptions

    Layout.fillWidth: true
    Layout.preferredHeight: Math.max(96, row.implicitHeight + row.anchors.topMargin + row.anchors.bottomMargin)

    background: Rectangle {
        id: bg
        color: VPNTheme.theme.white
        radius: VPNTheme.theme.cornerRadius

        VPNRectangularGlow {
            anchors.fill: bg
            glowRadius: radioDelegate.checked ? 8 : 1
            spread: radioDelegate.checked ? 0.1 : 0
            color: "#4D0C0C0D"
            cornerRadius: rect.radius + glowRadius
        }

        Rectangle {
            id: rect
            anchors.fill: parent
            color: VPNTheme.theme.white
            radius: parent.radius
            clip: true
        }
    }
    indicator: Item {
        anchors.fill: parent
        activeFocusOnTab: false

        // Purple left-hand indicator bar visible
        // when product is selected
        Rectangle {
            anchors {
                bottom: parent.bottom
                left: parent.left
                top: parent.top
            }
            color: radioDelegate.checked ? VPNTheme.theme.purple60 : VPNTheme.theme.white
            opacity: radioDelegate.checked ? 1 : 0
            radius: VPNTheme.theme.cornerRadius
            width: VPNTheme.theme.windowMargin

            Rectangle {
                anchors {
                    bottom: parent.bottom
                    right: parent.right
                    top: parent.top
                }
                color: VPNTheme.theme.white
                width: VPNTheme.theme.listSpacing * 0.5
            }

            Behavior on color {
                ColorAnimation {
                    duration: 100
                }
            }
        }

        // Purple border when product is selected or focused
        Rectangle {
            anchors.fill: parent
            border.color: (radioDelegate.checked || radioDelegate.focus) ? VPNTheme.theme.purple60 : VPNTheme.theme.white
            color: VPNTheme.theme.transparent
            radius: VPNTheme.theme.cornerRadius

            Behavior on border.color {
                ColorAnimation {
                    duration: 100
                }
            }
        }
    }
    onFocusChanged: {
        if (focus) {
            vpnFlickable.ensureVisible(radioDelegate);
        }
    }
    onPressed: {
        if (radioDelegate.checked) {
            return VPNPurchase.subscribe(subscriptionOptions.checkedButton.productId);
        }
    }

    RowLayout {
        id: row

        anchors {
            fill: parent
            topMargin: chipFreeTrial.visible ? (VPNTheme.theme.vSpacing / 2) : VPNTheme.theme.vSpacing
            leftMargin: VPNTheme.theme.windowMargin * 1.5
            rightMargin: VPNTheme.theme.windowMargin
            bottomMargin: chipFreeTrial.visible ? (VPNTheme.theme.vSpacing / 2) : VPNTheme.theme.vSpacing
            verticalCenter: parent.verticalCenter
        }
        spacing: VPNTheme.theme.listSpacing
        clip: true

        ColumnLayout {
            id: col

            // TODO (maybe) - Do we want to add the subscription duration in months to the model?
            property var subscriptionDuration: getSubscriptionDuration(productType)

            //% "Monthly plan"
            property string productSingleMonth: qsTrId("vpn.subscription.monthlyPlan")

            //: %1 is replaced by the subscription duration in months. %2 is replaced by the total subscription cost.
            //% "%1-month plan: %2"
            property string productMultiMonth: qsTrId("vpn.subscription.multiMonthPlan").arg(col.subscriptionDuration).arg(productPrice)

            //: “/month” stands for “per month”. %1 is replaced by the monthly cost (including currency).
            //% "%1/month"
            property string monthlyPrice: qsTrId("vpn.subscription.price").arg(productMonthlyPrice)

            property int trialDays: productTrialDays

            spacing: 0
            Layout.fillHeight: true

            VPNBoldLabel {
                id: headLineLabel
                font.pixelSize: 16
                lineHeight: VPNTheme.theme.labelLineHeight + 2
                lineHeightMode: Text.FixedHeight
                text: col.subscriptionDuration > 1 ? col.productMultiMonth : col.monthlyPrice
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WordWrap

                Layout.fillWidth: true
            }

            VPNVerticalSpacer {
                Layout.fillHeight: true
            }

            VPNLightLabel {
                font.pixelSize: VPNTheme.theme.fontSize
                lineHeight: 17.68
                lineHeightMode: Text.FixedHeight
                text: col.subscriptionDuration !== -1 ? (col.subscriptionDuration > 1 ? col.monthlyPrice : col.productSingleMonth) : ""
                wrapMode: Text.WordWrap

                Layout.fillWidth: true
            }

            VPNVerticalSpacer {
                visible: chipFreeTrial.visible
                Layout.fillHeight: true
            }

            //Free trial chip
            Rectangle {
                id: chipFreeTrial
                Layout.preferredWidth: labelFreeTrial.implicitWidth
                Layout.preferredHeight: labelFreeTrial.implicitHeight

                //hardcoding to 7 because the chip's text below is currently hardcoded for 7-day trials specifically
                visible: col.trialDays === 7
                color: VPNTheme.colors.green5

                Text {
                    id: labelFreeTrial

                    topPadding: 3
                    leftPadding: 8
                    rightPadding: 8
                    bottomPadding: 3

                    text: VPNI18n.FreeTrialsFreeTrialLabel
                    verticalAlignment: Text.AlignVCenter
                    lineHeight: VPNTheme.theme.controllerInterLineHeight
                    lineHeightMode: Text.FixedHeight
                    color: VPNTheme.colors.green90
                    font.pixelSize: VPNTheme.theme.fontSizeSmallest
                    font.family: VPNTheme.theme.fontInterSemiBoldFamily
                }
            }

            function getSubscriptionDuration(product) {
                switch (product) {
                    case VPNProducts.ProductMonthly:
                        return 1;
                    case VPNProducts.ProductHalfYearly:
                        return 6;
                    case VPNProducts.ProductYearly:
                        return 12;
                    default:
                        return -1;
                }
            }
        }

        ColumnLayout {
            spacing: 0

            Layout.alignment: Qt.AlignTop
            Layout.fillWidth: true

            VPNInterLabel {
                //: Appears on the in-app purchase view beside a subscription plan. "%1" is replaced by the percentage amount saved when selecting that plan.
                //% "Save %1%"
                text: qsTrId("vpn.subscription.savePercent").arg(productSavings)

                color: VPNTheme.theme.purple60
                font.family: VPNTheme.theme.fontBoldFamily
                horizontalAlignment: Qt.AlignRight
                lineHeight: VPNTheme.theme.labelLineHeight * 0.9
                lineHeightMode: Text.FixedHeight
                verticalAlignment: Text.AlignVCenter
                visible: productSavings > 0
                wrapMode: Text.WordWrap

                Layout.minimumWidth: row.width * 0.3
                Layout.fillWidth: true
            }
        }
    }
}
