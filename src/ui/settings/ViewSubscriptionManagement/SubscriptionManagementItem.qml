/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1

ColumnLayout {
    spacing: 0

    RowLayout {
        spacing: VPNTheme.theme.listSpacing

        Layout.alignment: Qt.AlignVCenter
        Layout.bottomMargin: VPNTheme.theme.listSpacing
        Layout.fillWidth: true
        Layout.preferredHeight: VPNTheme.theme.rowHeight
        Layout.topMargin: VPNTheme.theme.listSpacing

        VPNInterLabel {
            id: rowLabel

            horizontalAlignment: Text.AlignLeft
            font.pixelSize: VPNTheme.theme.fontSizeSmall
            text: getLocalizedLabel(key, values)
            wrapMode: Text.WordWrap

            Layout.fillWidth: true

            VPNIcon {
                id: labelIcon

                source: "qrc:/ui/resources/logos/mastercard.svg"
                // source: "qrc:/ui/resources/logos/android.svg"
                // source: "qrc:/ui/resources/logos/apple.svg"
                sourceSize.height: VPNTheme.theme.iconSizeSmall * 1.5
                sourceSize.width: VPNTheme.theme.iconSizeSmall * 1.5
                visible: key == "payment-method-iap" || key == "payment-method-credit"

                anchors {
                    left: parent.right
                    verticalCenter: parent.verticalCenter
                }
            }
        }

        VPNInterLabel {
            id: rowText

            color: VPNTheme.theme.fontColorDark
            horizontalAlignment: Text.AlignRight
            font.pixelSize: VPNTheme.theme.fontSizeSmall
            text: parseValue(key, values)
            wrapMode: Text.WordWrap
            visible: key !== "status"

            Layout.alignment: Qt.AlignRight
            Layout.fillWidth: true
        }

        VPNPill {
            id: rowPill

            color: values.toString() === "active"
                ? VPNTheme.colors.green90
                : VPNTheme.colors.red70
            background: values.toString() === "active"
                ? VPNTheme.colors.green5
                : VPNTheme.colors.red5
            text: values.toString() === "active" ? "Active" : "Inactive"
            visible: key === "status"

            Layout.alignment: Qt.AlignRight
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

    function getLocalizedLabel(itemKey, itemValues) {
        switch (itemKey) {
            case "sub-plan":
                return VPNl18n.SubscriptionManagementPlanLabel;
            case "sub-status":
                return VPNl18n.SubscriptionManagementStatusLabel;
            case "sub-activated":
                return VPNl18n.SubscriptionManagementActivatedLabel;
            case "next-billed":
                return VPNl18n.SubscriptionManagementNextLabel;
            case "payment-method-iap":
                return VPNl18n.SubscriptionManagementPaymentMethod.arg(itemKey);
            case "payment-method-credit":
                const [paymentType, creditCardBrand, creditCardLast4] = itemValues;
                return creditCardBrand == "visa" ? "Visa" : "MasterCard";
            case "sub-expires":
            case "payment-method-expires":
                return VPNl18n.SubscriptionManagementExpiresLabel;
            default:
                return itemKey;
        }
    }

    function parseValue(itemKey, itemValues) {
        const localeCode = VPNLocalizer.code;

        switch (itemKey) {
            case "sub-plan":
                const [currency, amount, intervalCount] = itemValues;
                const localizedCurrency = Number(amount / 100).toLocaleCurrencyString(Qt.locale(localeCode), currency.toUpperCase());

                if (intervalCount === "1") {
                    return VPNl18n.SubscriptionManagementPlanValueMonthly.arg(localizedCurrency);
                } else if (intervalCount === "6") {
                    return VPNl18n.SubscriptionManagementPlanValueHalfYearly.arg(localizedCurrency);
                } else if (intervalCount === "12") {
                    return VPNl18n.SubscriptionManagementPlanValueYearly.arg(localizedCurrency);
                }
            case "sub-activated":
            case "sub-expires":
            case "sub-next-billed":
                return new Date(itemValues * 1000).toLocaleDateString(Qt.locale(localeCode), Locale.ShortFormat);
            case "payment-method-iap":
                return itemValue;
            case "payment-method-credit":
                const [paymentType, creditCardBrand, creditCardLast4] = itemValues;
                return "Card ending in " + creditCardLast4;
            case "payment-method-expires":
                const [expMonth, expYear] = itemValues;
                return new Date(parseInt(expYear), parseInt(expMonth) - 1).toLocaleDateString(Qt.locale(localeCode), Locale.ShortFormat);
            case "sub-status":
            default:
                return itemValues;
        }
    }

    function getLocaleDateString(localeCode) {
        const dateFormats = {
            "de_DE": "dd.MM.yyyy",
            "en_GB": "dd/MM/yyyy",
            "en_US": "M/d/yyyy",
        };

        return dateFormats[localeCode] || dateFormats["en_US"];
    }

}
