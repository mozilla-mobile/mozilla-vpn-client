/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.3
import QtTest 1.0

import TestHelper 1.0
import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import org.mozilla.Glean 0.30
import ui 0.1

Item {
    Main {
        id: mainTest
    }

    TestCase {
        name: "MainTestsGleanMocks"
        when: windowShown

        property var spyApplicationId
        property var spyUploadEnabledInitialize
        property var spyConfig
        property var spyTags
        property var spyLogPings
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
            function mockGleanSetLogPings(flag) {
                spyLogPings = flag
            }
            Glean.setLogPings = mockGleanSetLogPings;
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
            MZSettings.gleanEnabled = false
            compare(spyUploadEnabled, false)

            MZSettings.gleanEnabled = true
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
            compare(spyConfig.osVersion, MZEnv.osVersion)
            compare(spyConfig.architecture, [MZEnv.architecture, MZEnv.graphicsApi].join(" ").trim())
        }

        function test_onInitializeGleanCallsInitializeCorrectUploadEnabled() {
            MZSettings.gleanEnabled = false
            TestHelper.triggerInitializeGlean()
            compare(spyUploadEnabledInitialize, false)

            MZSettings.gleanEnabled = true
            TestHelper.triggerInitializeGlean()
            compare(spyUploadEnabledInitialize, true)
        }

        function test_onInitializeGleanByStagingMode() {
            MZEnv.stagingMode = false
            TestHelper.triggerInitializeGlean()
            compare(spyConfig.channel, "production")

            MZEnv.stagingMode = true
            TestHelper.triggerInitializeGlean()
            compare(spyConfig.channel, "staging")
        }

        function test_onInitializeGleanByDebugMode() {
            MZEnv.debugMode = false
            TestHelper.triggerInitializeGlean()
            compare(spyLogPings, undefined)

            MZEnv.debugMode = true
            TestHelper.triggerInitializeGlean()
            compare(spyLogPings, true)
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
