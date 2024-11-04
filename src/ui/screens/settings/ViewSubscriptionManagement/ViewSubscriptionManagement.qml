/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1

MZViewBase {
    id: vpnFlickable
    objectName: "subscriptionManagmentView"

    property string telemetryScreenId: isAnnualUpgradeAvailable() && VPNSubscriptionData.type === VPNSubscriptionData.SubscriptionWeb ? "account_with_change_plan" : "account"

    Component.onDestruction: () => VPNProfileFlow.reset()

    _menuTitle: MZI18n.SubscriptionManagementSectionTitle
    _menuOnBackClicked: () => { stackview.pop(null) }
    _viewContentData: ColumnLayout {
        Layout.preferredWidth: parent.width
        spacing: MZTheme.theme.windowMargin

        ListModel {
            id: subscriptionInfoModel
        }

        ListModel {
            id: subscriptionPaymentModel
        }

        MZUserProfile {
            Layout.leftMargin: MZTheme.theme.windowMargin / 2
            Layout.rightMargin: MZTheme.theme.windowMargin / 2

            _objNameBase: "subscriptionUserProfile"

            _iconSource: "qrc:/nebula/resources/open-in-new.svg"
            _buttonOnClicked: () => {
                Glean.interaction.editSelected.record({
                    screen: vpnFlickable.telemetryScreenId,
                });
                MZUrlOpener.openUrlLabel("account");
            }
        }

        ColumnLayout {
            spacing: 0
            objectName: "subscriptionItem"

            Layout.alignment: Qt.AlignTop
            Layout.leftMargin: MZTheme.theme.windowMargin
            Layout.rightMargin: MZTheme.theme.windowMargin
            Layout.preferredWidth: parent.width


            MZMetropolisLabel {
                color: MZTheme.colors.fontColorDark
                horizontalAlignment: Text.AlignLeft
                font.family: MZTheme.theme.fontBoldFamily
                text: MZI18n.SubscriptionManagementSummaryHeadline

                Layout.bottomMargin: MZTheme.theme.listSpacing * 0.5
                Layout.topMargin: MZTheme.theme.listSpacing * 0.5
                Layout.fillWidth: true
            }

            Repeater {
                model: subscriptionInfoModel
                delegate: Loader {
                    objectName: _objectName
                    Layout.fillWidth: true
                    sourceComponent: SubscriptionManagementItem {
                        telemetryScreenId: vpnFlickable.telemetryScreenId
                    }
                }
            }

            MZMetropolisLabel {
                color: MZTheme.colors.fontColorDark
                horizontalAlignment: Text.AlignLeft
                font.family: MZTheme.theme.fontBoldFamily
                text: MZI18n.SubscriptionManagementPaymentHeadline

                Layout.topMargin: MZTheme.theme.windowMargin * 2
                Layout.bottomMargin: MZTheme.theme.listSpacing * 0.5
                Layout.fillWidth: true
            }

            Repeater {
                model: subscriptionPaymentModel
                delegate: Loader {
                    Layout.fillWidth: true
                    objectName: _objectName
                    source: "qrc:/qt/qml/Mozilla/VPN/screens/settings/ViewSubscriptionManagement/SubscriptionManagementItem.qml"
                }
            }

            MZButton {
                objectName: "manageSubscriptionButton"
                id: manageSubscriptionButton

                onClicked: handleManageAccountClicked()
                text: MZI18n.SubscriptionManagementManageSubscriptionButton
                width: undefined

                Layout.topMargin: MZTheme.theme.windowMargin * 2
                Layout.fillWidth: true
            }

            MZLinkButton {
                objectName: "accountDeletionButton"
                fontName: MZTheme.theme.fontBoldFamily
                labelText: MZI18n.DeleteAccountButtonLabel
                linkColor: MZTheme.colors.redLinkButton
                visible: MZFeatureList.get("accountDeletion").isSupported

                onClicked: {
                    MZNavigator.requestScreen(VPN.ScreenDeleteAccount)
                }

                Layout.alignment: Qt.AlignHCenter
                Layout.topMargin: MZTheme.theme.vSpacing
            }

            MZSignOut {
                id: signOutLink

                objectName: "accountLogout"
                colorStrength: MZSignOut.ColorStrength.Soft
                anchors {
                    horizontalCenter: undefined
                    bottom: undefined
                    bottomMargin: undefined
                }
                Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
                Layout.topMargin: MZTheme.theme.vSpacingSmall - parent.spacing

                preLogoutCallback: () => {
                    Glean.interaction.signOutSelected.record({
                        screen: vpnFlickable.telemetryScreenId,
                    });
                }
            }
        }
    }

    function handleManageAccountClicked() {
        switch(VPNSubscriptionData.type) {
            case VPNSubscriptionData.SubscriptionWeb:
                MZUrlOpener.openUrlLabel("subscriptionFxa");
                break;
            case VPNSubscriptionData.SubscriptionGoogle:
                MZUrlOpener.openUrlLabel("subscriptionIapGoogle");
                break;
            case VPNSubscriptionData.SubscriptionApple:
                MZUrlOpener.openUrlLabel("subscriptionIapApple");
                break;
            default:
                MZUrlOpener.openUrlLabel("account");
        }

        Glean.interaction.manageSubscriptionSelected.record({
            screen: vpnFlickable.telemetryScreenId,
        });
    }

    function isAnnualUpgradeAvailable() {
        // Annual upgrade UI is only shown to users with monthly subscriptions purchased via the web
        // And only when the annualUpgrade feature flag is enabled
        // The check for web-based subscriptions occurs on L160
        if (!MZFeatureList.get("annualUpgrade").isSupported) {
            return false;
        }
        return VPNSubscriptionData.planBillingInterval ==  VPNSubscriptionData.BillingIntervalMonthly;
    }

    function populateListModels() {
        // Subscription info model
        // Subscription plan
        if (
            VPNSubscriptionData.planCurrency
            && VPNSubscriptionData.planAmount
            && VPNSubscriptionData.type === VPNSubscriptionData.SubscriptionWeb
        ) {
            subscriptionInfoModel.append({
                _objectName: "subscriptionItem-plan",
                labelText: MZI18n.SubscriptionManagementPlanLabel,
                valueText: getPlanText(
                    VPNSubscriptionData.planCurrency,
                    VPNSubscriptionData.planAmount,
                ),
                type: isAnnualUpgradeAvailable()? "text-upgrade" : "text",
            });
        }

        // Status
        subscriptionInfoModel.append({
            _objectName: "subscriptionItem-status",
            labelText: MZI18n.SubscriptionManagementStatusLabel,
            valueText: VPNSubscriptionData.status === VPNSubscriptionData.Active
                ? MZI18n.SubscriptionManagementStatusActive
                : MZI18n.SubscriptionManagementStatusInactive,
            type: "pill",
        });

        // Created at
        if (VPNSubscriptionData.createdAt) {
            subscriptionInfoModel.append({
                _objectName: "subscriptionItem-activated",
                labelText: MZI18n.SubscriptionManagementActivatedLabel,
                valueText: epochTimeToDate(VPNSubscriptionData.createdAt),
                type: "text",
            });
        }

        // Expires or next billed
        subscriptionInfoModel.append({
            _objectName: "subscriptionItem-cancelled",
            labelText: VPNSubscriptionData.isCancelled
                ? MZI18n.SubscriptionManagementExpiresLabel
                : MZI18n.SubscriptionManagementNextLabel,
            valueText: epochTimeToDate(VPNSubscriptionData.expiresOn),
            type: "text",
        });

        // Subscription payment model
        if (
            VPNSubscriptionData.type === VPNSubscriptionData.SubscriptionWeb
            && VPNSubscriptionData.paymentProvider
        ) {
            if (
                VPNSubscriptionData.creditCardBrand
                && VPNSubscriptionData.creditCardLast4
            ) {
                // Credit card brand
                subscriptionPaymentModel.append({
                    _objectName: "subscriptionItem-brand",
                    labelText: VPNSubscriptionData.creditCardBrand,
                    valueText: MZI18n.SubscriptionManagementCardLast4.arg(VPNSubscriptionData.creditCardLast4),
                    type: "payment",
                });

                if (
                    VPNSubscriptionData.creditCardExpMonth
                    && VPNSubscriptionData.creditCardExpYear
                ) {
                    // Credit card expires
                    subscriptionPaymentModel.append({
                        _objectName: "subscriptionItem-expires",
                        labelText: MZI18n.SubscriptionManagementCardExpiresLabel,
                        valueText: getPaymentExpiration(),
                        type: "text",
                    });
                }
            } else {
                // Payment provider
                subscriptionPaymentModel.append({
                    _objectName: "subscriptionItem-payment",
                    labelText: VPNSubscriptionData.paymentProvider,
                    valueText: "",
                    type: "payment",
                });
            }
        } else if (VPNSubscriptionData.type === VPNSubscriptionData.SubscriptionApple) {
            // IAP Apple
            subscriptionPaymentModel.append({
                _objectName: "subscriptionItem-payment",
                labelText: "iap_apple",
                valueText: "",
                type: "payment",
            });
        } else if (VPNSubscriptionData.type === VPNSubscriptionData.SubscriptionGoogle) {
            // IAP Google
            subscriptionPaymentModel.append({
                _objectName: "subscriptionItem-payment",
                labelText: "iap_google",
                valueText: "",
                type: "payment",
            });
        }
    }

    function epochTimeToDate(unixTimestamp) {
        return new Date(unixTimestamp).toLocaleDateString(MZLocalizer.locale, Locale.ShortFormat);
    }

    function getPaymentExpiration() {
        return (
            MZLocalizer.locale.monthName(VPNSubscriptionData.creditCardExpMonth - 1)
            + " "
            + VPNSubscriptionData.creditCardExpYear
        );
    }

    function getPlanText(currencyCode, amount) {
        const amountDisplay = (amount || 0) / 100;
        const localizedCurrency = MZLocalizer.localizeCurrency(amountDisplay, currencyCode);

        switch (VPNSubscriptionData.planBillingInterval) {
            case VPNSubscriptionData.BillingIntervalMonthly:
                // {¤amount} Monthly
                return VPNSubscriptionData.planRequiresTax ? MZI18n.SubscriptionManagementPlanValueMonthlyPlusTax.arg(localizedCurrency)
                                : MZI18n.SubscriptionManagementPlanValueMonthly.arg(localizedCurrency);
            case VPNSubscriptionData.BillingIntervalHalfYearly:
                // {¤amount} Half-yearly
                return VPNSubscriptionData.planRequiresTax ? MZI18n.SubscriptionManagementPlanValueHalfYearlyPlusTax.arg(localizedCurrency)
                                : MZI18n.SubscriptionManagementPlanValueHalfYearly.arg(localizedCurrency);
            case VPNSubscriptionData.BillingIntervalYearly:
                // {¤amount} Yearly
                return VPNSubscriptionData.planRequiresTax ? MZI18n.SubscriptionManagementPlanValueYearlyPlusTax.arg(localizedCurrency)
                                : MZI18n.SubscriptionManagementPlanValueYearly.arg(localizedCurrency);
            default:
                // If we made it here something went wrong. In case we encounter
                // an unhandled TypeBillingInterval we should have should have
                // already handled this SubscriptionData::fromJson and not
                // render this view at all.
                throw new Error("ViewSubscriptionManagement out of sync");
                console.warn("Unhandled billing interval.");
                return "";
        }
    }

    Component.onCompleted: {
        populateListModels();
        MZNavigator.addView(VPN.ScreenSettings, vpnFlickable)
        Glean.impression.accountScreen.record({
            screen: telemetryScreenId,
        });
    }
}
