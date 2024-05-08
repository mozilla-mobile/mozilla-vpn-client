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
    id: root
    objectName: "DNSSettingsView"

    _menuTitle: MZI18n.SettingsDnsSettings

    readonly property string telemetryScreenId : "dns_settings"
    property bool customDNS: false
    property bool privacyDialogNeeded: true
    property bool dnsSelectionChanged: false
    property Component rightMenuButton: Component {
        Loader {
            active: true
            sourceComponent: MZIconButton {
                objectName: "dnsHelpButton"

                onClicked: {
                    helpSheet.open()

                    Glean.interaction.helpTooltipSelected.record({
                        screen: root.telemetryScreenId,
                    });
                }

                accessibleName: MZI18n.GetHelpLinkText

                Image {
                    anchors.centerIn: parent

                    source: "qrc:/nebula/resources/question.svg"
                    fillMode: Image.PreserveAspectFit
                }
            }
        }
    }


    function applyFrontendChanges(settingValue) {
        if (settingValue === MZSettings.Gateway) {
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
        ipInput.forceActiveFocus();
    }

    function saveChange(dnsProviderFlags, userDNS = undefined) {
        if (MZSettings.dnsProviderFlags !== dnsProviderFlags) {
            MZSettings.dnsProviderFlags = dnsProviderFlags;
        }

        if (userDNS !== undefined && MZSettings.userDNS != userDNS) {
            MZSettings.userDNS = userDNS;
        }
    }

    function maybeSaveChange() {
        if (!root.dnsSelectionChanged && ipInput.text === MZSettings.userDNS) {
            return;
        }

        root.dnsSelectionChanged = false;

        if (!root.customDNS) {
            saveChange(MZSettings.Gateway);
            return;
        }

        if (ipInput.text !== "" && VPN.validateUserDNS(ipInput.text)) {
            saveChange(MZSettings.Custom, ipInput.text);
            return;
        }

        if (MZSettings.userDNS === "" || !VPN.validateUserDNS(MZSettings.userDNS)) {
            saveChange(MZSettings.Gateway);
        }
    }

    function reset() {
        root.customDNS = MZSettings.dnsProviderFlags === MZSettings.Custom;
        root.privacyDialogNeeded = MZSettings.dnsProviderFlags !== MZSettings.Custom &&
                MZSettings.dnsProviderFlags !== MZSettings.Gateway;
        ipInput.text = MZSettings.userDNS;
    }


    _viewContentData: ColumnLayout {
        spacing: MZTheme.theme.windowMargin * 1.5
        Layout.leftMargin: MZTheme.theme.windowMargin
        Layout.rightMargin: MZTheme.theme.windowMargin
        Layout.preferredWidth: parent.width

        Loader {
            objectName: "DNSSettingsInformationCardLoader"
            active: !VPNController.silentServerSwitchingSupported && VPNController.state !== VPNController.StateOff
            visible: active
            Layout.alignment: Qt.AlignHCenter
            sourceComponent: MZInformationCard {
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
                checked: !root.customDNS
                ButtonGroup.group: radioButtonGroup
                accessibleName: MZI18n.SettingsDnsSettingsStandardDNSTitle
                onClicked: applyFrontendChanges(MZSettings.Gateway);
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
                        onClicked: applyFrontendChanges(MZSettings.Gateway);
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
                checked: root.customDNS
                ButtonGroup.group: radioButtonGroup
                accessibleName: MZI18n.SettingsDnsSettingsCustomDNSTitle
                onClicked: applyFrontendChanges(MZSettings.Custom);
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
                        onClicked: applyFrontendChanges(MZSettings.Custom);
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
                    enabled: root.customDNS

                    _placeholderText: MZI18n.SettingsDnsSettingsInputPlaceholder
                    text: ""
                    Layout.fillWidth: true
                    Layout.maximumWidth: MZTheme.theme.maxTextWidth
                    Layout.topMargin: 12
                    inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhFormattedNumbersOnly

                    PropertyAnimation on opacity {
                        duration: 200
                    }

                    onTextChanged: {
                        ipInput.valueInvalid = ipInput.text !== "" && !VPN.validateUserDNS(ipInput.text);
                        ipInput.error = MZI18n.SettingsDnsSettingsCustomDNSError
                    }

                    onActiveFocusChanged: {
                        if (!activeFocus && !ipInput.focusReasonA11y) {
                            maybeSaveChange();
                        }
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
        Glean.impression.dnsSettingsScreen.record({
            screen: telemetryScreenId,
        });
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
        objectName: "dnsOverwriteLoader"
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
                        root.privacyDialogNeeded = false;
                        applyFrontendChanges(MZSettings.Custom);
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

    MZHelpSheet {
        id: helpSheet
        objectName: "dnsHelpSheet"

        property string telemetryScreenId: "dns_settings_info"

        title: MZI18n.HelpSheetsDnsTitle

        model: [
            {type: MZHelpSheet.BlockType.Title, text: MZI18n.HelpSheetsDnsHeader},
            {type: MZHelpSheet.BlockType.Text, text: MZI18n.HelpSheetsDnsBody1, margin: MZTheme.theme.helpSheetTitleBodySpacing},
            {type: MZHelpSheet.BlockType.Text, text: MZI18n.HelpSheetsDnsBody2, margin: MZTheme.theme.helpSheetBodySpacing},
            {type: MZHelpSheet.BlockType.LinkButton, text: MZI18n.GlobalLearnMore, margin: MZTheme.theme.helpSheetBodyButtonSpacing, objectName: "learnMoreLink", action: () => {
                    MZUrlOpener.openUrlLabel("sumoDns")
                    Glean.interaction.learnMoreSelected.record({
                        screen: telemetryScreenId
                    });
                }}
        ]

        onOpened: {
            Glean.impression.dnsSettingsInfoScreen.record({
                screen: telemetryScreenId,
            });
        }
    }
}
