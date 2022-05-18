/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1

VPNFlickable {
    property string _menuTitle: "Profile"

    id: vpnFlickable
    objectName: "settingsView"

    anchors.top: parent.top
    height: parent.height
    flickContentHeight: contentColumn.implicitHeight + VPNTheme.theme.rowHeight
    windowHeightExceedsContentHeight: !(flickContentHeight > height)

    ListModel {
        id: subscriptionModel
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

        Text {
            text: VPNSubscriptionData.initialized ? ":)" : ":("
        }

        ColumnLayout {
            spacing: 0

            Layout.leftMargin: VPNTheme.theme.windowMargin / 2
            Layout.rightMargin: VPNTheme.theme.windowMargin / 2

            VPNMetropolisLabel {
                color: VPNTheme.theme.fontColorDark
                horizontalAlignment: Text.AlignLeft
                font.family: VPNTheme.theme.fontBoldFamily
                text: "Subscription summary" // TODO: Add localized strings

                Layout.bottomMargin: VPNTheme.theme.listSpacing * 0.5
                Layout.fillWidth: true
            }

            Repeater {
                model: subscriptionModel
                delegate: Loader {
                    Layout.fillWidth: true
                    source: "qrc:/ui/settings/ViewSubscriptionManagement/SubscriptionManagementItemTest.qml"
                }
            }

            VPNMetropolisLabel {
                color: VPNTheme.theme.fontColorDark
                horizontalAlignment: Text.AlignLeft
                font.family: VPNTheme.theme.fontBoldFamily
                text: "Payment information" // TODO: Add localized strings

                Layout.topMargin: VPNTheme.theme.windowMargin * 2
                Layout.bottomMargin: VPNTheme.theme.listSpacing * 0.5
                Layout.fillWidth: true
            }

            VPNButton {
                id: manageSubscriptionButton

                onClicked: {
                    handleManageAccountClicked();
                }
                text: "Manage subscription" // TODO: Add localized strings
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
        }
    }

    function handleManageAccountClicked() {
        Sample.manageAccountClicked.record();
        VPN.openLink(VPN.LinkAccount);
    }

    function populateListModel() {
        subscriptionModel.append({
            labelText: "Subscription plan",
            valueText: getPlanText(
                VPNSubscriptionData.planCurrency,
                VPNSubscriptionData.planAmount,
                VPNSubscriptionData.planIntervalCount
            ),
            type: "text",
        });

        subscriptionModel.append({
            labelText: "Status",
            valueText: VPNSubscriptionData.status,
            type: "pill",
        });

        subscriptionModel.append({
            labelText: "Next billed",
            valueText: unixToDate(VPNSubscriptionData.expiresOn),
            type: "text",
        });

        subscriptionModel.append({
            labelText: "Activated",
            valueText: unixToDate(VPNSubscriptionData.createdAt),
            type: "text",
        });

        subscriptionModel.append({
            labelText: "Expires/Expired",
            valueText: unixToDate(VPNSubscriptionData.expiresOn),
            type: "text",
        });
    }

    function unixToDate(unixTimestamp) {
        return new Date(unixTimestamp * 1000).toLocaleDateString(Qt.locale(VPNLocalizer.code), Locale.ShortFormat);
    }

    function getPlanText(currency, amount, intervalCount) {
        const localizedCurrency = Number(amount / 100).toLocaleCurrencyString(Qt.locale(VPNLocalizer.code), currency.toUpperCase());

        if (intervalCount === 12) {
            return VPNl18n.SubscriptionManagementPlanValueYearly.arg(localizedCurrency);
        } else if (intervalCount === 6) {
            return VPNl18n.SubscriptionManagementPlanValueHalfYearly.arg(localizedCurrency);
        } else if (intervalCount === 1) {
            return VPNl18n.SubscriptionManagementPlanValueMonthly.arg(localizedCurrency);
        }

        // TODO: Check if true, but probably it’s a free trial if we made it here.
        return "Free trial";
    }

    Component.onCompleted: {
        const data = '{"created_at":1626704467,"expires_on":1652970067,"is_cancelled":false,"payment":{"credit_card_brand":"visa","credit_card_exp_month":12,"credit_card_exp_year":2022,"credit_card_last4":"0016","provider":"stripe","type":"credit"},"plan":{"amount":499,"currency":"eur","interval_count":1,"interval":"month"},"status":"active","type":"web"}';

        VPNSubscriptionData.fromJson(data);

        populateListModel();
    }
}