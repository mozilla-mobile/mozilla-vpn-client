/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import "qrc:/nebula/utils/MZAssetLookup.js" as MZAssetLookup

MZViewBase {
    id: vpnFlickable
    objectName: "devicesSettingsView"

    property var isModalDialogOpened: removePopup.visible
    property var wasmView
    property string deviceCountLabelText: ""
    property bool isEditing: false
    property bool isDeletingLastItem
    property Component rightMenuButton: Component {
        MZLinkButton {
            id: editLink

            property bool isEditing: false
            property bool skipEnsureVisible: true

            enabled: VPNDeviceModel.activeDevices > 1
            labelText: (!vpnFlickable.isEditing || VPNDeviceModel.activeDevices < 2) ? MZI18n.InAppMessagingEditButton : MZI18n.InAppSupportWorkflowSupportResponseButton
            onClicked: {
                vpnFlickable.isEditing = !vpnFlickable.isEditing
            }
        }
    }

    onIsEditingChanged: deviceList.isEditingChanged(isEditing)

    _menuTitle: MZI18n.DevicesSectionTitle
    _viewContentData: ColumnLayout {
        id: content

        spacing: 0


        RowLayout {
            Layout.topMargin: -8 //Because we are in a MZViewBase with default 16px top margin
            Layout.leftMargin: MZTheme.theme.windowMargin * 1.5
            Layout.rightMargin: MZTheme.theme.windowMargin * 1.5

            spacing: 4

            MZInterLabel {
                Layout.maximumWidth: vpnFlickable.width - parent.Layout.leftMargin - parent.Layout.rightMargin - parent.spacing - helpIconButtonLoader.implicitWidth

                horizontalAlignment: Text.AlignLeft
                text: MZI18n.DevicesCountLabel.arg(VPNDeviceModel.activeDevices).arg(VPNUser.maxDevices)
            }

            Loader {
                id: helpIconButtonLoader
                active: true
                sourceComponent: MZIconButton {
                    objectName: "devicesHelpButton"

                    onClicked: {
                        helpSheet.open()
                    }

                    accessibleName: MZI18n.GetHelpLinkText

                    Image {
                        anchors.centerIn: parent

                        source: MZAssetLookup.getImageSource("Question")
                        fillMode: Image.PreserveAspectFit
                    }
                }
            }
        }

        Rectangle {
            Layout.topMargin: MZTheme.theme.listSpacing
            Layout.fillWidth: true
            Layout.preferredHeight: 1

            color: MZTheme.colors.divider
        }

        VPNDeviceList {
            id: deviceList
            Layout.fillWidth: true
            onRemoveItem: (name, publicKey) => { removePopup.initializeAndOpen(name, publicKey) }
        }
    }

    VPNRemoveDevicePopup {
        id: removePopup

        function initializeAndOpen(name, publicKey) {
            removePopup.deviceName = name;
            removePopup.devicePublicKey = publicKey;
            removePopup.open();
        }
    }

    MZHelpSheet {
        id: helpSheet
        objectName: "devicesHelpSheet"

        title: MZI18n.HelpSheetsDevicesTitle

        model: [
            {type: MZHelpSheet.BlockType.Title, text: MZI18n.HelpSheetsDevicesHeader},
            {type: MZHelpSheet.BlockType.Text, text: MZI18n.HelpSheetsDevicesBody1, margin: MZTheme.theme.helpSheetTitleBodySpacing},
            {type: MZHelpSheet.BlockType.Text, text: MZI18n.HelpSheetsDevicesBody2, margin: MZTheme.theme.helpSheetBodySpacing},
            {type: MZHelpSheet.BlockType.LinkButton, text: MZI18n.GlobalLearnMore, margin: MZTheme.theme.helpSheetBodyButtonSpacing, objectName: "learnMoreLink", action: () => {
                    MZUrlOpener.openUrlLabel("sumoDevices")
                }}
        ]
    }

    Connections {
        target: deviceList
        function onIsEditing(isEditing) {
            vpnFlickable.isEditing = isEditing
        }
    }

    Component.onCompleted: {
        VPN.refreshDevices()
    }
}
