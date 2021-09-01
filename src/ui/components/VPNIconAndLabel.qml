/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import "../themes/themes.js" as Theme
import "../themes/colors.js" as Color

// VPNIconAndLabel
Item {
    id: iconAndLabel
    property alias icon: icon.source
    property alias title: title.text
    property string fontColor: Theme.fontColorDark
    property bool showIndicator: false

    implicitHeight: title.implicitHeight
    width: parent.width

    VPNIcon {
        id: icon

        anchors.verticalCenter: parent.verticalCenter
    }

    VPNBoldLabel {
        id: title

        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        color: fontColor
        // VPNIconAndLabel is only used inside a VPNClickableRow
        // which acts as atomic interactive control thus we want
        // to hide its content (such as VPNIconAndLabel) from
        // assistive technology.
        Accessible.ignored: true
        wrapMode: Text.WordWrap
        leftPadding: icon.width + 14
        rightPadding: icon.width + 14
        width: title.implicitWidth < parent.width ? undefined : parent.width

        Rectangle {
            id: indicator

            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.rightMargin: icon.width
            color: Color.error.default
            height: Theme.iconSize / 2
            radius: width / 2
            visible: iconAndLabel.showIndicator
            width: Theme.iconSize / 2
        }
    }
}
