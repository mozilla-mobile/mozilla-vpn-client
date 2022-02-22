import QtQuick 2.3
import QtTest 1.0

import TestHelper 1.0
import Mozilla.VPN 1.0
import org.mozilla.Glean 0.30
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
            compare(spyConfig.osVersion, VPN.osVersion)
            compare(spyConfig.architecture, VPN.architecture)
        }

        function test_onInitializeGleanCallsInitializeCorrectUploadEnabled() {
            VPNSettings.gleanEnabled = false
            TestHelper.triggerInitializeGlean()
            compare(spyUploadEnabledInitialize, false)

            VPNSettings.gleanEnabled = true
            TestHelper.triggerInitializeGlean()
            compare(spyUploadEnabledInitialize, true)
        }

        function test_onInitializeGleanByStagingMode() {
            TestHelper.stagingMode = false
            TestHelper.triggerInitializeGlean()
            compare(spyConfig.channel, "production")

            TestHelper.stagingMode = true
            TestHelper.triggerInitializeGlean()
            compare(spyConfig.channel, "staging")
        }

        function test_onInitializeGleanByDebugMode() {
            TestHelper.debugMode = false
            TestHelper.triggerInitializeGlean()
            compare(spyConfig.debug.logPings, undefined)
            compare(spyConfig.debug.debugViewTag, undefined)

            TestHelper.debugMode = true
            TestHelper.triggerInitializeGlean()
            compare(spyConfig.debug.logPings, true)
            compare(spyConfig.debug.debugViewTag, "MozillaVPN")
        }

        /*
         * TODO - We should also have a companion unit test for the 
         * mozillavpn method mainWindowLoaded that checks that:
         * a) mainWindowLoaded calls initializeGlean
         * b) sets up a timer
         *
         * But we don't have a way to test mozillavpn.cpp functions yet.
         * I have added to the integration test cases to cover our bases 
         * on these test cases.
        */
    }
}