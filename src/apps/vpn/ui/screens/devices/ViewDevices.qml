/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1

MZViewBase {
    id: vpnFlickable
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

            enabled: VPNDeviceModel.rowCount() > 1
            labelText: !vpnFlickable.isEditing ? MZI18n.InAppMessagingEditButton : MZI18n.InAppSupportWorkflowSupportResponseButton
            onClicked: {
                vpnFlickable.isEditing = !vpnFlickable.isEditing
            }
        }
    }

    onIsEditingChanged: deviceList.isEditingChanged(isEditing)

    _menuTitle: MZI18n.DevicesMenuTitle
    _viewContentData: ColumnLayout {
        id: content

        spacing: 0

        MZInterLabel {
            Layout.topMargin: -8
            Layout.leftMargin: MZTheme.theme.windowMargin * 1.5
            Layout.rightMargin: MZTheme.theme.windowMargin * 1.5
            Layout.fillWidth: true

            horizontalAlignment: Text.AlignLeft
            color: MZTheme.theme.fontColor
            text: MZI18n.DevicesCountLabel.arg(VPNDeviceModel.activeDevices).arg(VPNUser.maxDevices)
        }

        Rectangle {
            Layout.topMargin: MZTheme.theme.listSpacing
            Layout.fillWidth: true
            Layout.preferredHeight: 1

            color: MZTheme.colors.grey10
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
