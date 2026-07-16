/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import "qrc:/nebula/utils/MZAssetLookup.js" as MZAssetLookup

MZSimplePopup {
    id: root
    property bool receivedPing: false;

    anchors.centerIn: parent
    imageSrc: MZAssetLookup.getImageSource("GlobeNoConnection")
    imageSize: Qt.size(80, 80)
    title: MZI18n.ServerUnavailableModalHeaderText
    description: {
        // In case the handshake failed but the ping succeeded - use the Firewall Error Message as the first sentence
        const firstPart = receivedPing ? MZI18n.ServerUnavailableNotificationBodyTextFireWallBlocked
                                        : MZI18n.ServerUnavailableModalBodyText1
        const secondPart = MZSettings.obfuscationPolicy === MZSettings.NoObfuscation ?
                     MZI18n.ServerUnavailableModalBodyText2EnableObfuscation : MZI18n.ServerUnavailableModalBodyText2ChangeObfuscation
        return firstPart + " " + secondPart
    }
    closeButtonObjectName: "serverUnavailablePopup-closeButton"
    buttons: [
        MZButton {
            text: MZI18n.ServerUnavailableModalButtonLabel
            objectName: "serverUnavailablePopup-button"
            Layout.fillWidth: true
            onClicked: {
                MZNavigator.requestScreen(VPN.ScreenHome)
                window.showServerList(true);
                root.close();
            }
        },
        MZButton {
            text: MZI18n.ServerUnavailableModalConfigureObfuscationButtonLabel
            objectName: "serverUnavailablePopup-configureObfuscationButton"
            Layout.fillWidth: true
            onClicked: {
                window.pendingShowObfuscationSettings = true;
                MZNavigator.requestScreen(VPN.ScreenSettings);
                root.close();
            }
        }]
}
