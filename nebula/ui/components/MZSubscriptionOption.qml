/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import compat 0.1

RadioDelegate {
    id: radioDelegate

    // used by the `Subscribe Now` to pass productIdentifier to VPNPurchase.subscribe()
    property var productId: productIdentifier

    activeFocusOnTab: true
    checked: productFeatured

    Layout.fillWidth: true
    Layout.preferredHeight: Math.max(96, row.implicitHeight + row.anchors.topMargin + row.anchors.bottomMargin)

    background: Rectangle {
        id: bg
        color: MZTheme.theme.white
        radius: MZTheme.theme.cornerRadius

        MZRectangularGlow {
            anchors.fill: bg
            glowRadius: radioDelegate.checked ? 8 : 1
            spread: radioDelegate.checked ? 0.1 : 0
            color: "#4D0C0C0D"
            cornerRadius: rect.radius + glowRadius
        }

        Rectangle {
            id: rect
            anchors.fill: parent
            color: MZTheme.theme.white
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
            color: radioDelegate.checked ? MZTheme.theme.purple60 : MZTheme.theme.white
            opacity: radioDelegate.checked ? 1 : 0
            radius: MZTheme.theme.cornerRadius
            width: MZTheme.theme.windowMargin

            Rectangle {
                anchors {
                    bottom: parent.bottom
                    right: parent.right
                    top: parent.top
                }
                color: MZTheme.theme.white
                width: MZTheme.theme.listSpacing * 0.5
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
            border.color: (radioDelegate.checked || radioDelegate.focus) ? MZTheme.theme.purple60 : MZTheme.theme.white
            color: MZTheme.theme.transparent
            radius: MZTheme.theme.cornerRadius

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
            topMargin: chipFreeTrial.visible ? (MZTheme.theme.vSpacing / 2) : MZTheme.theme.vSpacing
            leftMargin: MZTheme.theme.windowMargin * 1.5
            rightMargin: MZTheme.theme.windowMargin
            bottomMargin: chipFreeTrial.visible ? (MZTheme.theme.vSpacing / 2) : MZTheme.theme.vSpacing
            verticalCenter: parent.verticalCenter
        }
        spacing: MZTheme.theme.listSpacing
        clip: true

        ColumnLayout {
            id: col

            // TODO (maybe) - Do we want to add the subscription duration in months to the model?
            property var subscriptionDuration: getSubscriptionDuration(productType)

            property string productMultiMonth: MZI18n.PurchaseSubscriptionOption.arg(col.subscriptionDuration).arg(productPrice)

            property int trialDays: productTrialDays

            spacing: 0
            Layout.fillHeight: true

            MZBoldLabel {
                id: headLineLabel
                font.pixelSize: 16
                lineHeight: MZTheme.theme.labelLineHeight + 2
                lineHeightMode: Text.FixedHeight
                text: col.subscriptionDuration > 1 ? col.productMultiMonth : MZI18n.SubscriptionManagementPlanMonthly.arg(productMonthlyPrice)
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WordWrap

                Layout.fillWidth: true
            }

            MZVerticalSpacer {
                Layout.fillHeight: true
            }

            MZLightLabel {
                font.pixelSize: MZTheme.theme.fontSize
                lineHeight: 17.68
                lineHeightMode: Text.FixedHeight
                text: {
                    if(col.subscriptionDuration > 0) {
                        return ["USD", "CAD"].includes(productCurrencyCode) ? MZI18n.SubscriptionManagementPlanMonthlyPlusTax.arg(productMonthlyPrice) : MZI18n.SubscriptionManagementPlanMonthlyWithoutTax.arg(productMonthlyPrice)
                    }
                    return ""
                }
                wrapMode: Text.WordWrap

                Layout.fillWidth: true
            }

            MZVerticalSpacer {
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
                color: MZTheme.colors.green5

                Text {
                    id: labelFreeTrial

                    topPadding: 3
                    leftPadding: 8
                    rightPadding: 8
                    bottomPadding: 3

                    text: MZI18n.FreeTrialsFreeTrialLabel
                    verticalAlignment: Text.AlignVCenter
                    lineHeight: MZTheme.theme.controllerInterLineHeight
                    lineHeightMode: Text.FixedHeight
                    color: MZTheme.colors.green90
                    font.pixelSize: MZTheme.theme.fontSizeSmallest
                    font.family: MZTheme.theme.fontInterSemiBoldFamily
                    Accessible.ignored: !visible
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

            MZInterLabel {
                text: MZI18n.PurchasePercentSaved.arg(productSavings)

                color: MZTheme.theme.purple60
                font.family: MZTheme.theme.fontBoldFamily
                horizontalAlignment: Qt.AlignRight
                lineHeight: MZTheme.theme.labelLineHeight * 0.9
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
