/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11
import "../themes/themes.js" as Theme

VPNClickableRow {
    property var settingTitle
    property var imageLeftSrc
    property var imageRightSrc
    accessibleName: settingTitle

    anchors.left: undefined
    anchors.right: undefined
    Layout.fillWidth: true
    Layout.preferredHeight: Theme.rowHeight

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 8
        anchors.rightMargin: 8

        VPNIconAndLabel {
            icon: imageLeftSrc
            title: settingTitle
        }

        Item {
            Layout.fillWidth: true
        }

        VPNIcon {
            id: imageRight

            source: imageRightSrc
        }

    }
}
