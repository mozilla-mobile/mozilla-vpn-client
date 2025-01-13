/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import "qrc:/nebula/utils/MZAssetLookup.js" as MZAssetLookup

RowLayout {
    id: root
    property string paymentMethod: "unbranded"
    property var selectedPaymentMethod: getPaymentMethod()

    visible: true

    MZInterLabel {
        id: label
        objectName: "paymentLabel"

        horizontalAlignment: Text.AlignLeft
        font.pixelSize: MZTheme.theme.fontSizeSmall
        text: selectedPaymentMethod.name
        visible: typeof(text) !== "undefined" && text.length > 0
        wrapMode: Text.WordWrap
    }

    MZIcon {
        id: icon

        source: selectedPaymentMethod.icon
        sourceSize.height: MZTheme.theme.iconSizeSmall * 1.5
        sourceSize.width: MZTheme.theme.iconSizeSmall * 1.5

        Layout.alignment: Qt.AlignVCenter
    }

    function getPaymentMethod() {
        switch(paymentMethod) {
            case "amex":
                return {
                    icon: MZAssetLookup.getImageSource("PaymentAmex"),
                    name: MZI18n.PaymentMethodsAmex
                };
            case "diners":
                return {
                    icon: MZAssetLookup.getImageSource("PaymentDiners"),
                    name: MZI18n.PaymentMethodsDiners
                };
            case "discover":
                return {
                    icon: MZAssetLookup.getImageSource("PaymentDiscover"),
                    name: MZI18n.PaymentMethodsDiscover
                };
            case "jcb":
                return {
                    icon: MZAssetLookup.getImageSource("PaymentJcb"),
                    name: MZI18n.PaymentMethodsJcb
                };
            case "iap_apple":
                return {
                    icon: MZAssetLookup.getImageSource("PaymentApple"),
                    name: MZI18n.PaymentMethodsIapApple
                };
            case "iap_google":
                return {
                    icon: MZAssetLookup.getImageSource("PaymentGoogle"),
                    name: MZI18n.PaymentMethodsIapGoogle
                };
            case "mastercard":
                return {
                    icon: MZAssetLookup.getImageSource("PaymentMastercard"),
                    name: MZI18n.PaymentMethodsMastercard
                };
            case "paypal":
                return {
                    icon: MZAssetLookup.getImageSource("PaymentPayPal"),
                    name: MZI18n.PaymentMethodsPaypal
                };
            case "unionpay":
                return {
                    icon: MZAssetLookup.getImageSource("PaymentUnionPay"),
                    name: MZI18n.PaymentMethodsUnionpay
                };
            case "visa":
                return {
                    icon: MZAssetLookup.getImageSource("PaymentVisa"),
                    name: MZI18n.PaymentMethodsVisa
                };
            default:
                return {
                    icon: MZAssetLookup.getImageSource("PaymentUnbranded"),
                    name: ""
                };
        }
    }
}
