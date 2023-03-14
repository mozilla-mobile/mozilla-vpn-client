/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1

MZViewBase {
    id: vpnFlickable
    objectName: "DNSSettingsView"

    _menuTitle: MZI18n.SettingsDnsSettings

    function maybeApplyChange(settingValue) {
        if (settingValue === MZSettings.Gateway) {
            if (MZSettings.dnsProviderFlags !== MZSettings.Custom) {
                // We are not changing anything interesting. Let's keep the current value.
                return;
            }

            MZSettings.dnsProviderFlags = settingValue;
            return;
        }

        if (MZSettings.dnsProviderFlags === MZSettings.Custom ||
            MZSettings.dnsProviderFlags === MZSettings.Gateway) {
            MZSettings.dnsProviderFlags = settingValue;
            return;
        }

        dnsOverwriteLoader.active = true;
    }

    _viewContentData: ColumnLayout {
        spacing: MZTheme.theme.windowMargin * 1.5
        Layout.leftMargin: MZTheme.theme.windowMargin
        Layout.rightMargin: MZTheme.theme.windowMargin

        Loader {
            objectName: "DNSSettingsInformationCardLoader"
            active: !VPNController.silentServerSwitchingSupported && VPNController.state !== VPNController.StateOff
            visible: active
            Layout.alignment: Qt.AlignHCenter
            sourceComponent: InformationCard {
                objectName: "DNSSettingsViewInformationCard"
                height: textBlocks.height + MZTheme.theme.windowMargin * 2
                width: Math.min(window.width - MZTheme.theme.windowMargin * 2, MZTheme.theme.navBarMaxWidth)
                _infoContent: ColumnLayout {
                    id: textBlocks

                    spacing: 0
                    MZTextBlock {
                        Layout.fillWidth: true
                        text: MZI18n.SettingsDnsSettingsDisconnectWarning
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
            spacing: MZTheme.theme.windowMargin
            Layout.rightMargin: MZTheme.theme.windowMargin / 2

            MZRadioButton {
                objectName: "dnsStandard"

                id: gatewayRadioButton
                Layout.preferredWidth: MZTheme.theme.vSpacing
                Layout.preferredHeight: MZTheme.theme.rowHeight
                Layout.alignment: Qt.AlignTop
                checked: MZSettings.dnsProviderFlags !== MZSettings.Custom
                ButtonGroup.group: radioButtonGroup
                accessibleName: MZI18n.SettingsDnsSettingsStandardDNSTitle
                onClicked: maybeApplyChange(MZSettings.Gateway);
            }

            ColumnLayout {
                spacing: 4

                MZInterLabel {
                    Layout.fillWidth: true

                    text: MZI18n.SettingsDnsSettingsStandardDNSTitle
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignLeft

                    MZMouseArea {
                        anchors.fill: parent

                        enabled: gatewayRadioButton.enabled
                        width: Math.min(parent.implicitWidth, parent.width)
                        propagateClickToParent: false
                        onClicked: maybeApplyChange(MZSettings.Gateway);
                    }
                }

                MZTextBlock {
                    text: MZI18n.SettingsDnsSettingsStandardDNSBody
                    Layout.fillWidth: true
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: MZTheme.theme.windowMargin

            MZRadioButton {
                objectName: "dnsCustom"

                id: customRadioButton
                Layout.preferredWidth: MZTheme.theme.vSpacing
                Layout.preferredHeight: MZTheme.theme.rowHeight
                Layout.alignment: Qt.AlignTop
                checked: MZSettings.dnsProviderFlags === MZSettings.Custom
                ButtonGroup.group: radioButtonGroup
                accessibleName: MZI18n.SettingsDnsSettingsCustomDNSTitle
                onClicked: maybeApplyChange(MZSettings.Custom);
            }

            ColumnLayout {
                spacing: 4
                Layout.fillWidth: true

                MZInterLabel {
                    Layout.fillWidth: true

                    text: MZI18n.SettingsDnsSettingsCustomDNSTitle
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignLeft

                    MZMouseArea {
                        anchors.fill: parent

                        enabled: customRadioButton.enabled
                        width: Math.min(parent.implicitWidth, parent.width)
                        propagateClickToParent: false
                        onClicked: maybeApplyChange(MZSettings.Custom);
                    }
                }
                MZTextBlock {
                    text: MZI18n.SettingsDnsSettingsCustomDNSBody
                    Layout.fillWidth: true
                }

                MZTextField {
                    id: ipInput
                    objectName: "dnsCustomInput"

                    property bool valueInvalid: false
                    property string error: "This is an error string"

                    hasError: valueInvalid
                    enabled: MZSettings.dnsProviderFlags === MZSettings.Custom
                    onEnabledChanged: if(enabled) forceActiveFocus()

                    _placeholderText: MZI18n.SettingsDnsSettingsInputPlaceholder
                    text: ""
                    Layout.fillWidth: true
                    Layout.maximumWidth: MZTheme.theme.maxTextWidth
                    Layout.topMargin: 12
                    inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhFormattedNumbersOnly

                    PropertyAnimation on opacity {
                        duration: 200
                    }

                    Component.onCompleted: {
                        ipInput.text = MZSettings.userDNS;
                    }

                    onEditingFinished: {
                        if (ipInput.text === "" || VPN.validateUserDNS(ipInput.text)) {
                            MZSettings.userDNS = ipInput.text
                        }
                    }

                    onTextChanged: {
                        ipInput.valueInvalid = ipInput.text !== "" && !VPN.validateUserDNS(ipInput.text);
                        ipInput.error = MZI18n.SettingsDnsSettingsCustomDNSError
                    }
                }

                MZContextualAlerts {
                    id: errorAlert

                    property string messageText: ""
                    property bool messageVisible: false

                    anchors {
                        left: undefined
                        right: undefined
                    }
                    Layout.topMargin: MZTheme.theme.listSpacing

                    visible: ipInput.valueInvalid && ipInput.visible && MZSettings.dnsProviderFlags === MZSettings.Custom

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
        if ((MZSettings.userDNS === "" || !VPN.validateUserDNS(ipInput.text)) &&
            MZSettings.dnsProviderFlags === MZSettings.Custom) {
          MZSettings.dnsProviderFlags = MZSettings.Gateway;
        }
      }
    }

    Loader {
        id: dnsOverwriteLoader

        active: false
        sourceComponent: MZSimplePopup {
            id: dnsOverwritePopup

            anchors.centerIn: Overlay.overlay
            closeButtonObjectName: "dnsOverwritePopupPopupCloseButton"
            imageSrc: "qrc:/ui/resources/logo-dns-privacy.svg"
            imageSize: Qt.size(80, 80)
            title: MZI18n.DnsOverwriteDialogTitlePrivacy
            description: MZI18n.DnsOverwriteDialogBodyPrivacy
            buttons: [
                MZButton {
                    objectName: "dnsOverwritePopupDiscoverNowButton"
                    text: MZI18n.DnsOverwriteDialogPrimaryButton
                    Layout.fillWidth: true
                    onClicked: {
                        MZSettings.dnsProviderFlags = MZSettings.Custom;
                        dnsOverwritePopup.close()
                    }
                },
                MZLinkButton {
                    objectName: "dnsOverwritePopupGoBackButton"
                    labelText: MZI18n.DnsOverwriteDialogSecondaryButton
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

