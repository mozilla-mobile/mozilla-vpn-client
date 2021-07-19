/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.14
import QtQuick.Layouts 1.14
import "../themes/themes.js" as Theme

// VPNIconAndLabel
RowLayout {
    property alias icon: icon.source
    property alias title: title.text

    spacing: 0
    height: parent.height

    VPNIcon {
        id: icon

        Layout.alignment: Qt.AlignHCenter
    }

    VPNBoldLabel {
        id: title

        Layout.leftMargin: 14
        Layout.alignment: Qt.AlignVCenter
        // VPNIconAndLabel is only used inside a VPNClickableRow
        // which acts as atomic interactive control thus we want
        // to hide its content (such as VPNIconAndLabel) from
        // assistive technology.
        Accessible.ignored: true
        wrapMode: Text.WordWrap
        Layout.fillWidth: true
    }

}
