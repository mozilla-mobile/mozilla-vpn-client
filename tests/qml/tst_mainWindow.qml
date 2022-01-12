import QtQuick 2.3
import QtTest 1.0

import TestHelper 1.0
import Mozilla.VPN 1.0
import org.mozilla.Glean 0.30
import ui 0.1

// This is intended as a demo of a simpler test for the mainWindow
// compared to the GleanTests.

Item {
    Main {
        id: mainTest
    }

    TestCase {
        name: "MainTests"

        function test_isWasmApp() {
            compare(mainTest.isWasmApp, false)
        }
    }
}