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
        visible: typeof(text) !== "undefined" && text.length > 0
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
                    name: VPNl18n.PaymentMethodsAmex
                };
            case "diners":
                return {
                    key: cardBrand,
                    icon: "qrc:/ui/resources/payment/diners.svg",
                    name: VPNl18n.PaymentMethodsDiners
                };
            case "discover":
                return {
                    key: cardBrand,
                    icon: "qrc:/ui/resources/payment/discover.svg",
                    name: VPNl18n.PaymentMethodsDiscover
                };
            case "jcb":
                return {
                    key: cardBrand,
                    icon: "qrc:/ui/resources/payment/jcb.svg",
                    name: VPNl18n.PaymentMethodsJcb
                };
            case "mastercard":
                return {
                    key: cardBrand,
                    icon: "qrc:/ui/resources/payment/mastercard.svg",
                    name: VPNl18n.PaymentMethodsMastercard
                };
            case "paypal":
                return {
                    key: cardBrand,
                    icon: "qrc:/ui/resources/payment/paypal.svg",
                    name: VPNl18n.PaymentMethodsPaypal
                };
            case "unionpay":
                return {
                    key: cardBrand,
                    icon: "qrc:/ui/resources/payment/unionpay.svg",
                    name: VPNl18n.PaymentMethodsUnionpay
                };
            case "visa":
                return {
                    key: cardBrand,
                    icon: "qrc:/ui/resources/payment/visa.svg",
                    name: VPNl18n.PaymentMethodsVisa
                };
            default:
                return {
                    key: "unbranded",
                    icon: "qrc:/ui/resources/payment/unbranded.svg",
                    name: ""
                };
        }
    }
}
