/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1

VPNFlickable {
    property string _menuTitle: VPNl18n.SubscriptionManagementMenuTitle
    id: vpnFlickable
    objectName: "subscriptionManagmentView"

    anchors.top: parent.top
    height: parent.height
    flickContentHeight: contentColumn.implicitHeight + VPNTheme.theme.rowHeight

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
            _iconButtonOnClicked: () => {
                VPN.recordGleanEvent("manageAccountClicked");
                VPN.openLink(VPN.LinkAccount);
            }

            Layout.leftMargin: VPNTheme.theme.windowMargin / 2
            Layout.topMargin: VPNTheme.theme.windowMargin * 2
        }

        ColumnLayout {
            spacing: 0
            objectName: "subscriptionItem"

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
                    objectName: _objectName
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
                    objectName: _objectName
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
                    mainStackView.push("qrc:/ui/deleteAccount/ViewDeleteAccount.qml");
                    VPN.recordGleanEvent("deleteAccountRequested");
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
        switch(VPNSubscriptionData.type) {
            case VPNSubscriptionData.web:
                VPN.openLink(VPN.LinkSubscriptionFxa);
                break;
            case VPNSubscriptionData.iap_google:
                VPN.openLink(VPN.LinkSubscriptionIapGoogle);
                break;
            case VPNSubscriptionData.iap_apple:
                VPN.openLink(VPN.LinkSubscriptionIapApple);
                break;
            default:
                VPN.openLink(VPN.LinkAccount);
        }

        VPN.recordGleanEvent("manageSubscriptionClicked");
    }

    function populateListModels() {
        // Subscription info model
        // Subscription plan
        subscriptionInfoModel.append({
            _objectName: "subscriptionItem-plan",
            labelText: VPNl18n.SubscriptionManagementPlanLabel,
            valueText: getPlanText(
                VPNSubscriptionData.planCurrency,
                VPNSubscriptionData.planAmount,
                VPNSubscriptionData.planIntervalCount
            ),
            type: "text",
        });

        // Status
        subscriptionInfoModel.append({
            _objectName: "subscriptionItem-status",
            labelText: VPNl18n.SubscriptionManagementStatusLabel,
            valueText: VPNSubscriptionData.status,
            type: "pill",
        });

        // Created at
        if (VPNSubscriptionData.createdAt) {
            subscriptionInfoModel.append({
                _objectName: "subscriptionItem-activated",
                labelText: VPNl18n.SubscriptionManagementActivatedLabel,
                valueText: epochTimeToDate(VPNSubscriptionData.createdAt),
                type: "text",
            });
        }

        // Expires or next billed
        subscriptionInfoModel.append({
            _objectName: "subscriptionItem-cancelled",
            labelText: VPNSubscriptionData.isCancelled
                ? VPNl18n.SubscriptionManagementExpiresLabel
                : VPNl18n.SubscriptionManagementNextLabel,
            valueText: epochTimeToDate(VPNSubscriptionData.expiresOn),
            type: "text",
        });

        // Subscription payment model
        if (VPNSubscriptionData.paymentProvider) {
            if (VPNSubscriptionData.paymentType === "credit") {
                // Credit card brand
                subscriptionPaymentModel.append({
                    _objectName: "subscriptionItem-brand",
                    labelText: VPNSubscriptionData.creditCardBrand,
                    valueText: VPNl18n.SubscriptionManagementCardLast4.arg(VPNSubscriptionData.creditCardLast4),
                    type: "payment",
                });

                // Credit card expires
                subscriptionPaymentModel.append({
                    _objectName: "subscriptionItem-expires",
                    labelText: VPNl18n.SubscriptionManagementCardExpiresLabel,
                    valueText: getPaymentExpiration(),
                    type: "text",
                });
            } else {
                // Payment type or provider
                subscriptionPaymentModel.append({
                    _objectName: "subscriptionItem-payment",
                    labelText: VPNSubscriptionData.paymentType || VPNSubscriptionData.paymentProvider,
                    valueText: "",
                    type: "payment",
                });
            }
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

    function getPlanText(currencyCode, amount, intervalCount) {
        const amountDisplay = (amount || 0) / 100;
        const localizedCurrency = VPNLocalizer.localizeCurrency(amountDisplay, currencyCode);

        let labelText;

        if (intervalCount === 12) {
            // {¤amount} Yearly
            labelText = VPNl18n.SubscriptionManagementPlanValueYearly.arg(localizedCurrency);
        } else if (intervalCount === 6) {
            // {¤amount} Half-yearly
            labelText = VPNl18n.SubscriptionManagementPlanValueHalfYearly.arg(localizedCurrency);
        } else if (intervalCount === 1) {
            // {¤amount} Monthly
            labelText = VPNl18n.SubscriptionManagementPlanValueMonthly.arg(localizedCurrency);
        } else {
            console.warn(`Unexpected value for intervalCount: ${intervalCount}`);
            VPN.recordGleanEventWithExtraKeys("unhandledSubPlanInterval", {
                "interval_count": intervalCount,
            });
        }

        return labelText;
    }

    Component.onCompleted: {
        populateListModels();
    }
}
