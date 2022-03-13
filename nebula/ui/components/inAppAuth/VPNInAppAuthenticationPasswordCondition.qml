/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1

RowLayout {

    property bool _passwordConditionIsSatisfied: false
    property alias _passwordConditionDescription: passwordConditionDescription.text
    Layout.maximumWidth: 260

    VPNIcon {
        source: parent._passwordConditionIsSatisfied ? "qrc:/nebula/resources/checkmark.svg" : "qrc:/nebula/resources/close-darker.svg"
        sourceSize.width: 12
        sourceSize.height: 12
        Layout.alignment: Qt.AlignTop
        Layout.topMargin: 3
    }

    Text {
        id: passwordConditionDescription
        Layout.fillWidth: true
        text: "Must be at least 8 characters"
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        lineHeight: 1
    }
}
