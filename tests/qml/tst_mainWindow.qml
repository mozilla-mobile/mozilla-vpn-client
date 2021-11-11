import QtQuick 2.3
import QtTest 1.0

import ui 0.1

Item {
    Main {
        id: mainTest
    }

    TestCase {
        name: "MainTests"
        when: windowShown

        function test_isWasmApp() {
            compare(mainTest.isWasmApp, false)
        }
    }
}