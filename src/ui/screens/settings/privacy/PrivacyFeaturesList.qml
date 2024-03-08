/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import Mozilla.Shared 1.0
import components 0.1

ColumnLayout {
    id: root

    property bool isOnboarding: false
    property string telemetryScreenId
    property int dividerSpacing: MZTheme.theme.toggleRowDividerSpacing


    signal settingClicked(dnsProviderFlags: int, active: bool)

    spacing: dividerSpacing

    Repeater {
        id: repeater

        model: [
            {
                objectName: "blockAds",
                settingValue: MZSettings.BlockAds,
                settingTitle: MZI18n.SettingsPrivacyAdblockTitle,
                settingDescription: MZI18n.SettingsPrivacyAdblockBody,
            }, {
                objectName: "blockTrackers",
                settingValue: MZSettings.BlockTrackers,
                settingTitle: MZI18n.SettingsPrivacyTrackerTitle,
                settingDescription: MZI18n.SettingsPrivacyTrackerBody,
            }, {
                objectName: "blockMalware",
                settingValue: MZSettings.BlockMalware,
                settingTitle: MZI18n.SettingsPrivacyMalwareTitle,
                settingDescription: MZI18n.SettingsPrivacyMalwareBody2,
            }
        ];

        delegate: MZToggleRow {
            objectName: modelData.objectName

            labelText: modelData.settingTitle
            subLabelText: modelData.settingDescription
            checked: MZSettings.dnsProviderFlags & modelData.settingValue
            dividerTopMargin: dividerSpacing
            showDivider: !isOnboarding || index + 1 !== repeater.model.length

            onClicked: {
                let dnsProviderFlags = MZSettings.dnsProviderFlags;
                dnsProviderFlags &= ~MZSettings.Custom;
                dnsProviderFlags &= ~MZSettings.Gateway;

                if (dnsProviderFlags & modelData.settingValue) {
                    dnsProviderFlags &= ~modelData.settingValue;
                } else {
                    dnsProviderFlags |= modelData.settingValue;
                }

                // We are not changing anything interesting for the privacy/dns dialog.
                if (MZSettings.dnsProviderFlags !== MZSettings.Custom) {
                    MZSettings.dnsProviderFlags = dnsProviderFlags;
                    if(root.isOnboarding) recordOnboardingTelemetry()
                    return;
                }

                settingClicked(dnsProviderFlags, true)
            }

            function recordOnboardingTelemetry() {
                switch (modelData.settingValue) {
                case MZSettings.BlockAds:
                    if (MZSettings.dnsProviderFlags & modelData.settingValue) {
                        Glean.interaction.blockAdsEnabled.record({
                            screen: root.telemetryScreenId,
                        });
                    }
                    else {
                        Glean.interaction.blockAdsDisabled.record({
                            screen: root.telemetryScreenId,
                        });
                    }
                    break
                case MZSettings.BlockTrackers:
                    if (MZSettings.dnsProviderFlags & modelData.settingValue) {
                        Glean.interaction.blockTrackersEnabled.record({
                            screen: root.telemetryScreenId,
                        });
                    }
                    else {
                        Glean.interaction.blockTrackersDisabled.record({
                            screen: root.telemetryScreenId,
                        });
                    }
                    break
                case MZSettings.BlockMalware:
                    if (MZSettings.dnsProviderFlags & modelData.settingValue) {
                        Glean.interaction.blockMalwareEnabled.record({
                            screen: root.telemetryScreenId,
                        });
                    }
                    else {
                        Glean.interaction.blockMalwareDisabled.record({
                            screen: root.telemetryScreenId,
                        });
                    }
                    break
                default:
                    break
                }
            }
        }
    }
}
