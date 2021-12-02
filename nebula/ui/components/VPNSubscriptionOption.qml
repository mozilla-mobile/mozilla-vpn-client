/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import compat 0.1
import themes 0.1

RadioDelegate {
    id: radioDelegate

    // used by the `Subscribe Now` to pass productIdentifier to VPNIAP.subscribe()
    property var productId: productIdentifier

    activeFocusOnTab: true
    checked: productFeatured
    ButtonGroup.group: subscriptionOptions

    Layout.fillWidth: true
    Layout.minimumHeight: 68
    Layout.preferredHeight: row.implicitHeight + Theme.windowMargin * 2

    background: Rectangle {
        id: bg
        color: Theme.white
        radius: Theme.cornerRadius

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
            color: Theme.white
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
            color: radioDelegate.checked ? Theme.purple60 : Theme.white
            opacity: radioDelegate.checked ? 1 : 0
            radius: Theme.cornerRadius
            width: Theme.windowMargin

            Rectangle {
                anchors {
                    bottom: parent.bottom
                    right: parent.right
                    top: parent.top
                }
                color: Theme.white
                width: Theme.listSpacing * 0.5
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
            border.color: (radioDelegate.checked || radioDelegate.focus) ? Theme.purple60 : Theme.white
            color: "transparent"
            radius: Theme.cornerRadius

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
            return VPNIAP.subscribe(subscriptionOptions.checkedButton.productId);
        }
    }

    RowLayout {
        id: row

        anchors {
            fill: parent
            leftMargin: Theme.windowMargin * 1.5
            rightMargin: Theme.windowMargin
            verticalCenter: parent.verticalCenter
        }
        spacing: Theme.listSpacing

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

            spacing: Theme.listSpacing * 0.5

            VPNBoldLabel {
                font.pixelSize: Theme.fontSize * 1.1
                lineHeight: Theme.labelLineHeight
                lineHeightMode: Text.FixedHeight
                text: col.subscriptionDuration > 1 ? col.productMultiMonth : col.monthlyPrice
                verticalAlignment: Text.AlignTop
                wrapMode: Text.WordWrap

                Layout.fillWidth: true
            }

            VPNLightLabel {
                font.pixelSize: Theme.fontSize
                text: col.subscriptionDuration !== -1 ? (col.subscriptionDuration > 1 ? col.monthlyPrice : col.productSingleMonth) : ""
                wrapMode: Text.WordWrap

                Layout.fillWidth: true
            }

            function getSubscriptionDuration(product) {
                switch (product) {
                    case VPNIAP.ProductMonthly:
                        return 1;
                    case VPNIAP.ProductHalfYearly:
                        return 6;
                    case VPNIAP.ProductYearly:
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
            Layout.topMargin: Theme.windowMargin

            VPNInterLabel {
                //: Appears on the in-app purchase view beside a subscription plan. "%1" is replaced by the percentage amount saved when selecting that plan.
                //% "Save %1%"
                text: qsTrId("vpn.subscription.savePercent").arg(productSavings)

                color: Theme.purple60
                font.family: Theme.fontBoldFamily
                horizontalAlignment: Qt.AlignRight
                lineHeight: Theme.labelLineHeight * 0.9
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
