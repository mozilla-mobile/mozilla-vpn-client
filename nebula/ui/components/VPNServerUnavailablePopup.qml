/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14

import Mozilla.VPN 1.0

VPNSimplePopup {
    id: root

    anchors.centerIn: parent
    imageSrc: "qrc:/nebula/resources/server-unavailable.svg"
    imageSize: Qt.size(80, 80)
    title: VPNl18n.ServerUnavailableModalHeaderText
    description: VPNl18n.ServerUnavailableModalBodyText
    buttons: [
        VPNButton {
            text: VPNl18n.ServerUnavailableModalButtonLabel
            onClicked: {
                root.close();
                window.goToServersView();
            }
        }]
}
