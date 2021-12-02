/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

import themes 0.1

Rectangle {
    id: loader
    anchors.fill: parent
    anchors.margins: -1
    radius: Theme.cornerRadius
    opacity: 0
    color: "#98bff2"

    state: "inactive"

    states: [
        State {
            name: "active"
        },

        State {
            name: "inactive"
        }
    ]

    transitions: [
        Transition {
            to: "active"
            ParallelAnimation {
                PropertyAnimation {
                    target: loader
                    property: "opacity"
                    from: 0
                    to: 1
                    duration: 200
                    easing.type: Easing.OutCurve
                }

                PropertyAnimation {
                    target: loadingIcon
                    property: "scale"
                    from: 0.4
                    to: 1
                    duration: 200
                    easing.type: Easing.OutCurve
                }
            }
        },

        Transition {
            to: "inactive"

            ParallelAnimation {
                PropertyAnimation {
                    target: loader
                    property: "opacity"
                    from: 1
                    to: 0
                    duration: 200
                    easing.type: Easing.OutCurve
                }

                PropertyAnimation {
                    target: loadingIcon
                    property: "scale"
                    from: 1
                    to: 0.4
                    duration: 200
                    easing.type: Easing.OutCurve
                }
            }
        }
    ]

    VPNIcon {
        id: loadingIcon

        source: "qrc:/nebula/resources/buttonLoader.svg"
        anchors.centerIn: loader
        sourceSize.height: 28
        sourceSize.width: 28
    }

    PropertyAnimation {
        id: animation
        running: loader.state == "active"
        target: loadingIcon
        property: "rotation"
        from: 0
        to: 360
        duration: 4000
        loops: Animation.Infinite
    }

}
