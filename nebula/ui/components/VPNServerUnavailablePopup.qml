/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0

VPNSimplePopup {
    id: root
    property bool receivedPing: false;

    anchors.centerIn: parent
    imageSrc: "qrc:/nebula/resources/server-unavailable.svg"
    imageSize: Qt.size(80, 80)
    title: VPNI18n.ServerUnavailableModalHeaderText
    // In case the handshake failed but the ping succeeded - use the Firewall Error Message
    description: receivedPing ? VPNI18n.ServerUnavailableNotificationBodyTextFireWallBlocked : VPNI18n.ServerUnavailableModalBodyText
    buttons: [
        VPNButton {
            text: VPNI18n.ServerUnavailableModalButtonLabel
            Layout.fillWidth: true
            onClicked: {
                VPNNavigator.requestScreen(VPNNavigator.ScreenHome)
                window.showServerList();
                root.close();
            }
        }]
}
