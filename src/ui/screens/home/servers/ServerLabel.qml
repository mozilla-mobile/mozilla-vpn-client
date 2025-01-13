/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1
import compat 0.1
import "qrc:/nebula/utils/MZAssetLookup.js" as MZAssetLookup

RowLayout {
    property var serversList
    property bool narrowStyle: true
    property string fontColor: MZTheme.colors.fontColor
    id: root

    Flow {
        id: serverItems

        spacing: MZTheme.theme.listSpacing
        Layout.fillWidth: true

        Repeater {
            id: serverRepeater
            model: serversList
            delegate: RowLayout {
                spacing: narrowStyle
                    ? MZTheme.theme.listSpacing
                    : MZTheme.theme.listSpacing * 1.5

                Image {
                    id: flag
                    fillMode: Image.PreserveAspectFit

                    Layout.preferredWidth: MZTheme.theme.windowMargin
                    Layout.preferredHeight: MZTheme.theme.windowMargin
                    Layout.alignment: Qt.AlignLeft | Qt.AlignCenter

                    Component.onCompleted: {
                        if (typeof(countryCode) !== "undefined") {
                            return source = "qrc:/ui/resources/flags/" + countryCode.toUpperCase() + ".png"
                        }

                        if (typeof(modelData.countryCode) === "undefined" || modelData.countryCode === "") {
                            return visible = false;
                        }

                        source = "qrc:/ui/resources/flags/" + modelData.countryCode.toUpperCase() + ".png"

                    }
                }

                MZLightLabel {
                    id: serverLocation
                    objectName: root.objectName + '-label'
                    color: root.fontColor
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

                MZIcon {
                    id: arrowIcon

                    source: MZAssetLookup.getImageSource("ArrowForward")
                    sourceSize.height: MZTheme.theme.iconSize
                    sourceSize.width: MZTheme.theme.iconSize
                    Layout.leftMargin: MZTheme.theme.listSpacing
                    Layout.rightMargin: MZTheme.theme.listSpacing
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
