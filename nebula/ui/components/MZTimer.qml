/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

Timer {
    id: timer

    function setTimeout(callback, timeoutDuration) {
        timer.interval = timeoutDuration;
        timer.repeat = false;
        timer.triggered.connect(callback);
        timer.triggered.connect(function release() {
            timer.triggered.disconnect(callback);
            timer.triggered.disconnect(release);
        });
        timer.start();
    }
}