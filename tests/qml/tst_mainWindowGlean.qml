import QtQuick 2.3
import QtTest 1.0

import TestHelper 1.0
import Mozilla.VPN 1.0
import org.mozilla.Glean 0.24
import ui 0.1

Item {
    Main {
        id: mainTestGlean
    }

    TestCase {
        name: "MainTestsGlean"

        property var spyApplicationId
        property var spyUploadEnabled
        property var spyConfig
        property var spyTags
        property var spyShutdownCalled: false

        function initTestCase() {
            function mockGleanInitialize(applicationId, uploadEnabled, config) {
                spyApplicationId = applicationId;
                spyUploadEnabled = uploadEnabled;
                spyConfig = config;
            }
            Glean.initialize = mockGleanInitialize;
            function mockGleanSetSourceTags(tags) {
                spyTags = tags;
            }
            Glean.setSourceTags = mockGleanSetSourceTags;
            function mockGleanShutdown() {
                // Should be false first
                compare(spyShutdownCalled, false);
                spyShutdownCalled = true;
            }
            Glean.shutdown = mockGleanShutdown;
        }

        function test_onCompletedCallsMainWindowLoaded() {
            // TODO: Need a companion test, that:
            // a) mainWindowLoaded calls initializeGlean
            // b) sets up a timer
            compare(true, false)
        }

        function test_onSetGleanSourceTagsPassesTagsToGlean() {
            TestHelper.triggerSetGleanSourceTags("tag1,tag2")
            compare(spyTags, "tag1")
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
            compare(spyApplicationId, "mzillavpn");
        }

        function test_onGleanEnabledChangedCallsGleanSetUploadEnabledCorrectly() {
            compare(true, false)
        }

    }
}