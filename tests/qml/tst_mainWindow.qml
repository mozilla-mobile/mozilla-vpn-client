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

        function test_onCompletedCallsMainWindowLoaded() {
            // TODO: Need a companion test, that:
            // a) mainWindowLoaded calls initializeGlean
            // b) sets up a timer
            compare(true, false)
        }

        function test_onSetGleanSourceTagsPassesTagsToGlean() {
            compare(true, false)
        }

        function test_onSendGleanPingsSubmitsPings() {
            compare(true, false)
        }

        function test_onRecordGleanEventRecordsAppropriateSample() {
            compare(true, false)
        }

        function test_onAboutToQuitCallsGleanShutdown() {
            compare(true, false)
        }

        function test_onInitializeGleanSetsDebugValuesInDebugMode() {
            compare(true, false)
        }

        function test_onInitializeGleanSetsChannelBasedOnStagingMode() {
            compare(true, false)
        }

        function test_onInitializeGleanCallsInitializeWithCoreSettings() {
            compare(true, false)
        }

    }
}