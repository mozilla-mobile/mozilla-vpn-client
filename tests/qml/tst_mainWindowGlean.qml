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
        /*
        function test_onCompletedCallsMainWindowLoaded() {
            // TODO: Need a companion test, that:
            // a) mainWindowLoaded calls initializeGlean
            // b) sets up a timer
            compare(true, false)
        }
        */

        function test_onSetGleanSourceTagsPassesTagsToGlean() {
            TestHelper.triggerSetGleanSourceTags("tag1,tag2")
            compare(spyTags.toString(), "tag1,tag2")
        }

        /*
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

        function test_onGleanEnabledChangedCallsGleanSetUploadEnabledCorrectly() {
            compare(true, false)
        }
        */

        function test_onInitializeGleanSetsChannelBasedOnStagingMode() {
            VPN.stagingMode = true;
            TestHelper.triggerInitializeGlean()
            compare(spyConfig.channel, "staging")

            VPN.stagingMode = false;
            TestHelper.triggerInitializeGlean()
            compare(spyConfig.channel, "production")
        }

        function test_onInitializeGleanCallsUploadEnabledBasedOnVPNSettings() {
            VPNSettings.gleanEnabled = false;
            TestHelper.triggerInitializeGlean()
            compare(spyUploadEnabled, false)

            VPNSettings.gleanEnabled = true;
            TestHelper.triggerInitializeGlean()
            compare(spyUploadEnabled, true)
        }

        function test_onInitializeGleanCallsInitializeWithCoreSettings() {
            TestHelper.triggerInitializeGlean()
            compare(spyApplicationId, "mozillavpn")
        }
    }
}