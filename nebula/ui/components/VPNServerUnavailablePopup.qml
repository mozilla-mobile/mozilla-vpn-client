/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import themes 0.1

VPNPopup {
    id: root

    anchors.centerIn: parent
    maxWidth: Theme.desktopAppWidth
    contentItem: ColumnLayout {
        id: popupContentItem

        Item {
            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: Theme.listSpacing
            Layout.preferredHeight: 80
            Layout.preferredWidth: 80
            Layout.topMargin: Theme.vSpacing * 1.5

            Image {
                anchors.fill: parent
                source: "qrc:/nebula/resources/server-unavailable.svg"
                sourceSize.height: parent.height
                sourceSize.width: parent.width
                fillMode: Image.PreserveAspectFit
            }
        }

        VPNMetropolisLabel {
            color: Theme.fontColorDark
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: Theme.fontSizeLarge
            text: VPNl18n.ServerUnavailableModalHeaderText

            Layout.bottomMargin: Theme.listSpacing
            Layout.fillWidth: true
        }

        VPNTextBlock {
            horizontalAlignment: Text.AlignHCenter
            text: VPNl18n.ServerUnavailableModalBodyText

            Layout.fillWidth: true
            Layout.preferredWidth: parent.width
        }

        VPNButton {
            radius: Theme.cornerRadius
            text: VPNl18n.ServerUnavailableModalButtonLabel

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignBottom
            Layout.topMargin: Theme.vSpacing

            onClicked: {
                noUpdateAvailablePopup.close();
            }
        }
    }

    Component.onCompleted: {
        root.open();
    }
}
