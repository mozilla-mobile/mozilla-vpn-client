/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import Mozilla.VPN 1.0
import Mozilla.VPN.qmlcomponents 1.0
import components 0.1

ColumnLayout {
    id: layout

    property string title
    property string description

    property string type
    property var customFilter

    property alias list: tipsAndTricksListLoader.item
    visible: list ? list.count > 0 : false

    // Title
    VPNBoldLabel {
        Layout.fillWidth: true

        id: guideListTitle
        text: title
        wrapMode: Text.WordWrap

        Accessible.role: Accessible.StaticText
        Accessible.name: text
    }

    // Description
    VPNTextBlock {
        Layout.fillWidth: true
        Layout.topMargin: 4

        id: guideListDescription
        text: description
        wrapMode: Text.WordWrap

        Accessible.role: Accessible.StaticText
        Accessible.name: text
    }

    Loader {
        id: tipsAndTricksListLoader

        objectName: 'tipsAndTricksSetionLoader'

        Layout.topMargin: VPNTheme.theme.vSpacingSmall
        Layout.fillWidth: true
    }

    Component.onCompleted: {
        const source = type === "tutorials" ? "VPNTutorialList.qml" : "VPNGuideList.qml";
        const options = { customFilter };
        tipsAndTricksListLoader.setSource(source, options)
    }
}
