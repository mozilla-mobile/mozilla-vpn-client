/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1

VPNViewBase {
    id: vpnFlickable
    objectName: "DNSSettingsView"

    _menuTitle: VPNl18n.SettingsDnsSettings

    function maybeApplyChange(settingValue) {
        // We are not changing anything interesting for the privacy/dns dialog.
        if (VPNSettings.dnsProviderFlags === VPNSettings.Custom ||
            VPNSettings.dnsProviderFlags === VPNSettings.Gateway) {
            VPNSettings.dnsProviderFlags = settingValue;
            return;
        }

        dnsOverwriteLoader.dnsProviderValue = settingValue;
        dnsOverwriteLoader.active = true;
    }

    _viewContentData: ColumnLayout {
        spacing: VPNTheme.theme.windowMargin * 2
        Layout.fillWidth: true
        Layout.leftMargin: VPNTheme.theme.windowMargin
        Layout.rightMargin: VPNTheme.theme.windowMargin
        Layout.topMargin: VPNTheme.theme.windowMargin / 2

        SettingsDisconnectWarning {
            objectName: "viewDnsIOSWarning"
            Layout.topMargin: VPNTheme.theme.windowMargin
            Layout.leftMargin: VPNTheme.theme.windowMargin
            Layout.rightMargin: VPNTheme.theme.windowMargin
            Layout.fillWidth: true
            Layout.minimumHeight: textBlocks.implicitHeight
            visible: Qt.platform.os === "ios" && VPNController.state !== VPNController.StateOff

            _infoContent: ColumnLayout {
                id: textBlocks
                VPNTextBlock {
                    Layout.fillWidth: true
                    width: undefined
                    text: VPNl18n.SettingsIOSDisconnectWarning
                }
            }
        }

        ButtonGroup {
            id: radioButtonGroup
        }

        RowLayout {
            spacing: VPNTheme.theme.windowMargin

            VPNRadioButton {
                objectName: "dnsStandard"

                id: gatewayRadioButton
                Layout.preferredWidth: VPNTheme.theme.vSpacing
                Layout.preferredHeight: VPNTheme.theme.rowHeight
                Layout.alignment: Qt.AlignTop
                checked: VPNSettings.dnsProviderFlags === VPNSettings.Gateway
                ButtonGroup.group: radioButtonGroup
                accessibleName: VPNl18n.SettingsDnsSettingsStandardDNSTitle
                onClicked: maybeApplyChange(VPNSettings.Gateway);
            }

            ColumnLayout {
                spacing: 4

                VPNInterLabel {
                    Layout.fillWidth: true

                    text: VPNl18n.SettingsDnsSettingsStandardDNSTitle
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignLeft

                    VPNMouseArea {
                        anchors.fill: parent

                        enabled: gatewayRadioButton.enabled
                        width: Math.min(parent.implicitWidth, parent.width)
                        propagateClickToParent: false
                        onClicked: maybeApplyChange(VPNSettings.Gateway);
                    }
                }

                VPNTextBlock {
                    text: VPNl18n.SettingsDnsSettingsStandardDNSBody
                    Layout.fillWidth: true
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: VPNTheme.theme.windowMargin

            VPNRadioButton {
                objectName: "dnsCustom"

                id: customRadioButton
                Layout.preferredWidth: VPNTheme.theme.vSpacing
                Layout.preferredHeight: VPNTheme.theme.rowHeight
                Layout.alignment: Qt.AlignTop
                checked: VPNSettings.dnsProviderFlags === VPNSettings.Custom
                ButtonGroup.group: radioButtonGroup
                accessibleName: VPNl18n.SettingsDnsSettingsCustomDNSTitle
                onClicked: maybeApplyChange(VPNSettings.Custom);
            }

            ColumnLayout {
                spacing: 4
                Layout.fillWidth: true

                VPNInterLabel {
                    Layout.fillWidth: true

                    text: VPNl18n.SettingsDnsSettingsCustomDNSTitle
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignLeft

                    VPNMouseArea {
                        anchors.fill: parent

                        enabled: customRadioButton.enabled
                        width: Math.min(parent.implicitWidth, parent.width)
                        propagateClickToParent: false
                        onClicked: maybeApplyChange(VPNSettings.Custom);
                    }
                }
                VPNTextBlock {
                    text: VPNl18n.SettingsDnsSettingsCustomDNSBody
                    Layout.fillWidth: true
                }

                VPNTextField {
                    id: ipInput
                    objectName: "dnsCustomInput"

                    property bool valueInvalid: false
                    property string error: "This is an error string"

                    hasError: valueInvalid
                    enabled: VPNSettings.dnsProviderFlags === VPNSettings.Custom
                    onEnabledChanged: if(enabled) forceActiveFocus()

                    _placeholderText: VPNl18n.SettingsDnsSettingsInputPlaceholder
                    text: ""
                    Layout.fillWidth: true
                    Layout.topMargin: 12
                    inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhFormattedNumbersOnly

                    PropertyAnimation on opacity {
                        duration: 200
                    }

                    Component.onCompleted: {
                        ipInput.text = VPNSettings.userDNS;
                    }

                    onTextChanged: text => {
                        if (ipInput.text === "") {
                            // If nothing is entered, thats valid too. We will ignore the value later.
                            ipInput.valueInvalid = false;
                            VPNSettings.userDNS = ipInput.text
                            return;
                        }
                        if (VPN.validateUserDNS(ipInput.text)) {
                            ipInput.valueInvalid = false;
                            VPNSettings.userDNS = ipInput.text
                        } else {
                            ipInput.error = VPNl18n.SettingsDnsSettingsCustomDNSError
                            ipInput.valueInvalid = true;
                        }
                    }
                }

                VPNContextualAlerts {
                    id: errorAlert

                    property string messageText: ""
                    property bool messageVisible: false

                    anchors {
                        left: undefined
                        right: undefined
                    }
                    Layout.topMargin: VPNTheme.theme.listSpacing

                    visible: ipInput.valueInvalid && ipInput.visible

                    messages: [
                        {
                            type: "error",
                            message: ipInput.error,
                            visible: ipInput.valueInvalid && ipInput.visible
                        }
                    ]
                }
            }
        }
    }

    Loader {
        id: dnsOverwriteLoader

        // This is the value we are going to set if the user confirms.
        property var dnsProviderValue;

        active: false
        sourceComponent: VPNSimplePopup {
            id: dnsOverwritePopup

            anchors.centerIn: Overlay.overlay
            closeButtonObjectName: "dnsOverwritePopupPopupCloseButton"
            imageSrc: "qrc:/ui/resources/logo-dns-privacy.svg"
            imageSize: Qt.size(80, 80)
            title: VPNl18n.DnsOverwriteDialogTitlePrivacy
            description: VPNl18n.DnsOverwriteDialogBodyPrivacy
            buttons: [
                VPNButton {
                    objectName: "dnsOverwritePopupDiscoverNowButton"
                    text: VPNl18n.DnsOverwriteDialogPrimaryButton
                    onClicked: {
                        VPNSettings.dnsProviderFlags = dnsOverwriteLoader.dnsProviderValue;
                        dnsOverwritePopup.close()
                    }
                },
                VPNLinkButton {
                    objectName: "dnsOverwritePopupGoBackButton"
                    labelText: VPNl18n.DnsOverwriteDialogSecondaryButton
                    onClicked: dnsOverwritePopup.close()
                }
            ]

            onClosed: {
                dnsOverwriteLoader.active = false
            }
        }

        onActiveChanged: if (active) { item.open() }
    }
}

