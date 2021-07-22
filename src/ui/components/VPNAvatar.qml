/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0

Item {
    id: logoRoot

    property string avatarUrl: ""

    Image {
        id: avatar

        anchors.fill: logoRoot
        asynchronous: true
        fillMode: Image.PreserveAspectFit
        source: avatarUrl
        visible: avatar.status === Image.Ready

        Image {
            id: avatarPlaceholder

            property int imageSourceSize: 320

            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            opacity: 1
            source: "../resources/avatar-default.png"
            sourceSize.height: imageSourceSize
            sourceSize.width: imageSourceSize
            visible: placeholderTransition.running
        }
    }

    states: [
        State {
            name: "hidePlaceholder";
            when: avatar.status === Image.Ready

            PropertyChanges {
                target: avatarPlaceholder
                opacity: 0
            }
        }
    ]

    transitions: [
        Transition {
            id: placeholderTransition
            to: "hidePlaceholder"

            NumberAnimation {
                properties: "opacity"
                easing.type: Easing.InCubic
                duration: 120
            }
        }
    ]
}
