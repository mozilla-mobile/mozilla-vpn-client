/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import Mozilla.Shared 1.0
import components 0.1

ColumnLayout {
    id: root

    property bool recordTelemetry: false
    property string telemetryScreenId

    signal settingClicked(dnsProviderFlags: int, active: bool)

    spacing: MZTheme.theme.vSpacingSmall

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
                settingDescription: MZI18n.SettingsPrivacyMalwareBody,
            }
        ];

        delegate: MZCheckBoxRow {
            objectName: modelData.objectName

            leftMargin: 0

            labelText: modelData.settingTitle
            subLabelText: modelData.settingDescription
            isChecked: MZSettings.dnsProviderFlags & modelData.settingValue
            showDivider: false
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
                    if(root.recordTelemetry) recordTelemetry()
                    return;
                }

                settingClicked(dnsProviderFlags, true)
            }

            function recordTelemetry() {
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
