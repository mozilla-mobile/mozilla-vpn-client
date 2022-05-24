/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0

RowLayout {
    id: root
    property string brand: "unbranded"
    property var selectedCreditCard: getCreditCard(brand)

    visible: true

    VPNInterLabel {
        id: label

        horizontalAlignment: Text.AlignLeft
        font.pixelSize: VPNTheme.theme.fontSizeSmall
        text: selectedCreditCard.name
        wrapMode: Text.WordWrap
    }

    VPNIcon {
        id: icon

        source: selectedCreditCard.icon
        sourceSize.height: VPNTheme.theme.iconSizeSmall * 1.5
        sourceSize.width: VPNTheme.theme.iconSizeSmall * 1.5

        Layout.alignment: Qt.AlignVCenter
    }

    function getCreditCard(cardBrand) {
        switch(cardBrand) {
            case "amex":
                return {
                    key: cardBrand,
                    icon: "qrc:/ui/resources/payment/amex.svg",
                    name: "American Express"
                };
            case "diners":
                return {
                    key: cardBrand,
                    icon: "qrc:/ui/resources/payment/diners.svg",
                    name: "Diners Club"
                };
            case "discover":
                return {
                    key: cardBrand,
                    icon: "qrc:/ui/resources/payment/discover.svg",
                    name: "Discover"
                };
            case "jcb":
                return {
                    key: cardBrand,
                    icon: "qrc:/ui/resources/payment/jcb.svg",
                    name: "JCB"
                };
            case "mastercard":
                return {
                    key: cardBrand,
                    icon: "qrc:/ui/resources/payment/mastercard.svg",
                    name: "Mastercard"
                };
            case "paypal":
                return {
                    key: cardBrand,
                    icon: "qrc:/ui/resources/payment/paypal.svg",
                    name: "PayPal"
                };
            case "unionpay":
                return {
                    key: cardBrand,
                    icon: "qrc:/ui/resources/payment/unionpay.svg",
                    name: "UnionPay"
                };
            case "visa":
                return {
                    key: cardBrand,
                    icon: "qrc:/ui/resources/payment/visa.svg",
                    name: "Visa"
                };
            default:
                return {
                    key: "unbranded",
                    icon: "qrc:/ui/resources/payment/unbranded.svg",
                    name: "Credit card"
                };
        }
    }
}
