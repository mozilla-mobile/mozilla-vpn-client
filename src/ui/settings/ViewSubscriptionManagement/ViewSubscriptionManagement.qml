/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1

VPNFlickable {
    id: vpnFlickable
    objectName: "settingsView"
    property string _menuTitle: VPNl18n.SubscriptionManagementMenuTitle
    property var currentLanguageCode: VPNLocalizer.code === ""
        ? Qt.locale(VPNLocalizer.previousCode)
        : Qt.locale(VPNLocalizer.code)

    anchors.top: parent.top
    height: parent.height
    flickContentHeight: contentColumn.implicitHeight + VPNTheme.theme.rowHeight
    windowHeightExceedsContentHeight: !(flickContentHeight > height)

    ListModel {
        id: subscriptionInfoModel
    }

    ListModel {
        id: subscriptionPaymentModel
    }

    ColumnLayout {
        id: contentColumn

        height: Math.max(vpnFlickable.height - VPNTheme.theme.menuHeight, contentColumn.implicitHeight)
        spacing: VPNTheme.theme.windowMargin * 2
        width: parent.width - VPNTheme.theme.windowMargin

        anchors {
            horizontalCenter: parent.horizontalCenter
            top: parent.top
        }

        VPNUserProfile {
            _iconButtonImageSource: "qrc:/nebula/resources/open-in-new.svg"
            _iconButtonOnClicked: () => handleManageAccountClicked()

            Layout.leftMargin: VPNTheme.theme.windowMargin / 2
            Layout.topMargin: VPNTheme.theme.windowMargin * 2
        }

        ColumnLayout {
            spacing: 0

            Layout.leftMargin: VPNTheme.theme.windowMargin / 2
            Layout.rightMargin: VPNTheme.theme.windowMargin / 2

            VPNMetropolisLabel {
                color: VPNTheme.theme.fontColorDark
                horizontalAlignment: Text.AlignLeft
                font.family: VPNTheme.theme.fontBoldFamily
                text: VPNl18n.SubscriptionManagementSummaryHeadline

                Layout.bottomMargin: VPNTheme.theme.listSpacing * 0.5
                Layout.fillWidth: true
            }

            Repeater {
                model: subscriptionInfoModel
                delegate: Loader {
                    Layout.fillWidth: true
                    source: "qrc:/ui/settings/ViewSubscriptionManagement/SubscriptionManagementItem.qml"
                }
            }

            VPNMetropolisLabel {
                color: VPNTheme.theme.fontColorDark
                horizontalAlignment: Text.AlignLeft
                font.family: VPNTheme.theme.fontBoldFamily
                text: VPNl18n.SubscriptionManagementPaymentHeadline

                Layout.topMargin: VPNTheme.theme.windowMargin * 2
                Layout.bottomMargin: VPNTheme.theme.listSpacing * 0.5
                Layout.fillWidth: true
            }

            Repeater {
                model: subscriptionPaymentModel
                delegate: Loader {
                    Layout.fillWidth: true
                    source: "qrc:/ui/settings/ViewSubscriptionManagement/SubscriptionManagementItem.qml"
                }
            }

            VPNButton {
                id: manageSubscriptionButton

                onClicked: {
                    handleManageAccountClicked();
                }
                text: VPNl18n.SubscriptionManagementManageSubscriptionButton
                width: undefined

                Layout.topMargin: VPNTheme.theme.windowMargin * 2
                Layout.fillWidth: true
            }

            VPNLinkButton {
                fontName: VPNTheme.theme.fontBoldFamily
                labelText: VPNl18n.DeleteAccountButtonLabel
                linkColor: VPNTheme.theme.redButton
                visible: VPNFeatureList.get("accountDeletion").isSupported

                onClicked: {
                    settingsStackView.push("qrc:/ui/deleteAccount/ViewDeleteAccount.qml");
                }

                Layout.alignment: Qt.AlignHCenter
                Layout.topMargin: VPNTheme.theme.vSpacing
            }

            VPNVerticalSpacer {
                Layout.fillWidth: true
                Layout.minimumHeight: VPNTheme.theme.rowHeight
            }
        }
    }

    function handleManageAccountClicked() {
        Sample.manageAccountClicked.record();
        VPN.openLink(VPN.LinkAccount);
    }

    function populateListModels() {
        // Subscription info model
        subscriptionInfoModel.append({
            labelText: VPNl18n.SubscriptionManagementPlanLabel,
            valueText: getPlanText(
                VPNSubscriptionData.planCurrency,
                VPNSubscriptionData.planAmount,
                VPNSubscriptionData.planIntervalCount
            ),
            type: "text",
        });

        subscriptionInfoModel.append({
            labelText: VPNl18n.SubscriptionManagementStatusLabel,
            valueText: VPNSubscriptionData.status,
            type: "pill",
        });

        subscriptionInfoModel.append({
            labelText: VPNl18n.SubscriptionManagementNextLabel,
            valueText: unixToDate(VPNSubscriptionData.expiresOn),
            type: "text",
        });

        subscriptionInfoModel.append({
            labelText: VPNl18n.SubscriptionManagementActivatedLabel,
            valueText: unixToDate(VPNSubscriptionData.createdAt),
            type: "text",
        });

        subscriptionInfoModel.append({
            labelText: VPNl18n.SubscriptionManagementExpiresLabel,
            valueText: unixToDate(VPNSubscriptionData.expiresOn),
            type: "text",
        });

        // Subscription payment model
        if (VPNSubscriptionData.paymentType === "credit") {
            subscriptionPaymentModel.append({
                labelText: VPNSubscriptionData.creditCardBrand,
                valueText: VPNl18n.SubscriptionManagementCardLast4.arg(VPNSubscriptionData.creditCardLast4),
                type: "creditCard",
            });

            subscriptionPaymentModel.append({
                labelText: VPNl18n.SubscriptionManagementCardExpiresLabel,
                valueText: VPNSubscriptionData.creditCardExpMonth + " " + VPNSubscriptionData.creditCardExpYear, // TODO: Format and localize date
                type: "text",
            });
        } else {
            subscriptionPaymentModel.append({
                labelText: VPNl18n.SubscriptionManagementSubscriptionPlatformLabel.arg("IAP Google or Apple"),
                valueText: "",
                type: "text",
            });
        }
    }

    function unixToDate(unixTimestamp) {
        return new Date(unixTimestamp * 1000).toLocaleDateString(currentLanguageCode, Locale.ShortFormat);
    }

    function getPlanText(currencyCode, amount, intervalCount) {
        const amountDisplay = (amount || 0) / 100;

        const currencySymbol = getCurrencySymbolFromCode(currencyCode);
        const localizedCurrency = Number(amountDisplay).toLocaleCurrencyString(currentLanguageCode, currencySymbol);

        if (intervalCount === 12) {
            return VPNl18n.SubscriptionManagementPlanValueYearly.arg(localizedCurrency);
        } else if (intervalCount === 6) {
            return VPNl18n.SubscriptionManagementPlanValueHalfYearly.arg(localizedCurrency);
        } else if (intervalCount === 1) {
            return VPNl18n.SubscriptionManagementPlanValueMonthly.arg(localizedCurrency);
        }

        // TODO: Confirm that’s really the case: If we made it here the user hass a free trial.
        return VPNl18n.FreeTrialsFreeTrialLabel;
    }

    Component.onCompleted: {
        populateListModels();
    }

    // TODO: Get a list with currencies
    function getCurrencySymbolFromCode(code) {
        const currencies = [
            {
                code: "eur",
                sign: "€"
            },
            {
                code: "usd",
                sign: "$"
            }
        ];
        const selectedCurrency = currencies.find((currency) => currency.code === code);

        return selectedCurrency ? selectedCurrency.sign : code.toUpperCase();
    }
}
