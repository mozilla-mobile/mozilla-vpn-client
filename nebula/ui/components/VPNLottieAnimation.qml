/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14

import Qt.labs.lottieqt 1.0
import QtMultimedia 5.15

Rectangle {
    id: root

    anchors.fill: parent
    color: "pink"

    // Text {
    //     text: video.position
    // }

    // Timer {
    //     interval: 500
    //     running: true
    //     repeat: true
    //     onTriggered: {
    //         console.log("video test - before: ", video.position);
    //         video.seek(video.position - 500);
    //         console.log("video test - after: ", video.position);
    //     }
    // }

    // LottieAnimation {
    //     id: sparkle
    //     // anchors.fill: parent
    //     anchors.horizontalCenter: parent.horizontalCenter
    //     anchors.verticalCenter: parent.verticalCenter

    //     scale: 1
    //     source: "qrc:/nebula/resources/animations/test_animation.json"
    //     // source: "qrc:/nebula/resources/animations/star_animation.json"
    //     // source: "qrc:/nebula/resources/animations/clock_animation.json"
    //     // source: "qrc:/nebula/resources/animations/vpnactive_animation.json"
    //     autoPlay: true
    //     frameRate: 20
    //     loops: LottieAnimation.Infinite
    //     opacity: 1
    //     z: 1

    //     onStatusChanged: {
    //         console.log("onStatusChanged");

    //         if (status === LottieAnimation.Null) {
    //             console.log("LottieAnimation.Null");
    //         }
    //         if (status === LottieAnimation.Loading) {
    //             console.log("LottieAnimation.Loading");
    //         }
    //         if (status === LottieAnimation.Ready) {
    //             console.log("LottieAnimation.Ready");
    //         }
    //         if (status === LottieAnimation.Error) {
    //             console.log("LottieAnimation.Error");
    //         }
    //     }
    // }


    // Qt5 & Qt6.3: Video
    // Video {
    //     id: video

    //     source: "qrc:/nebula/resources/animations/globe_vid_Large.mp4"
    //     autoPlay: true
    //     flushMode: VideoOutput.FirstFrame
    //     loops: MediaPlayer.Infinite

    //     anchors.fill: parent
    // }

    // MouseArea {
    //     anchors.fill: parent
    //     onPressed: {
    //         video.seek(video.position - 500);
    //         // video.playbackState == MediaPlayer.PlayingState ? video.pause() : video.play()
    //     }
    // }


    // Qt6.2: MediaPlayer
    // MediaPlayer {
    //     id: player
    //     source: "qrc:/nebula/resources/animations/globe_vid_Large.mp4"
    //     videoOutput: videoOutput
    // }

    // VideoOutput {
    //     id: videoOutput
    //     anchors.fill: parent
    // }


    // Qt5: MediaPlayer
    // MediaPlayer {
    //     id: mediaplayer
    //     source: "qrc:/nebula/resources/animations/globe_vid_Large.mp4"
    //     loops: MediaPlayer.Infinite
    // }

    // VideoOutput {
    //     anchors.fill: parent
    //     source: mediaplayer
    // }

    // MouseArea {
    //     id: playArea
    //     anchors.fill: parent
    //     onPressed: mediaplayer.play();
    // }

    // Qt5 & Qt6: AnimatedImage
    AnimatedImage {
        id: animation
        source: "qrc:/nebula/resources/animations/globe_gif_Large.gif"

        height: parent.width / 1.75
        width: parent.width

        anchors.verticalCenter: parent.verticalCenter
    }
}
