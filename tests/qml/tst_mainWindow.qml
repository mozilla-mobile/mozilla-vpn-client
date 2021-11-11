import QtQuick 2.3
import QtTest 1.0

import main 0.1

Item {
    VPN {
        id: mainTest
    }

    TestCase {
        name: "MainTests"
        when: windowShown

        function test_isWasmApp() {
            //verify(mainTest.iosSafeAreaTopMargin.color, "blue")
            verify(mainTest.isWasmApp === false, "isWasmApp is not false. " + mainTest.isWasmApp)
        }
    }
}