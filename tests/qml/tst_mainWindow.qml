import QtQuick 2.3
import QtTest 1.0

TestCase {
    name: "WindowTests"
    when: windowShown

    function test_isWasmApp() {
        wait(2000)
        //verify(window.iosSafeAreaTopMargin.color, "blue")
        //verify(window.isWasmApp === false, "isWasmApp is not false. " + window.isWasmApp)
    }
}
