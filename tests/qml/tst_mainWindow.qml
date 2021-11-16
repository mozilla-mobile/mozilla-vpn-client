import QtQuick 2.3
import QtTest 1.0

import TestHelper 1.0
import Mozilla.VPN 1.0
import org.mozilla.Glean 0.24
import ui 0.1

Item {
    id: testItem
    property var applicationId
    property var uploadEnabled
    property var config
    property var tags
    property var shutdownCalled: false
    
    Main {
        id: mainTest
    }

    TestCase {
        name: "BeforeInit"

        function initTestCase() {
            function mockGleanInitialize(applicationId, uploadEnabled, config) {
                testItem.applicationId = applicationId;
                testItem.uploadEnabled = uploadEnabled;
                testItem.config = config;
            }
            Glean.initialize = mockGleanInitialize;
            function mockGleanSetSourceTags(tags) {
                testItem.tags = tags;
            }
            Glean.setSourceTags = mockGleanSetSourceTags;
            function mockGleanShutdown() {
                // Should be false first
                compare(testItem.shutdownCalled, false);
                testItem.shutdownCalled = true;
            }
            Glean.shutdown = mockGleanShutdown;
        }
    }

    TestCase {
        name: "MainTests"
        when: windowShown

        // Demo test for something simpler than the glean tests
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
            TestHelper.setGleanSourceTags("tag1,tag2")
            compare(testItem.tags, "tag1")
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
            compare(testItem.applicationId, "mzillavpn");
        }

        function test_onGleanEnabledChangedCallsGleanSetUploadEnabledCorrectly() {
            compare(true, false)
        }

    }
}