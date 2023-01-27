/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1
import compat 0.1

import org.mozilla.Glean 0.30
import telemetry 0.30


VPNViewBase {
    id: root
    objectName: "privacySettingsView"

    _menuTitle: VPNl18n.SettingsPrivacySettings
    _interactive: false

    _viewContentData: ColumnLayout {
        id: tabs
        spacing: VPNTheme.theme.windowMargin * 2
        Layout.fillWidth: true

        Item {
            Layout.topMargin: VPNTheme.theme.windowMargin
            Layout.bottomMargin: VPNTheme.theme.windowMargin *3
            Layout.leftMargin: VPNTheme.theme.windowMargin * 2
            Layout.rightMargin: VPNTheme.theme.windowMargin * 2
            Layout.fillWidth: true

            VPNDropShadow {
                anchors.fill: bg
                source: bg
                horizontalOffset: 1
                verticalOffset: 1
                radius: 6
                color: VPNTheme.colors.grey60
                opacity: .15
                transparentBorder: true
                cached: true
            }

            Rectangle {
                id: bg
                anchors.fill: info
                anchors.topMargin: -VPNTheme.theme.windowMargin
                anchors.bottomMargin: anchors.topMargin
                anchors.leftMargin: -VPNTheme.theme.windowMargin
                anchors.rightMargin: anchors.leftMargin
                color: VPNTheme.theme.white
                radius: 4
            }

            RowLayout {
                id: info
                spacing: VPNTheme.theme.windowMargin * 0.75
                anchors.left: parent.left
                anchors.right: parent.right
                VPNIcon {
                    source: "qrc:/nebula/resources/info.svg"
                    Layout.alignment: Qt.AlignTop
                }
                VPNTextBlock {
                    Layout.fillWidth: true
                    width: undefined
                    text: VPNl18n.SettingsDnsSettingsWarning
                }
            }
        }

        Repeater {
            id: repeater

            delegate: VPNCheckBoxRow {
                objectName: modelData.objectName

                Layout.fillWidth: true
                Layout.rightMargin: VPNTheme.theme.windowMargin
                labelText: modelData.settingTitle
                subLabelText: modelData.settingDescription
                isChecked: VPNSettings.dnsProviderFlags & modelData.settingValue
                showDivider: false
                onClicked: {
                    let dnsProviderFlags = VPNSettings.dnsProviderFlags;
                    dnsProviderFlags &= ~VPNSettings.Custom;
                    dnsProviderFlags &= ~VPNSettings.Gateway;

                    if (dnsProviderFlags & modelData.settingValue) {
                        dnsProviderFlags &= ~modelData.settingValue;
                    } else {
                        dnsProviderFlags |= modelData.settingValue;
                    }

                    // We are not changing anything interesting for the privacy/dns dialog.
                    if (VPNSettings.dnsProviderFlags !== VPNSettings.Custom) {
                        VPNSettings.dnsProviderFlags = dnsProviderFlags;
                        return;
                    }

                    privacyOverwriteLoader.dnsProviderValue = dnsProviderFlags;
                    privacyOverwriteLoader.active = true;
                }
            }
        }

        Component.onCompleted: {
            repeater.model = [
                {
                    objectName: "blockAds",
                    settingValue: VPNSettings.BlockAds,
                    settingTitle: VPNl18n.SettingsPrivacyAdblockTitle,
                    settingDescription: VPNl18n.SettingsPrivacyAdblockBody,
                }, {
                    objectName: "blockTrackers",
                    settingValue: VPNSettings.BlockTrackers,
                    settingTitle: VPNl18n.SettingsPrivacyTrackerTitle,
                    settingDescription: VPNl18n.SettingsPrivacyTrackerBody,
                }, {
                    objectName: "blockMalware",
                    settingValue: VPNSettings.BlockMalware,
                    settingTitle: VPNl18n.SettingsPrivacyMalwareTitle,
                    settingDescription: VPNl18n.SettingsPrivacyMalwareBody,
                }
            ];
        }
    }

    Loader {
        id: privacyOverwriteLoader

        // This is the value we are going to set if the user confirms.
        property var dnsProviderValue;

        objectName: "privacyOverwriteLoader"
        active: false
        sourceComponent: VPNSimplePopup {
            id: privacyOverwritePopup

            anchors.centerIn: Overlay.overlay
            closeButtonObjectName: "privacyOverwritePopupPopupCloseButton"
            imageSrc: "qrc:/ui/resources/logo-dns-settings.svg"
            imageSize: Qt.size(80, 80)
            title: VPNl18n.DnsOverwriteDialogTitleDNS
            description: VPNl18n.DnsOverwriteDialogBodyDNS
            buttons: [
                VPNButton {
                    objectName: "privacyOverwritePopupDiscoverNowButton"
                    text: VPNl18n.DnsOverwriteDialogPrimaryButton
                    onClicked: {
                        VPNSettings.dnsProviderFlags = privacyOverwriteLoader.dnsProviderValue;
                        privacyOverwritePopup.close()
                    }
                },
                VPNLinkButton {
                    objectName: "privacyOverwritePopupGoBackButton"
                    labelText: VPNl18n.DnsOverwriteDialogSecondaryButton
                    onClicked: privacyOverwritePopup.close()
                }
            ]

            onClosed: {
                privacyOverwriteLoader.active = false
            }
        }

        onActiveChanged: if (active) { item.open() }
    }
}

