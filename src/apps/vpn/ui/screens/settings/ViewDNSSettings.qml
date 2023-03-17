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
    id: root
    objectName: "DNSSettingsView"

    _menuTitle: VPNl18n.SettingsDnsSettings

    property bool customDNS: false
    property bool privacyDialogNeeded: true
    property bool dnsSelectionChanged: false

    function applyFrontendChanges(settingValue) {
        if (settingValue === VPNSettings.Gateway) {
            root.dnsSelectionChanged = true;
            root.customDNS = false;
            return;
        }

        if (root.privacyDialogNeeded) {
            dnsOverwriteLoader.active = true;
            return;
        }

        root.customDNS = true;
        root.dnsSelectionChanged = true;
    }

    function maybeSaveChange() {
        if (!root.dnsSelectionChanged && ipInput.text === VPNSettings.userDNS) {
            return;
        }

        root.dnsSelectionChanged = false;

        if (!root.customDNS) {
            VPNSettings.dnsProviderFlags = VPNSettings.Gateway;
            return;
        }

        if (ipInput.text !== "" && VPN.validateUserDNS(ipInput.text)) {
            VPNSettings.userDNS = ipInput.text
            VPNSettings.dnsProviderFlags = VPNSettings.Custom;
            return;
        }

        VPNSettings.dnsProviderFlags = VPNSettings.Gateway;
    }

    function reset() {
        root.customDNS = VPNSettings.dnsProviderFlags === VPNSettings.Custom;
        root.privacyDialogNeeded = VPNSettings.dnsProviderFlags !== VPNSettings.Custom &&
                                   VPNSettings.dnsProviderFlags !== VPNSettings.Gateway;
        ipInput.text = VPNSettings.userDNS;
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
                        text: VPNl18n.SettingsDnsSettingsDisconnectWarning
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
                checked: !root.customDNS
                ButtonGroup.group: radioButtonGroup
                accessibleName: VPNl18n.SettingsDnsSettingsStandardDNSTitle
                onClicked: applyFrontendChanges(VPNSettings.Gateway);
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
                        onClicked: applyFrontendChanges(VPNSettings.Gateway);
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
                checked: root.customDNS
                ButtonGroup.group: radioButtonGroup
                accessibleName: VPNl18n.SettingsDnsSettingsCustomDNSTitle
                onClicked: applyFrontendChanges(VPNSettings.Custom);
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
                        onClicked: applyFrontendChanges(VPNSettings.Custom);
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
                    enabled: root.customDNS
                    onEnabledChanged: if(enabled) forceActiveFocus()

                    _placeholderText: VPNl18n.SettingsDnsSettingsInputPlaceholder
                    text: ""
                    Layout.fillWidth: true
                    Layout.maximumWidth: VPNTheme.theme.maxTextWidth
                    Layout.topMargin: 12
                    inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhFormattedNumbersOnly

                    PropertyAnimation on opacity {
                        duration: 200
                    }


                    onTextChanged: {
                        ipInput.valueInvalid = ipInput.text !== "" && !VPN.validateUserDNS(ipInput.text);
                        ipInput.error = VPNl18n.SettingsDnsSettingsCustomDNSError
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

                    visible: ipInput.valueInvalid && ipInput.visible && root.customDNS

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

    Component.onCompleted: {
        reset();
    }

    Component.onDestruction: {
        maybeSaveChange();
    }

    onVisibleChanged: {
      if (!visible) {
          maybeSaveChange();
      } else {
          reset();
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
            title: VPNl18n.DnsOverwriteDialogTitlePrivacy
            description: VPNl18n.DnsOverwriteDialogBodyPrivacy
            buttons: [
                VPNButton {
                    objectName: "dnsOverwritePopupDiscoverNowButton"
                    text: VPNl18n.DnsOverwriteDialogPrimaryButton
                    onClicked: {
                        root.privacyDialogNeeded = false;
                        applyFrontendChanges(VPNSettings.Custom);
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

