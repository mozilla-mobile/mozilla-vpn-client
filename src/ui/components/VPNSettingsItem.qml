/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import "../themes/themes.js" as Theme
import "../themes/colors.js" as Color

VPNClickableRow {
    property var settingTitle
    property var imageLeftSrc
    property var imageRightSrc
    property bool showIndicator
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

            Rectangle {
                id: indicator

                color: Color.error.default
                height: Theme.iconSize / 2
                radius: width / 2
                visible: showIndicator
                width: Theme.iconSize / 2
            }
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
