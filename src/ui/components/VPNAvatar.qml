/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtGraphicalEffects 1.14

Item {
    id: logoRoot

    property string avatarUrl: ""  

    Image {
        id: avatar

        asynchronous: true
        fillMode: Image.PreserveAspectFit
        source: avatarUrl
        smooth: true
        layer.enabled: true
        layer.effect: OpacityMask {
            maskSource: avatarMask
        }
        anchors.centerIn: parent
        height: logoRoot.height
    }

    Image {
        id: avatarPlaceholder

        property int avatarSourceSize: 320

        anchors.fill: parent
        smooth: true
        fillMode: Image.PreserveAspectFit
        source: "../resources/avatar-default.png"
        sourceSize.height: imageSourceSize
        sourceSize.width: imageSourceSize
        visible: avatar.status !== Image.Ready
    }

    Rectangle {
        id: avatarMask

        height: avatar.height
        width: avatar.height
        radius: avatar.width / 2
        visible: false
    }
}
