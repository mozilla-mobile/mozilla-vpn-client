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

    _menuTitle: VPNI18n.SettingsDnsSettings

    function maybeApplyChange(settingValue) {
        if (settingValue === VPNSettings.Gateway) {
            if (VPNSettings.dnsProviderFlags !== VPNSettings.Custom) {
                // We are not changing anything interesting. Let's keep the current value.
                return;
            }

            VPNSettings.dnsProviderFlags = settingValue;
            return;
        }

        if (VPNSettings.dnsProviderFlags === VPNSettings.Custom ||
            VPNSettings.dnsProviderFlags === VPNSettings.Gateway) {
            VPNSettings.dnsProviderFlags = settingValue;
            return;
        }

        dnsOverwriteLoader.active = true;
    }

    _viewContentData: ColumnLayout {
        spacing: VPNTheme.theme.windowMargin * 1.5
        Layout.leftMargin: VPNTheme.theme.windowMargin
        Layout.rightMargin: VPNTheme.theme.windowMargin

        Loader {
            objectName: "DNSSettingsInformationCardLoader"
            active: !VPNController.silentServerSwitchingSupported && VPNController.state !== VPNController.StateOff
            Layout.alignment: Qt.AlignHCenter
            sourceComponent: InformationCard {
                objectName: "DNSSettingsViewInformationCard"
                height: textBlocks.height + VPNTheme.theme.windowMargin * 2
                width: Math.min(window.width - VPNTheme.theme.windowMargin * 2, VPNTheme.theme.navBarMaxWidth)
                _infoContent: ColumnLayout {
                    id: textBlocks

                    spacing: 0
                    VPNTextBlock {
                        Layout.fillWidth: true
                        text: VPNI18n.SettingsDnsSettingsDisconnectWarning
                        verticalAlignment: Text.AlignVCenter
                        Accessible.role: Accessible.StaticText
                        Accessible.name: text
                    }
                }
            }
        }

        ButtonGroup {
            id: radioButtonGroup
        }

        RowLayout {
            spacing: VPNTheme.theme.windowMargin
            Layout.rightMargin: VPNTheme.theme.windowMargin / 2

            VPNRadioButton {
                objectName: "dnsStandard"

                id: gatewayRadioButton
                Layout.preferredWidth: VPNTheme.theme.vSpacing
                Layout.preferredHeight: VPNTheme.theme.rowHeight
                Layout.alignment: Qt.AlignTop
                checked: VPNSettings.dnsProviderFlags !== VPNSettings.Custom
                ButtonGroup.group: radioButtonGroup
                accessibleName: VPNI18n.SettingsDnsSettingsStandardDNSTitle
                onClicked: maybeApplyChange(VPNSettings.Gateway);
            }

            ColumnLayout {
                spacing: 4

                VPNInterLabel {
                    Layout.fillWidth: true

                    text: VPNI18n.SettingsDnsSettingsStandardDNSTitle
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
                    text: VPNI18n.SettingsDnsSettingsStandardDNSBody
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
                accessibleName: VPNI18n.SettingsDnsSettingsCustomDNSTitle
                onClicked: maybeApplyChange(VPNSettings.Custom);
            }

            ColumnLayout {
                spacing: 4
                Layout.fillWidth: true

                VPNInterLabel {
                    Layout.fillWidth: true

                    text: VPNI18n.SettingsDnsSettingsCustomDNSTitle
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
                    text: VPNI18n.SettingsDnsSettingsCustomDNSBody
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

                    _placeholderText: VPNI18n.SettingsDnsSettingsInputPlaceholder
                    text: ""
                    Layout.fillWidth: true
                    Layout.maximumWidth: VPNTheme.theme.maxTextWidth
                    Layout.topMargin: 12
                    inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhFormattedNumbersOnly

                    PropertyAnimation on opacity {
                        duration: 200
                    }

                    Component.onCompleted: {
                        ipInput.text = VPNSettings.userDNS;
                    }

                    onEditingFinished: {
                        if (ipInput.text === "" || VPN.validateUserDNS(ipInput.text)) {
                            VPNSettings.userDNS = ipInput.text
                        }
                    }

                    onTextChanged: {
                        ipInput.valueInvalid = ipInput.text !== "" && !VPN.validateUserDNS(ipInput.text);
                        ipInput.error = VPNI18n.SettingsDnsSettingsCustomDNSError
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

                    visible: ipInput.valueInvalid && ipInput.visible && VPNSettings.dnsProviderFlags === VPNSettings.Custom

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

    onVisibleChanged: {
      if (!visible) {
        if (VPNSettings.userDNS === "" && VPNSettings.dnsProviderFlags === VPNSettings.Custom) {
          VPNSettings.dnsProviderFlags = VPNSettings.Gateway;
        }
      }
    }

    Loader {
        id: dnsOverwriteLoader

        active: false
        sourceComponent: VPNSimplePopup {
            id: dnsOverwritePopup

            anchors.centerIn: Overlay.overlay
            closeButtonObjectName: "dnsOverwritePopupPopupCloseButton"
            imageSrc: "qrc:/ui/resources/logo-dns-privacy.svg"
            imageSize: Qt.size(80, 80)
            title: VPNI18n.DnsOverwriteDialogTitlePrivacy
            description: VPNI18n.DnsOverwriteDialogBodyPrivacy
            buttons: [
                VPNButton {
                    objectName: "dnsOverwritePopupDiscoverNowButton"
                    text: VPNI18n.DnsOverwriteDialogPrimaryButton
                    Layout.fillWidth: true
                    onClicked: {
                        VPNSettings.dnsProviderFlags = VPNSettings.Custom;
                        dnsOverwritePopup.close()
                    }
                },
                VPNLinkButton {
                    objectName: "dnsOverwritePopupGoBackButton"
                    labelText: VPNI18n.DnsOverwriteDialogSecondaryButton
                    onClicked: dnsOverwritePopup.close()
                    Layout.alignment: Qt.AlignHCenter
                }
            ]

            onClosed: {
                dnsOverwriteLoader.active = false
            }
        }

        onActiveChanged: if (active) { item.open() }
    }
}

