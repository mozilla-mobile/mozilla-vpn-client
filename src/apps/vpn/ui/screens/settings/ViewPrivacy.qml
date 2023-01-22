/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1

import org.mozilla.Glean 0.30
import telemetry 0.30


VPNViewBase {
    id: root
    objectName: "privacySettingsView"

    _menuTitle: VPNl18n.SettingsPrivacySettings
    _interactive: false

    function maybeApplyChange(settingValue) {
        // We are not changing anything interesting for the privacy/dns dialog.
        if (VPNSettings.dnsProvider !== VPNSettings.Custom &&
            VPNSettings.dnProvider !== VPNSettings.Gateway) {
            VPNSettings.dnsProvider = settingValue;
            return;
        }

        privacyOverwriteLoader.dnsProviderValue = settingValue;
        privacyOverwriteLoader.active = true;
    }

    _viewContentData: Column {
        id: tabs
        Layout.topMargin: -VPNTheme.theme.windowMargin
        Layout.preferredHeight: root.height - VPNTheme.theme.menuHeight
        Layout.preferredWidth: root.width

        Repeater {
            id: repeater

            delegate: RowLayout {
                Layout.fillWidth: true
                spacing: VPNTheme.theme.windowMargin
                Layout.rightMargin: VPNTheme.theme.windowMargin

                VPNRadioButton {
                    id: radioButton
                    Layout.preferredWidth: VPNTheme.theme.vSpacing
                    Layout.preferredHeight: VPNTheme.theme.rowHeight
                    Layout.alignment: Qt.AlignTop
                    checked: VPNSettings.dnsProvider == modelData.settingValue
                    accessibleName: modelData.settingTitle
                    onClicked: maybeApplyChange(modelData.settingValue);
                }

                ColumnLayout {
                    spacing: 4
                    Layout.fillWidth: true

                    VPNInterLabel {
                        Layout.fillWidth: true

                        text: modelData.settingTitle
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignLeft

                        VPNMouseArea {
                            anchors.fill: undefined
                            anchors.top: parent.top
                            anchors.left: parent.left
                            anchors.bottom: parent.bottom

                            enabled: radioButton.enabled
                            width: Math.min(parent.implicitWidth, parent.width)
                            propagateClickToParent: false
                            onClicked: maybeApplyChange(modelData.settingValue);
                        }
                    }

                    VPNTextBlock {
                        text: modelData.settingDescription

                        Layout.fillWidth: true
                    }
                }
            }
        }

        Component.onCompleted: {
                repeater.model = [{
                                     settingValue: VPNSettings.BlockAds,
                                     settingTitle: VPNl18n.CustomDNSSettingsDnsAdblockRadioHeader,
                                     settingDescription: VPNl18n.CustomDNSSettingsDnsAdblockRadioBody,
                                    },
         {   settingValue: VPNSettings.BlockTracking,
                                             settingTitle: VPNl18n.CustomDNSSettingsDnsAntitrackRadioHeader,
                                             settingDescription: VPNl18n.CustomDNSSettingsDnsAntitrackRadioBody,
                                             },
          {   settingValue: VPNSettings.BlockAll,
                                             settingTitle: VPNl18n.CustomDNSSettingsDnsAdblockAntiTrackRadioHeader,
                                             settingDescription: VPNl18n.CustomDNSSettingsDnsAdblockAntiTrackRadioBody,
                                             }];
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
                    id: tipAndTricksIntroButton
                    objectName: "privacyOverwritePopupDiscoverNowButton"
                    text: VPNl18n.DnsOverwriteDialogPrimaryButton
                    onClicked: {
                        VPNSettings.dnsProvider = privacyOverwriteLoader.dnsProviderValue;
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

