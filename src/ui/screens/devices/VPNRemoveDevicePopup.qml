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
import "qrc:/nebula/utils/MZAssetLookup.js" as MZAssetLookup

MZSimplePopup {
    id: popup

    property var deviceName
    property var devicePublicKey
    property var wasmView

    anchors.centerIn: Overlay.overlay
    topPadding: 24
    showCloseButton: false
    imageSrc: MZAssetLookup.getImageSource("DevicesRemove")
    imageSize: Qt.size(116, 80)
    title: MZI18n.DevicesRemovePopupTitle
    description: MZI18n.DevicesRemovePopupDescription.arg(popup.deviceName)
    buttons: [
        MZPopupButton {
            objectName: "confirmRemoveDeviceButton"

            buttonText: MZI18n.DevicesRemovePopupPrimaryButtonLabel
            buttonTextColor: MZTheme.colors.fontColorInverted
            colorScheme: MZTheme.colors.destructiveButton
            onClicked: {
                VPN.removeDeviceFromPublicKey(popup.devicePublicKey);
                popup.close();
            }
            isCancelBtn: false
        },
        MZCancelButton {
            Layout.alignment: Qt.AlignHCenter
            linkColor: MZTheme.colors.normalButton
            fontName: MZTheme.theme.fontInterFamily
            onClicked: {
                popup.close();
            }
        }
    ]
}
