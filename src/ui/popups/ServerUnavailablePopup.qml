/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import compat 0.1
import components 0.1
import "qrc:/nebula/utils/MZAssetLookup.js" as MZAssetLookup

MZSimplePopup {
    id: root
    property bool receivedPing: false;

    anchors.centerIn: parent
    imageSrc: MZAssetLookup.getImageSource("GlobeNoConnection")
    imageSize: Qt.size(80, 80)
    title: MZI18n.ServerUnavailableModalHeaderText
    // In case the handshake failed but the ping succeeded - use the Firewall Error Message
    description: receivedPing ? MZI18n.ServerUnavailableNotificationBodyTextFireWallBlocked : MZI18n.ServerUnavailableModalBodyText
    buttons: [
        MZButton {
            text: MZI18n.ServerUnavailableModalButtonLabel
            objectName: "serverUnavailablePopup-button"
            Layout.fillWidth: true
            onClicked: {
                MZNavigator.requestScreen(VPN.ScreenHome)
                window.showServerList();
                root.close();
            }
        }]
}
