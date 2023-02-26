/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1

VPNViewBase {
    id: vpnFlickable
    objectName: "subscriptionManagmentView"

    Component.onDestruction: () => VPNProfileFlow.reset()

    _menuTitle: VPNI18n.SubscriptionManagementMenuTitle
    _menuOnBackClicked: () => { stackview.pop(null) }
    _viewContentData: ColumnLayout {
        spacing: VPNTheme.theme.windowMargin

        ListModel {
            id: subscriptionInfoModel
        }

        ListModel {
            id: subscriptionPaymentModel
        }

        VPNUserProfile {
            Layout.leftMargin: VPNTheme.theme.windowMargin / 2
            Layout.rightMargin: VPNTheme.theme.windowMargin / 2

            objectName: "subscriptionUserProfile"
            _objNameBase: "subscriptionUserProfile"

            _iconSource: "qrc:/nebula/resources/open-in-new.svg"
            _buttonOnClicked: () => {
                MZGleanDeprecated.recordGleanEvent("manageAccountClicked");
                Glean.sample.manageAccountClicked.record();
                VPNUrlOpener.openUrlLabel("account");
            }
        }

        ColumnLayout {
            spacing: 0
            objectName: "subscriptionItem"

            Layout.alignment: Qt.AlignTop
            Layout.leftMargin: VPNTheme.theme.windowMargin
            Layout.rightMargin: VPNTheme.theme.windowMargin


            VPNMetropolisLabel {
                color: VPNTheme.theme.fontColorDark
                horizontalAlignment: Text.AlignLeft
                font.family: VPNTheme.theme.fontBoldFamily
                text: VPNI18n.SubscriptionManagementSummaryHeadline

                Layout.bottomMargin: VPNTheme.theme.listSpacing * 0.5
                Layout.topMargin: VPNTheme.theme.listSpacing * 0.5
                Layout.fillWidth: true
            }

            Repeater {
                model: subscriptionInfoModel
                delegate: Loader {
                    objectName: _objectName
                    Layout.fillWidth: true
                    source: "qrc:/ui/screens/settings/ViewSubscriptionManagement/SubscriptionManagementItem.qml"
                }
            }

            VPNMetropolisLabel {
                color: VPNTheme.theme.fontColorDark
                horizontalAlignment: Text.AlignLeft
                font.family: VPNTheme.theme.fontBoldFamily
                text: VPNI18n.SubscriptionManagementPaymentHeadline

                Layout.topMargin: VPNTheme.theme.windowMargin * 2
                Layout.bottomMargin: VPNTheme.theme.listSpacing * 0.5
                Layout.fillWidth: true
            }

            Repeater {
                model: subscriptionPaymentModel
                delegate: Loader {
                    Layout.fillWidth: true
                    objectName: _objectName
                    source: "qrc:/ui/screens/settings/ViewSubscriptionManagement/SubscriptionManagementItem.qml"
                }
            }

            VPNButton {
                objectName: "manageSubscriptionButton"
                id: manageSubscriptionButton

                onClicked: handleManageAccountClicked()
                text: VPNI18n.SubscriptionManagementManageSubscriptionButton
                width: undefined

                Layout.topMargin: VPNTheme.theme.windowMargin * 2
                Layout.fillWidth: true
            }

            VPNLinkButton {
                objectName: "accountDeletionButton"
                fontName: VPNTheme.theme.fontBoldFamily
                labelText: VPNI18n.DeleteAccountButtonLabel
                linkColor: VPNTheme.theme.redLinkButton
                visible: VPNFeatureList.get("accountDeletion").isSupported

                onClicked: {
                    MZGleanDeprecated.recordGleanEvent("deleteAccountRequested");
                    VPNNavigator.requestScreen(VPNNavigator.ScreenDeleteAccount)
                }

                Layout.alignment: Qt.AlignHCenter
                Layout.topMargin: VPNTheme.theme.vSpacing
            }
        }
    }

    function handleManageAccountClicked() {
        switch(VPNSubscriptionData.type) {
            case VPNSubscriptionData.SubscriptionWeb:
                VPNUrlOpener.openUrlLabel("subscriptionFxa");
                break;
            case VPNSubscriptionData.SubscriptionGoogle:
                VPNUrlOpener.openUrlLabel("subscriptionIapGoogle");
                break;
            case VPNSubscriptionData.SubscriptionApple:
                VPNUrlOpener.openUrlLabel("subscriptionIapApple");
                break;
            default:
                VPNUrlOpener.openUrlLabel("account");
        }

        MZGleanDeprecated.recordGleanEvent("manageSubscriptionClicked");
        Glean.sample.manageSubscriptionClicked.record();
    }

    // We show the bundle upgrade only supposed to be available to users that
    // have a VPN web subscription, are located in the US or Cananada and are
    // not already bundle subscribers.
    function isBundleUpgradeAvailable() {
        if (
            !VPNFeatureList.get("bundleUpgrade").isSupported ||
            VPNSubscriptionData.type !== VPNSubscriptionData.SubscriptionWeb
        ) {
            return false;
        }

        // We use the currency as a proxy to determine if the upgrade is
        // available to a user.
        const bundleUpgradeWhitelist = ["USD", "CAD"];
        return !VPNSubscriptionData.isPrivacyBundleSubscriber &&
            bundleUpgradeWhitelist.includes(VPNSubscriptionData.planCurrency);
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
                labelText: VPNI18n.SubscriptionManagementPlanLabel,
                valueText: getPlanText(
                    VPNSubscriptionData.planCurrency,
                    VPNSubscriptionData.planAmount,
                ),
                type: isBundleUpgradeAvailable() ? "text-upgrade" : "text",
            });
        }

        // Status
        subscriptionInfoModel.append({
            _objectName: "subscriptionItem-status",
            labelText: VPNI18n.SubscriptionManagementStatusLabel,
            valueText: VPNSubscriptionData.status === VPNSubscriptionData.Active
                ? VPNI18n.SubscriptionManagementStatusActive
                : VPNI18n.SubscriptionManagementStatusInactive,
            type: "pill",
        });

        // Created at
        if (VPNSubscriptionData.createdAt) {
            subscriptionInfoModel.append({
                _objectName: "subscriptionItem-activated",
                labelText: VPNI18n.SubscriptionManagementActivatedLabel,
                valueText: epochTimeToDate(VPNSubscriptionData.createdAt),
                type: "text",
            });
        }

        // Expires or next billed
        subscriptionInfoModel.append({
            _objectName: "subscriptionItem-cancelled",
            labelText: VPNSubscriptionData.isCancelled
                ? VPNI18n.SubscriptionManagementExpiresLabel
                : VPNI18n.SubscriptionManagementNextLabel,
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
                    valueText: VPNI18n.SubscriptionManagementCardLast4.arg(VPNSubscriptionData.creditCardLast4),
                    type: "payment",
                });

                if (
                    VPNSubscriptionData.creditCardExpMonth
                    && VPNSubscriptionData.creditCardExpYear
                ) {
                    // Credit card expires
                    subscriptionPaymentModel.append({
                        _objectName: "subscriptionItem-expires",
                        labelText: VPNI18n.SubscriptionManagementCardExpiresLabel,
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
        return new Date(unixTimestamp).toLocaleDateString(VPNLocalizer.locale, Locale.ShortFormat);
    }

    function getPaymentExpiration() {
        return (
            VPNLocalizer.locale.monthName(VPNSubscriptionData.creditCardExpMonth - 1)
            + " "
            + VPNSubscriptionData.creditCardExpYear
        );
    }

    function getPlanText(currencyCode, amount) {
        const amountDisplay = (amount || 0) / 100;
        const localizedCurrency = VPNLocalizer.localizeCurrency(amountDisplay, currencyCode);

        switch (VPNSubscriptionData.planBillingInterval) {
            case VPNSubscriptionData.BillingIntervalMonthly:
                // {¤amount} Monthly
                return VPNSubscriptionData.planRequiresTax ? VPNI18n.SubscriptionManagementPlanValueMonthlyPlusTax.arg(localizedCurrency)
                                : VPNI18n.SubscriptionManagementPlanValueMonthly.arg(localizedCurrency);
            case VPNSubscriptionData.BillingIntervalHalfYearly:
                // {¤amount} Half-yearly
                return VPNSubscriptionData.planRequiresTax ? VPNI18n.SubscriptionManagementPlanValueHalfYearlyPlusTax.arg(localizedCurrency)
                                : VPNI18n.SubscriptionManagementPlanValueHalfYearly.arg(localizedCurrency);
            case VPNSubscriptionData.BillingIntervalYearly:
                // {¤amount} Yearly
                return VPNSubscriptionData.planRequiresTax ? VPNI18n.SubscriptionManagementPlanValueYearlyPlusTax.arg(localizedCurrency)
                                : VPNI18n.SubscriptionManagementPlanValueYearly.arg(localizedCurrency);
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
    }
}
