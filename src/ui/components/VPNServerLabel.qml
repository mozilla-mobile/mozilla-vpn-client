/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14
import "../themes/themes.js" as Theme

RowLayout {
    property alias serversList: serverRepeater.model
    id: root

    Flow {
        id: serverItems

        spacing: Theme.listSpacing
        Layout.fillWidth: true

        Repeater {
            id: serverRepeater
            delegate: RowLayout {
                spacing: Theme.listSpacing

                Image {
                    id: flag
                    fillMode: Image.PreserveAspectFit

                    Layout.preferredWidth: Theme.windowMargin
                    Layout.preferredHeight: Theme.windowMargin
                    Layout.alignment: Qt.AlignLeft | Qt.AlignCenter

                    Component.onCompleted: {
                        if (typeof(countryCode) === "undefined") {
                            source = "../resources/back.svg";
                        } else {
                           source = "../resources/flags/" + countryCode.toUpperCase() + ".png"
                        }
                    }
                }

                VPNLightLabel {
                    id: serverLocation
                    text: localizedCityName
                    Accessible.ignored: true
                    Layout.alignment: Qt.AlignLeft
                }

                VPNIcon {
                    id: arrowIcon

                    source: "../resources/arrow-forward.svg"
                    sourceSize.height: Theme.iconSize
                    sourceSize.width: Theme.iconSize
                    Layout.leftMargin: Theme.listSpacing
                    Layout.rightMargin: Theme.listSpacing
                    Component.onCompleted: {
                        if (typeof(serverRepeater) === "undefined") {
                            visible = false;
                            return;
                        }
                        visible = (index !== serverRepeater.count - 1);
                    }
                }
            }
        }
    }

}
