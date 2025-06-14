/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import Mozilla.Shared 1.0
import components 0.1
import compat 0.1
import "qrc:/qt/qml/Mozilla/VPN/sharedViews"
import "qrc:/nebula/utils/MZAssetLookup.js" as MZAssetLookup

ViewFullScreen {
    id: root
    objectName: "resetVpnView"

    property string _menuTitle: MZI18n.ResetSettingsResetLabel
    property var _menuOnBackClicked:  () => {
                                          getHelpStackView.pop();
                                      }

    content: ColumnLayout {
        spacing : 0

        Image {
            id: icon

            Layout.alignment: Qt.AlignHCenter

            sourceSize.height: 80
            sourceSize.width: 80
            fillMode: Image.PreserveAspectFit
            source: MZAssetLookup.getImageSource("Reset")
        }

        MZHeadline {
            Layout.topMargin: 24
            Layout.fillWidth: true

            text: MZI18n.ResetSettingsTitle
        }

        MZInterLabel {
            Layout.topMargin: 8
            Layout.fillWidth: true

            text: MZI18n.ResetSettingsBody1
            horizontalAlignment: Text.AlignLeft
        }

        MZInterLabel {
            Layout.topMargin: 16
            Layout.fillWidth: true

            text: "<ul style='margin-left: -20px;'><li>%1</li></ul>".arg(MZI18n.HelpSheetsPrivacyTitle)
            textFormat: Text.RichText
            horizontalAlignment: Text.AlignLeft

            Accessible.name: MZI18n.HelpSheetsPrivacyTitle
        }

        MZInterLabel {
            Layout.topMargin: 12
            Layout.fillWidth: true

            text: "<ul style='margin-left: -20px;'><li>%1</li></ul>".arg(MZI18n.SettingsAppExclusionTitle)
            textFormat: Text.RichText
            horizontalAlignment: Text.AlignLeft
            visible: MZFeatureList.get("splitTunnel").isSupported

            Accessible.name: MZI18n.SettingsAppExclusionTitle
        }

        MZInterLabel {
            Layout.topMargin: 12
            Layout.fillWidth: true

            text: "<ul style='margin-left: -20px;'><li>%1</li></ul>".arg(MZI18n.MultiHopFeatureMultiHopConnectionsHeader)
            textFormat: Text.RichText
            horizontalAlignment: Text.AlignLeft

            Accessible.name: MZI18n.MultiHopFeatureMultiHopConnectionsHeader
        }

        MZInterLabel {
            Layout.topMargin: 12
            Layout.fillWidth: true

            text: "<ul style='margin-left: -20px;'><li>%1</li></ul>".arg(MZI18n.ResetSettingsListItemPreferences)
            textFormat: Text.RichText
            horizontalAlignment: Text.AlignLeft

            Accessible.name: MZI18n.ResetSettingsListItemPreferences
        }

        MZInterLabel {
            Layout.topMargin: 16
            Layout.fillWidth: true

            text: MZI18n.ResetSettingsBody2
            horizontalAlignment: Text.AlignLeft
        }
    }


    buttons: ColumnLayout {
        spacing: 0

        MZButton {
            objectName: "resetVpnButton"

            Layout.fillWidth: true

            buttonType: MZButton.ButtonType.destructive
            text: MZI18n.ResetSettingsResetButtonLabel

            onClicked: confirmResetPopupLoader.active = true
        }

        MZLinkButton {
            objectName: "goBackButton"

            Layout.topMargin: 16
            Layout.fillWidth: true

            implicitHeight: MZTheme.theme.rowHeight
            labelText: MZI18n.GlobalGoBack

            onClicked: {
                getHelpStackView.pop();
            }
        }
    }

    Loader {
        id: confirmResetPopupLoader
        objectName: "confirmResetPopupLoader"

        active: false
        sourceComponent: MZSimplePopup {
            id: confirmResetPopup

            anchors.centerIn: Overlay.overlay

            closeButtonObjectName: "confirmResetPopupCloseButton"
            imageSrc: MZAssetLookup.getImageSource("ResetWithWarning")
            imageSize: Qt.size(80, 80)
            title: MZI18n.ResetSettingsConfirmResetModalTitle
            description: MZI18n.ResetSettingsConfirmResetModalBody

            buttons: [
                MZButton {
                    objectName: "confirmResetButton"

                    Layout.fillWidth: true

                    text: MZI18n.ResetSettingsConfirmResetModalResetButtonLabel
                    buttonType: MZButton.ButtonType.destructive

                    onClicked: {
                        VPN.hardResetAndQuit()
                    }
                },
                MZLinkButton {
                    objectName: "cancelButton"

                    Layout.alignment: Qt.AlignHCenter

                    labelText: MZI18n.InAppSupportWorkflowSupportSecondaryActionText

                    onClicked: confirmResetPopup.close()
                }
            ]

            onClosed: {
                confirmResetPopupLoader.active = false
            }
        }

        onActiveChanged: if (active) { item.open() }
    }
}
