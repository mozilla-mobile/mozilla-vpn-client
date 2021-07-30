/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.14
import Mozilla.VPN 1.0

import "../components"
import "../components/forms"
import "../themes/themes.js" as Theme

Item {
  // TODO
  // This view should inform the user that "something is happening".
  // In the background, we are starting the authentication flow.
  // The next states are:
  // - StateStart: happy path.
  // - Authentication failed, network down, etc.
    id: root
    anchors.fill: parent

    Image {
        id: spinner

        anchors.horizontalCenter: root.horizontalCenter
        anchors.verticalCenter: root.verticalCenter
        sourceSize.height: 80
        sourceSize.width: 80
        antialiasing: true

        fillMode: Image.PreserveAspectFit
        source: "../resources/spinner.svg"
    }

    ParallelAnimation {
        id: startSpinning

        running: true

        PropertyAnimation {
            target: spinner
            property: "opacity"
            from: 0
            to: .8
            duration: 300
        }

        PropertyAnimation {
            target: spinner
            property: "scale"
            from: 0.8
            to: 1
            duration: 300
        }

        PropertyAnimation {
            target: spinner
            property: "rotation"
            from: 0
            to: 360
            duration: 10000
            loops: Animation.Infinite
        }

    }

  Component.onCompleted: {
      console.log("INITIALIZE");
  }

}
