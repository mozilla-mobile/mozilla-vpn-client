/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1
import compat 0.1

MZViewBase {
    id: root
    objectName: "privacySettingsView"

    property Component rightMenuButton: Component {
        Loader {
            active: MZFeatureList.get("helpSheets").isSupported
            sourceComponent: MZIconButton {
                onClicked: helpSheetLoader.active = true

                accessibleName: MZI18n.GlobalHelp
                Image {
                    anchors.centerIn: parent

                    source: "qrc:/nebula/resources/question.svg"
                    fillMode: Image.PreserveAspectFit
                }
            }
        }
    }

    _menuTitle: MZI18n.SettingsPrivacySettings

    _viewContentData: ColumnLayout {
        spacing: MZTheme.theme.windowMargin * 1.5
        Layout.preferredWidth: parent.width

        MZInformationCard {
            objectName: "privacySettingsViewInformationCard"
            Layout.preferredWidth: Math.min(window.width - MZTheme.theme.windowMargin * 2, MZTheme.theme.navBarMaxWidth)
            Layout.minimumHeight: textBlocks.height + MZTheme.theme.windowMargin * 2
            Layout.alignment: Qt.AlignHCenter

            _infoContent: ColumnLayout {
                id: textBlocks
                spacing: 0

                MZTextBlock {
                    Layout.fillWidth: true
                    width: undefined
                    text: MZI18n.SettingsDnsSettingsWarning
                    verticalAlignment: Text.AlignVCenter
                }
                Loader {
                    active: !VPNController.silentServerSwitchingSupported && VPNController.state !== VPNController.StateOff
                    Layout.fillWidth: true
                    visible: active
                    sourceComponent: MZTextBlock {
                        width: parent.width
                        text: MZI18n.SettingsDnsSettingsDisconnectWarning
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }

        PrivacyFeaturesList {
            id: privacyFeatureList
            Layout.leftMargin: 16
            Layout.rightMargin: 16
            Layout.fillWidth: true

            onSettingClicked: (dnsProviderFlags, active) => {
                privacyOverwriteLoader.dnsProviderValue = dnsProviderFlags;
                privacyOverwriteLoader.active = active;
            }
        }
    }

    Loader {
        id: privacyOverwriteLoader

        // This is the value we are going to set if the user confirms.
        property var dnsProviderValue;

        objectName: "privacyOverwriteLoader"
        active: false
        sourceComponent: MZSimplePopup {
            id: privacyOverwritePopup

            anchors.centerIn: Overlay.overlay
            closeButtonObjectName: "privacyOverwritePopupPopupCloseButton"
            imageSrc: "qrc:/ui/resources/logo-dns-settings.svg"
            imageSize: Qt.size(80, 80)
            title: MZI18n.DnsOverwriteDialogTitleDNS
            description: MZI18n.DnsOverwriteDialogBodyDNS
            buttons: [
                MZButton {
                    objectName: "privacyOverwritePopupDiscoverNowButton"
                    text: MZI18n.DnsOverwriteDialogPrimaryButton
                    Layout.fillWidth: true
                    onClicked: {
                        MZSettings.dnsProviderFlags = privacyOverwriteLoader.dnsProviderValue;
                        privacyOverwritePopup.close()
                    }
                },
                MZLinkButton {
                    objectName: "privacyOverwritePopupGoBackButton"
                    labelText: MZI18n.DnsOverwriteDialogSecondaryButton
                    Layout.alignment: Qt.AlignHCenter
                    onClicked: privacyOverwritePopup.close()
                }
            ]

            onClosed: {
                privacyOverwriteLoader.active = false
            }
        }

        onActiveChanged: if (active) { item.open() }
    }

    Loader {
        id: helpSheetLoader

        active: false

        onActiveChanged: if (active) item.open()

        sourceComponent: MZHelpSheet {
            title: MZI18n.HelpSheetsPrivacyTitle

            model: [
                {type: MZHelpSheet.BlockType.Title, text: MZI18n.HelpSheetsPrivacyHeader},
                {type: MZHelpSheet.BlockType.Text, text: MZI18n.HelpSheetsPrivacyBody1, margin: MZTheme.theme.helpSheetTitleBodySpacing},
                {type: MZHelpSheet.BlockType.Text, text: MZI18n.HelpSheetsPrivacyBody2, margin: MZTheme.theme.helpSheetBodySpacing},
                {type: MZHelpSheet.BlockType.LinkButton, text: MZI18n.GlobalLearnMore, margin: MZTheme.theme.helpSheetBodyButtonSpacing, action: () => { MZUrlOpener.openUrlLabel("sumoPrivacy") }},
            ]

            onClosed: helpSheetLoader.active = false
        }
    }
}

