/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtTest 1.0
import lottie 0.1

LottieAnimation {
    id: lottie
    anchors.fill: parent

    SignalSpy {
        id: loopCompletedSpy
        target: lottie
        signalName: "loopCompleted"
    }

    SignalSpy {
        id: statusChangedSpy
        target: lottie.status
        signalName: "changed"
    }

    TestCase {
        name: "LottieAnimation"
        when: windowShown

        function test_properties() {
            compare(lottie.source, "");
            lottie.source = "wow";
            compare(lottie.source, "wow");
            lottie.source = "";
            compare(lottie.source, "");

            compare(lottie.speed, 1);
            lottie.speed = 0;
            compare(lottie.speed, 0);
            lottie.speed = 2.4;
            compare(lottie.speed, 2.4);
            lottie.speed = -5;
            compare(lottie.speed, -5);
            lottie.speed = 1;
            compare(lottie.speed, 1);

            compare(lottie.loops, false);
            lottie.loops = true;
            compare(lottie.loops, true);
            lottie.loops = 12;
            compare(lottie.loops, 12);
            lottie.loops = 'wow';
            compare(lottie.loops, 12);
            lottie.loops = false;
            compare(lottie.loops, false);

            compare(lottie.reverse, false);
            lottie.reverse = true;
            compare(lottie.reverse, true);
            lottie.reverse = false;
            compare(lottie.reverse, false);

            compare(lottie.autoPlay, false);
            lottie.autoPlay = true;
            compare(lottie.autoPlay, true);
            lottie.autoPlay = false;
            compare(lottie.autoPlay, false);

            compare(lottie.fillMode, "stretch");
            lottie.fillMode = "wow";
            compare(lottie.fillMode, "stretch");
            lottie.fillMode = 12;
            compare(lottie.fillMode, "stretch");
            lottie.fillMode = "preserveAspectFit";
            compare(lottie.fillMode, "preserveAspectFit");
            lottie.fillMode = "stretch";
            compare(lottie.fillMode, "stretch");

            verify(!lottie.status.playing);

            verify(!!lottie.play);
            verify(!!lottie.pause);
            verify(!!lottie.stop);
        }

        function test_play() {
            lottie.source = ":/test.json";

            verify(!lottie.status.playing);
            lottie.play();
            verify(lottie.status.playing);
            tryVerify(() => !lottie.status.playing);

            lottie.source = "";
        }

        function test_playTwoLoops() {
            lottie.loops = 2;
            lottie.source = ":/test.json";

            verify(!lottie.status.playing);
            lottie.play();
            tryVerify(() => lottie.status.playing);

            loopCompletedSpy.clear();
            loopCompletedSpy.wait();
            compare(loopCompletedSpy.count, 1);
            loopCompletedSpy.wait();
            compare(loopCompletedSpy.count, 2);
            tryVerify(() => !lottie.status.playing);

            lottie.source = "";
            lottie.loops = false;
        }

        function test_playAndPause() {
            lottie.source = ":/test.json";

            verify(!lottie.status.playing);
            lottie.play();
            verify(lottie.status.playing);

            lottie.pause();
            verify(!lottie.status.playing);

            lottie.source = "";
        }

        function test_playAndStop() {
            lottie.source = ":/test.json";

            verify(!lottie.status.playing);
            lottie.play();
            verify(lottie.status.playing);

            lottie.stop();
            verify(!lottie.status.playing);

            lottie.source = "";
        }

        function test_playWithAutoplay() {
            lottie.autoPlay = true;
            lottie.source = ":/test.json";

            tryVerify(() => lottie.status.playing);
            tryVerify(() => !lottie.status.playing);

            lottie.autoPlay = false;
            lottie.source = "";
        }

        function test_playWithDoubleSpeed() {
            lottie.speed = 2;
            lottie.source = ":/test.json";

            verify(!lottie.status.playing);
            lottie.play();
            tryVerify(() => !lottie.status.playing);

            lottie.speed = 1;
            lottie.source = "";
        }

        function test_playChangingSpeed() {
            lottie.loops = true;
            lottie.source = ":/test.json";

            lottie.play();

            [3, 0.5].forEach(speed => {
              lottie.speed = speed;
              loopCompletedSpy.wait(10000);
            });

            lottie.stop();
            verify(!lottie.status.playing);

            lottie.speed = 1;
            lottie.loops = false;
            lottie.source = "";
        }

        function test_playReverse() {
            lottie.reverse = true;
            lottie.source = ":/test.json";

            verify(!lottie.status.playing);
            lottie.play();
            tryVerify(() => lottie.status.playing);
            tryVerify(() => !lottie.status.playing);

            lottie.reverse = false;
            lottie.source = "";
        }

        function test_playChangingDirection() {
            lottie.loops = true;
            lottie.source = ":/test.json";

            lottie.play();

            [false, true, false].forEach(reverse => {
              lottie.reverse = reverse;
              loopCompletedSpy.wait();
            });

            lottie.stop();
            verify(!lottie.status.playing);

            lottie.loops = false;
            lottie.source = "";
        }

        function test_playChangingFillMode() {
            lottie.loops = true;
            lottie.source = ":/test.json";

            lottie.play();

            ["stretch", "pad", "preserveAspectFit", "preserveAspectCrop"].forEach(fillMode => {
              lottie.fillMode = fillMode;
              loopCompletedSpy.wait();
            });

            lottie.stop();
            verify(!lottie.status.playing);

            lottie.loops = false;
            lottie.fillMode = "stretch"
            lottie.source = "";
        }

        function test_playChangingSource() {
            lottie.loops = true;
            lottie.source = ":/test.json";

            lottie.play();
            loopCompletedSpy.wait();

            lottie.source = ":/test.json";
            loopCompletedSpy.wait();

            lottie.stop();
            verify(!lottie.status.playing);

            lottie.speed = 1;
            lottie.loops = false;
            lottie.source = "";
        }

        function test_statusChanges() {
            lottie.source = ":/test.json";

            statusChangedSpy.clear();
            lottie.play();
            tryVerify(() => !lottie.status.playing);

            verify(statusChangedSpy.signalArguments.length > 3);

            // We start with 'playing' and we stop with 'not-playing'.
            verify(statusChangedSpy.signalArguments[0][0] === true);
            verify(statusChangedSpy.signalArguments[statusChangedSpy.signalArguments.length - 1][0] === false);

            // Checking the 'update' events. We must have them!
            const statusEvents = statusChangedSpy.signalArguments.filter(arguments => arguments[0] === true);
            verify(statusEvents.length > 0);

            // Incremental currentTime values.
            let prevCurrentTime = -1;
            statusEvents.forEach(event => {
              verify(prevCurrentTime < event[1]);
              prevCurrentTime = event[1];
            });

            // The currentTime is < than the totalTime.
            statusEvents.forEach((event, pos) => {
              verify(event[1] <= event[2]);
            });

            lottie.source = "";
        }
    }
}
