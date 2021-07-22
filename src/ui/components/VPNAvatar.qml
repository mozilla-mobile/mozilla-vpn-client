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

        anchors.centerIn: parent
        asynchronous: true
        height: logoRoot.height
        fillMode: Image.PreserveAspectFit
        layer.enabled: true
        layer.effect: OpacityMask {
            maskSource: avatarMask
        }
        smooth: true
        source: avatarUrl
    }

    Image {
        id: avatarPlaceholder

        property int avatarSourceSize: 320

        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        smooth: true
        source: "../resources/avatar-default.png"
        sourceSize.height: imageSourceSize
        sourceSize.width: imageSourceSize
        visible: avatar.status !== Image.Ready
    }

    Rectangle {
        id: avatarMask

        height: avatar.height
        radius: avatar.width / 2
        visible: false
        width: avatar.height
    }
}
