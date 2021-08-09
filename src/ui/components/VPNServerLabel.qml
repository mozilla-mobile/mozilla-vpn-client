/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14
import "../themes/themes.js" as Theme

RowLayout {
    property var servers: ({})
    property int imgSize: 20

    id: root

    Flow {
        id: serverItems

        spacing: Theme.listSpacing * 2
        Layout.fillWidth: true

        Repeater {
            id: serverRepeater
            model: servers
            delegate: serverDelegate
        }
    }

    Component {
        id: serverDelegate

        RowLayout {
            spacing: Theme.listSpacing

            Image {
                id: flag
                fillMode: Image.PreserveAspectFit
                source: "../resources/flags/" + modelData.countryCode.toUpperCase() + ".png"

                Layout.preferredWidth: imgSize
                Layout.preferredHeight: imgSize
                Layout.alignment: Qt.AlignLeft | Qt.AlignCenter
            }

            VPNMetropolisLabel {
                id: serverLocation
                text: modelData.localizedCityName
                Accessible.ignored: true
                Layout.alignment: Qt.AlignLeft
                elide: Text.ElideRight

                Rectangle {
                    height: parent.height
                    width: parent.width
                    color: "yellow"

                    z: -1
                }
            }

            VPNIcon {
                id: arrowIcon

                Layout.leftMargin: Theme.listSpacing
                source: "../resources/arrow-forward.svg"
                sourceSize.height: Theme.iconSize
                sourceSize.width: Theme.iconSize
                visible: index !== serverRepeater.model.length - 1

                Rectangle {
                    height: parent.height
                    width: parent.width
                    color: "lightblue"

                    z: -1
                }
            }
        }
    }


    Rectangle {
        height: parent.height
        width: parent.width
        color: "lightgrey"

        z: -1
    }
}
