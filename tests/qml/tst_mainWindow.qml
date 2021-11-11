import QtQuick 2.3
import QtTest 1.0

import components 0.1

Item {
    width: 600;
    height: 800;

    Window {
        id: windowTest
    }

    TestCase {
        name: "WindowTests"
        when: windowShown

        function test_isWasmApp() {
            verify(windowTest.isWasmApp === false, "isWasmApp is not false.")
        }
    }
}
