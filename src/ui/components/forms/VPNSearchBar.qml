/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../../themes/themes.js" as Theme
import "./../../components"

VPNTextField {
    // TODO Add strings for Accessible.description, Accessible.name
    property bool hasError: false

    id: searchBar

    leftInset: 48
    leftPadding: 48
    onActiveFocusChanged: if (focus && vpnFlickable.ensureVisible) vpnFlickable.ensureVisible(searchBar)

    VPNIcon {
        source: "qrc:/ui/resources/search.svg"
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: 20
        sourceSize.height: Theme.windowMargin
        sourceSize.width: Theme.windowMargin
        opacity: parent.focus ? 1 : .8
    }

    background: VPNInputBackground {
        showError: hasError
    }

    Keys.onPressed: {
        if (focus && hasError && (/[\w\[\]`!@#$%\^&*()={}:;<>+'-]/).test(event.text)) {
            event.accepted = true;
        }
    }
}
