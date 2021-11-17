import QtQuick 2.3
import QtTest 1.0

import TestHelper 1.0
import Mozilla.VPN 1.0
import org.mozilla.Glean 0.24
import ui 0.1

Item {
    Main {
        id: mainTest
    }

    TestCase {
        name: "MainTestsGleanMocks"

        property var spyApplicationId
        property var spyUploadEnabledInitialize
        property var spyConfig
        property var spyTags
        property var spyShutdownCalled: false
        property var spyUploadEnabled

        function initTestCase() {
            // A series of mock functions that we monkeypatch onto Glean
            // that allow us to spy on the calls main.qml makes to Glean.
            function mockGleanInitialize(applicationId, uploadEnabled, config) {
                spyApplicationId = applicationId
                spyUploadEnabledInitialize = uploadEnabled
                spyConfig = config
            }
            Glean.initialize = mockGleanInitialize;
            function mockGleanSetSourceTags(tags) {
                spyTags = tags
            }
            Glean.setSourceTags = mockGleanSetSourceTags;
            function mockGleanShutdown() {
                // Should be false before setting it to true in this function.
                // Helps protect us from bad testing state.
                compare(spyShutdownCalled, false)
                spyShutdownCalled = true
            }
            Glean.shutdown = mockGleanShutdown
            function mockSetUploadEnabled(uploadEnabled) {
                spyUploadEnabled = uploadEnabled
            }
            Glean.setUploadEnabled = mockSetUploadEnabled
        }

        function test_onCompletedCallsMainWindowLoaded() {
            compare(TestHelper.mainWindowLoadedCalled, true)
        }

        function test_onSetGleanSourceTagsPassesTagsToGlean() {
            TestHelper.triggerSetGleanSourceTags("tag1,tag2")
            compare(spyTags.toString(), "tag1,tag2")
        }

        function test_onGleanEnabledChangedCallsGleanSetUploadEnabledCorrectly() {
            VPNSettings.gleanEnabled = false
            compare(spyUploadEnabled, false)

            VPNSettings.gleanEnabled = true
            compare(spyUploadEnabled, true)
        }

        function test_onAboutToQuitCallsGleanShutdown() {
            TestHelper.triggerAboutToQuit()
            compare(spyShutdownCalled, true)
        }

        function test_onInitializeGleanCallsInitializeWithCoreSettings() {
            TestHelper.triggerInitializeGlean()
            compare(spyApplicationId, "mozillavpn")
            compare(spyConfig.appBuild, "MozillaVPN/QMLTest_AppVersion")
            compare(spyConfig.appDisplayVersion, "QMLTest_AppVersion")
            // Staging mode is on in the tests
            compare(spyConfig.channel, "staging")
            // Debug is on in the tests
            compare(spyConfig.debug.logPings, true)
            compare(spyConfig.debug.debugViewTag, "MozillaVPN")
            // See note in helper.h about this approach for expected values.
            compare(spyConfig.osVersion, TestHelper.osVersion())
            compare(spyConfig.architecture, TestHelper.architecture())
        }

        function test_onInitializeGleanCallsInitializeCorrectUploadEnabled() {
            VPNSettings.gleanEnabled = false
            TestHelper.triggerInitializeGlean()
            compare(spyUploadEnabledInitialize, false)

            VPNSettings.gleanEnabled = true
            TestHelper.triggerInitializeGlean()
            compare(spyUploadEnabledInitialize, true)
        }

        /*
         * TODO - The following are qml tests that I don't know how to write
         * but I don't think they should hold up merging of this PR.
         * I have added to the integration test cases to cover our bases 
         * on these test cases.

        function test_onInitializeGleanSetsProductionChannel() {
            // I have not found a way to set stagingMode dynamically
            // in tests. This doesn't surprise me, but does mean I can't
            // test the "production" value in a unit test.

            // Setup VPN.stagingMode as false;
            TestHelper.triggerInitializeGlean()
            compare(spyConfig.channel, "production")
        }
        function test_onInitializeGleanDoesNotSetDebugParams() {
            // Similar to stagingMode, I have not found a way to set debugMode dynamically in tests.

            // Setup VPN.debugMode as false;
            TestHelper.triggerInitializeGlean()
            compare(spyConfig.debug.logPings, undefined)
            compare(spyConfig.debug.debugViewTag, undefined)
        }

        // TODO: Ideally we would also have a companion unit test for the 
        // mozillavpn method mainWindowLoaded that checks that:
        // a) mainWindowLoaded calls initializeGlean
        // b) sets up a timer
        // But we don't have a way to test mozillavpn.cpp functions yet.
        */
    }
}