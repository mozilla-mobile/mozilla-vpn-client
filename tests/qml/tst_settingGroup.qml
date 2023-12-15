/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.3
import QtTest 1.0

import components.forms 0.1
import Mozilla.Shared 1.0


Item {
    width: 600;
    height: 800;

    MZTextArea {
        id: vpnTextAreaTest
    }

    TestCase {
        name: "SettingGroup"
        when: windowShown

        function test_settingsAreExposedToQML() {
            compare(TestSettingGroup["aKey"], "Hello, QML!");
        }
    }
}
