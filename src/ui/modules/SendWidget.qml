/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.14
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0

import "../components"

VPNPanel {
    logo: "../resources/logoSend.png"
    logoTitle: "Mozilla VPN"
    Layout.fillWidth: true
    anchors.horizontalCenter: undefined
    height: undefined
    width: undefined

    Layout.preferredHeight: childrenRect.height

    MouseArea {
      anchors.fill: parent
      propagateComposedEvents: true
      onPressed: {
        VPNModuleModel.loadModule("send");
      }
   }
}

