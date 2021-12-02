/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import themes 0.1

RowLayout {
    property var serversList
    id: root

    Flow {
        id: serverItems

        spacing: Theme.listSpacing
        Layout.fillWidth: true

        Repeater {
            id: serverRepeater
            model: serversList
            delegate: RowLayout {
                spacing: Theme.listSpacing

                Image {
                    id: flag
                    fillMode: Image.PreserveAspectFit

                    Layout.preferredWidth: Theme.windowMargin
                    Layout.preferredHeight: Theme.windowMargin
                    Layout.alignment: Qt.AlignLeft | Qt.AlignCenter

                    Component.onCompleted: {
                        if (typeof(countryCode) !== "undefined") {
                            return source = "qrc:/nebula/resources/flags/" + countryCode.toUpperCase() + ".png"
                        }

                        if (typeof(modelData.countryCode) === "undefined" || modelData.countryCode === "") {
                            return visible = false;
                        }

                        source = "qrc:/nebula/resources/flags/" + modelData.countryCode.toUpperCase() + ".png"

                    }
                }

                VPNLightLabel {
                    id: serverLocation
                    text: {
                        if (typeof(localizedCityName) !== "undefined") {
                             return localizedCityName
                         }
                         if (modelData.localizedCityName) {
                             return modelData.localizedCityName
                         }
                         return "";
                        }
                    Accessible.ignored: true
                    Layout.alignment: Qt.AlignLeft
                }

                VPNIcon {
                    id: arrowIcon

                    source: "qrc:/nebula/resources/arrow-forward.svg"
                    sourceSize.height: Theme.iconSize
                    sourceSize.width: Theme.iconSize
                    Layout.leftMargin: Theme.listSpacing
                    Layout.rightMargin: Theme.listSpacing
                    visible: (index !== serverRepeater.count - 1)
                    Component.onCompleted: {
                        if (typeof(countryCode) !== "undefined") {
                            return;
                        }

                        if (typeof(serverRepeater) === "undefined" || modelData.countryCode === "") {
                            return visible = false;
                        }
                    }
                }
            }
        }
    }

}
