/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import compat 0.1
import components 0.1

MZSimplePopup {
    id: root
    property bool receivedPing: false;

    anchors.centerIn: parent
    imageSrc: receivedPing ? "qrc:/ui/resources/server-unavailable.svg" : "qrc:/ui/resources/server-unavailable2.svg"
    imageSize: Qt.size(80, 80)
    title: receivedPing ? MZI18n.ServerUnavailableModalHeaderText : MZI18n.ServerUnavailableModalHeaderText2
    // In case the handshake failed but the ping succeeded - use the Firewall Error Message
    description: receivedPing ? MZI18n.ServerUnavailableNotificationBodyTextFireWallBlocked : MZI18n.ServerUnavailableModalBodyText2.arg(VPNCurrentServer.localizedExitCityName)
    buttons: [
        MZButton {
            text: receivedPing ? MZI18n.ServerUnavailableModalButtonLabel : MZI18n.ServerUnavailableModalButtonLabel2
            Layout.fillWidth: true
            onClicked: {
                MZNavigator.requestScreen(VPN.ScreenHome)
                window.showServerList();
                root.close();
            }
        }]
}
