/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0

RowLayout {
    id: root
    property string paymentMethod: "unbranded"
    property var selectedPaymentMethod: getPaymentMethod()

    visible: true

    VPNInterLabel {
        id: label
        objectName: "paymentLabel"

        horizontalAlignment: Text.AlignLeft
        font.pixelSize: VPNTheme.theme.fontSizeSmall
        text: selectedPaymentMethod.name
        visible: typeof(text) !== "undefined" && text.length > 0
        wrapMode: Text.WordWrap
    }

    VPNIcon {
        id: icon

        source: selectedPaymentMethod.icon
        sourceSize.height: VPNTheme.theme.iconSizeSmall * 1.5
        sourceSize.width: VPNTheme.theme.iconSizeSmall * 1.5

        Layout.alignment: Qt.AlignVCenter
    }

    function getPaymentMethod() {
        switch(paymentMethod) {
            case "amex":
                return {
                    icon: "qrc:/ui/resources/payment/amex.svg",
                    name: VPNI18n.PaymentMethodsAmex
                };
            case "diners":
                return {
                    icon: "qrc:/ui/resources/payment/diners.svg",
                    name: VPNI18n.PaymentMethodsDiners
                };
            case "discover":
                return {
                    icon: "qrc:/ui/resources/payment/discover.svg",
                    name: VPNI18n.PaymentMethodsDiscover
                };
            case "jcb":
                return {
                    icon: "qrc:/ui/resources/payment/jcb.svg",
                    name: VPNI18n.PaymentMethodsJcb
                };
            case "iap_apple":
                return {
                    icon: "qrc:/ui/resources/payment/apple.svg",
                    name: VPNI18n.PaymentMethodsIapApple
                };
            case "iap_google":
                return {
                    icon: "qrc:/ui/resources/payment/android.svg",
                    name: VPNI18n.PaymentMethodsIapGoogle
                };
            case "mastercard":
                return {
                    icon: "qrc:/ui/resources/payment/mastercard.svg",
                    name: VPNI18n.PaymentMethodsMastercard
                };
            case "paypal":
                return {
                    icon: "qrc:/ui/resources/payment/paypal.svg",
                    name: VPNI18n.PaymentMethodsPaypal
                };
            case "unionpay":
                return {
                    icon: "qrc:/ui/resources/payment/unionpay.svg",
                    name: VPNI18n.PaymentMethodsUnionpay
                };
            case "visa":
                return {
                    icon: "qrc:/ui/resources/payment/visa.svg",
                    name: VPNI18n.PaymentMethodsVisa
                };
            default:
                return {
                    icon: "qrc:/ui/resources/payment/unbranded.svg",
                    name: ""
                };
        }
    }
}
