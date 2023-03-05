/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import compat 0.1
import components 0.1

MZSimplePopup {
    id: popup

    property var deviceName
    property var devicePublicKey
    property var wasmView

    anchors.centerIn: Overlay.overlay
    topPadding: 24
    showCloseButton: false
    imageSrc: "qrc:/nebula/resources/devicesRemove.svg"
    imageSize: Qt.size(116, 80)
    //% "Remove device?"
    title: qsTrId("vpn.devices.removeDeviceQuestion")
    //: %1 is the name of the device being removed. The name is displayed on purpose on a new line.
    //% "Please confirm you would like to remove\n%1."
    description: qsTrId("vpn.devices.deviceRemovalConfirm").replace("\n", " ").arg(popup.deviceName)
    buttons: [
        MZPopupButton {
            objectName: "confirmRemoveDeviceButton"

            //: This is the “remove” device button.
            //% "Remove"
            buttonText: qsTrId("vpn.devices.removeDeviceButton")
            buttonTextColor: MZTheme.theme.white
            colorScheme: MZTheme.theme.redButton
            onClicked: {
                VPN.removeDeviceFromPublicKey(popup.devicePublicKey);
                popup.close();
            }
            isCancelBtn: false
        },
        MZCancelButton {
            Layout.alignment: Qt.AlignHCenter
            onClicked: {
                popup.close();
            }
        }
    ]

    onClosed: {
        // When closing the dialog, put the focus back on the
        // remove button that originally triggered the dialog.
        if (wasmView) {
            return;
        }

        if (deviceList.focusedIconButton) {
            deviceList.focusedIconButton.forceActiveFocus();
        }
    }
}
