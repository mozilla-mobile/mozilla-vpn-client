/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.9
import vpn.mozilla.lottie 1.0

Item {
    id: lottieItem

    // The source can be a QRC URL only.
    property alias source: lottiePrivate.source

    // Speed of execution. Default: 1.0
    property alias speed: lottiePrivate.speed

    // Possible values:
    // - true: infinite loop
    // - false: no loops
    // - number: the number of loops
    // Default: false
    property alias loops: lottiePrivate.loops

    // Reverse direction. Default: false
    property alias reverse: lottiePrivate.reverse

    // Read-only animation status. The status is an object containing the
    // following properties:
    // - playing: readonly boolean. Default: false
    // - currentTime: readonly double. Default: 0
    // - totalTime: readonly integer. Default: 0
    property alias status: lottiePrivate.status

    // Enable/disable the auto-play. Default: false
    property alias autoPlay: lottiePrivate.autoPlay

    // Similar to Image.fillMode, the supported values are:
    // - "stretch": the image is scaled to fit
    // - "preserveAspectFit": the image is scaled uniformly to fit without cropping
    // - "preserveAspectCrop": the image is scaled uniformly to fill, cropping if necessary
    // - "pad": the image is not transformed
    property alias fillMode: lottiePrivate.fillMode

    function play() { lottiePrivate.play(); }
    function pause() { lottiePrivate.pause(); }
    function stop() { lottiePrivate.stop(); }

    signal loopCompleted()

    Item {
        id: container
        anchors.fill: parent

        // HTML DOM compatibility API
        property real offsetWidth: width
        property real offsetHeight: height
        property var style: ({})
        property string innerText
        function appendChild(item) {
            item.parent = this;
        }
    }

    Canvas {
        id: canvas
        anchors.fill: parent

        // HTML DOM compatibility API
        property real offsetWidth: width
        property real offsetHeight: height
        property var style: ({})
        function setAttribute(name, value) {
            // does nothing
        }
    }

    onWidthChanged: Qt.callLater(lottiePrivate.clearAndResize)
    onHeightChanged: Qt.callLater(lottiePrivate.clearAndResize)

    LottiePrivate {
        id: lottiePrivate

        property bool componentCompleted: false

        readyToPlay: canvas.available && componentCompleted && source && lottieItem.visible
    }

    onParentChanged: Qt.callLater(lottiePrivate.destroyAndRecreate);

    Component.onCompleted: {
        lottiePrivate.loopCompleted.connect(loopCompleted);

        lottiePrivate.componentCompleted = true;
        lottiePrivate.setCanvasAndContainer(canvas, container);
    }
}
