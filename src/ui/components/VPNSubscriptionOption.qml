/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
// import QtGraphicalEffects 1.0

import Mozilla.VPN 1.0
import components 0.1
import themes 0.1

RadioDelegate {
    // used by the `Subscribe Now` to pass productIdentifier to VPNIAP.subscribe()
    property var productId: productIdentifier

    id: radioDelegate
    Layout.fillWidth: true
    Layout.minimumHeight: 68
    Layout.preferredHeight: implicitContentHeight
    checked: productFeatured

    ButtonGroup.group: subscriptionOptions
    activeFocusOnTab: true
    onFocusChanged: {
        if (focus) {
            vpnFlickable.ensureVisible(radioDelegate)
        }
    }

    onPressed: {
        if (checked) {
           return  VPNIAP.subscribe(subscriptionOptions.checkedButton.productId)
        }
    }

    background: Rectangle {
        id: bg
        color: Theme.white
        radius: Theme.cornerRadius

        // RectangularGlow {
        //     anchors.fill: bg
        //     glowRadius: checked ? 8 : 1
        //     spread: checked ? 0.1 : 0
        //     color: "#4D0C0C0D"
        //     cornerRadius: rect.radius + glowRadius
        // }

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

        // Purple left-hand indicator bar
        // visible when product is selected
        Rectangle {
            radius: Theme.cornerRadius
            opacity: checked ? 1 : 0
            color: checked ? Theme.purple60 : Theme.white
            width: Theme.windowMargin
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom

            Rectangle {
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 4
                color: Theme.white
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
            radius: Theme.cornerRadius
            border.color: checked || radioDelegate.focus ? Theme.purple60 : Theme.white
            color: "transparent"

            Behavior on border.color {
                ColorAnimation {
                    duration: 100
                }
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 12
        anchors.leftMargin: 24
        anchors.rightMargin: 24
        anchors.verticalCenter: parent.verticalCenter
        anchors.bottomMargin: 12

        ColumnLayout {
            id: col
                function getSubscriptionDuration(product) {
                    switch (product) {
                    case VPNIAP.ProductMonthly:
                        return 1;
                    case VPNIAP.ProductHalfYearly:
                        return 6;
                    case VPNIAP.ProductYearly:
                        return 12;
                    default:
                        return 0;
                    }
                }

            // TODO (maybe) - Do we want to add the subscription duration in months to the model?
            property var subscriptionDuration: getSubscriptionDuration(productType)

            //% "Monthly plan"
            property string productSingleMonth: qsTrId("vpn.subscription.monthlyPlan")

            //: %1 is replaced by the subscription duration in months. %2 is replaced by the total subscription cost.
            //% "%1-month plan: %2"
            property string productMultiMonth: qsTrId("vpn.subscription.multiMonthPlan").arg(subscriptionDuration).arg(productPrice)

            //: “/month” stands for “per month”. %1 is replaced by the monthly cost (including currency).
            //% "%1/month"
            property string monthlyPrice: qsTrId("vpn.subscription.price").arg(productMonthlyPrice)

            spacing: 0

            VPNBoldLabel {
                text: col.subscriptionDuration > 1 ? col.productMultiMonth : col.monthlyPrice
                lineHeightMode: Text.FixedHeight
                lineHeight: Theme.labelLineHeight
                verticalAlignment: Text.AlignTop
            }

            VPNLightLabel {
                text: col.subscriptionDuration > 0 ? (col.subscriptionDuration> 1 ? col.monthlyPrice : col.productSingleMonth) : ""
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
        }

        ColumnLayout {

            Layout.alignment: Qt.AlignTop
            spacing: 0

            VPNInterLabel {
                //: Appears on the in-app purchase view beside a subscription plan. "%1" is replaced by the percentage amount saved when selecting that plan.
                //% "Save %1%"
                text: qsTrId("vpn.subscription.savePercent").arg(productSavings)

                visible: productSavings > 0
                color: Theme.purple60
                font.family: Theme.fontBoldFamily
                horizontalAlignment: Qt.AlignRight
                verticalAlignment: Text.AlignVCenter
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                lineHeightMode: Text.FixedHeight
                lineHeight: Theme.labelLineHeight
            }
        }
    }
}
