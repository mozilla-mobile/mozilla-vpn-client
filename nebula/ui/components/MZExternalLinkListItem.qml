/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0

// MZExternalLinkListItem
MZClickableRow {
    property alias title: title.text
    property string iconSource:  "qrc:/nebula/resources/externalLink.svg"
    property alias iconMirror: icon.mirror

    backgroundColor: MZTheme.theme.clickableRowBlue

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: MZTheme.theme.windowMargin / 2
        anchors.rightMargin: MZTheme.theme.windowMargin / 2

        MZBoldLabel {
            id: title
        }

        Item {
            Layout.fillWidth: true
        }

        MZIcon {
            id: icon
            source: iconSource
        }

    }

}
