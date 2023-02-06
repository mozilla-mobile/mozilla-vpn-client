/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.3
import QtTest 1.0

import TestHelper 1.0
import org.mozilla.Glean 0.30
import telemetry 0.30
import ui 0.1

Item {
    Main {
        id: gleanEvents
    }
    
    TestCase {
        name: "TestGleanEvents"

        /**
         * Taken from: https://github.com/mozilla/glean.js/blob/main/samples/qt/src/Tests/tst_maintests.qml
         * 
         * This function will block until a given promise is fulfilled.
         *
         * Note: This needs to be inside a TestCase element,
         * because `tryVerify` is a test only API.
         */
        function awaitOn(promise) {
            // Whether or not the promise fulfilled, doesn't matter if resolved or rejected.
            let promiseFulfilled = false;
            // Whether or not the promise resolved succesfully.
            let promiseResolved = false;
            // The result of the resolved promise;
            let promiseData = undefined;

            promise
                .then(data => {
                    promiseResolved = true;
                    promiseFulfilled = true;
                    promiseData = data;
                })
                .catch(() => promiseFulfilled = true);

            // This will block waiting for the promise to fulfill.
            tryVerify(
                () => promiseFulfilled,
                5000,
                `Timed out while waiting for promise to fulfill.`
            );

            return promiseData;
        }

        function test_onRecordGleanEventRecordsAppropriateSample() {
            awaitOn(Glean.testResetGlean("mozillavpn", true));
            TestHelper.triggerRecordGleanEvent("settingsAboutUsOpened");
            const authenticationAbortedData = awaitOn(Sample.settingsAboutUsOpened.testGetValue());
            compare(authenticationAbortedData.length, 1);
            compare(authenticationAbortedData[0]["category"], "sample");
            compare(authenticationAbortedData[0]["name"], "settings_about_us_opened");
        }

        function test_onSendGleanPingsSubmitsPings() {
            awaitOn(Glean.testResetGlean("mozillavpn", true));
            // First we have no events
            let aboutUsData = awaitOn(Sample.settingsAboutUsOpened.testGetValue());
            compare(aboutUsData, undefined);

            // Then we have one event
            TestHelper.triggerRecordGleanEvent("settingsAboutUsOpened");
            aboutUsData = awaitOn(Sample.settingsAboutUsOpened.testGetValue());
            compare(aboutUsData.length, 1);
            
            // Then we send the ping which should result in us having no events again
            TestHelper.triggerSendGleanPings();
            aboutUsData = awaitOn(Sample.settingsAboutUsOpened.testGetValue());
            compare(aboutUsData, undefined);
        }

        function test_AboutUsEvent() {
            awaitOn(Glean.testResetGlean("mozillavpn", true));
            TestHelper.triggerRecordGleanEvent("settingsAboutUsOpened");
            let eventData = awaitOn(Sample.settingsAboutUsOpened.testGetValue());
            compare(eventData.length, 1);
        }

        function test_SpeedTestCompletedEvent() {
            awaitOn(Glean.testResetGlean("mozillavpn", true));
            TestHelper.triggerRecordGleanEventWithExtraKeys("speedTestCompleted", {"speed": "Fast"});
            let eventData = awaitOn(Sample.speedTestCompleted.testGetValue());
            compare(eventData.length, 1);
        }

        function test_BottomNavigationBarClickEvent() {
            awaitOn(Glean.testResetGlean("mozillavpn", true));
            TestHelper.triggerRecordGleanEventWithExtraKeys("bottomNavigationBarClick", {"bar_button": "ScreenMessaging"});
            let eventData = awaitOn(Sample.bottomNavigationBarClick.testGetValue());
            compare(eventData.length, 1);
        }

        function test_AuthenticationErrorEvent() {
            awaitOn(Glean.testResetGlean("mozillavpn", true));
            TestHelper.triggerRecordGleanEventWithExtraKeys("authenticationError", {"reason": "InvalidTOTP"});
            let eventData = awaitOn(Sample.authenticationError.testGetValue());
            compare(eventData.length, 1);
        }

        function test_UserChangedEndpointGeoEvent() {
            awaitOn(Glean.testResetGlean("mozillavpn", true));
            TestHelper.triggerRecordGleanEventWithExtraKeys("userChangedEndpointGeo", {"server": "multiHopExitServer"});
            let eventData = awaitOn(Sample.userChangedEndpointGeo.testGetValue());
            compare(eventData.length, 1);
        }

        function test_HelpMenuHelpCenterOpenedEvent() {
            awaitOn(Glean.testResetGlean("mozillavpn", true));
            TestHelper.triggerRecordGleanEvent("helpMenuHelpCenterOpened");
            let eventData = awaitOn(Sample.helpMenuHelpCenterOpened.testGetValue());
            compare(eventData.length, 1);
        }

        function test_HelpContactSupportOpenedEvent() {
            awaitOn(Glean.testResetGlean("mozillavpn", true));
            TestHelper.triggerRecordGleanEvent("helpContactSupportOpened");
            let eventData = awaitOn(Sample.helpContactSupportOpened.testGetValue());
            compare(eventData.length, 1);
        }

        function test_SupportCaseSubmittedEvent() {
            awaitOn(Glean.testResetGlean("mozillavpn", true));
            TestHelper.triggerRecordGleanEvent("supportCaseSubmitted");
            let eventData = awaitOn(Sample.supportCaseSubmitted.testGetValue());
            compare(eventData.length, 1);
        }

        function test_HelpMenuViewLogsOpenedEvent() {
            awaitOn(Glean.testResetGlean("mozillavpn", true));
            TestHelper.triggerRecordGleanEvent("helpMenuViewLogsOpened");
            let eventData = awaitOn(Sample.helpMenuViewLogsOpened.testGetValue());
            compare(eventData.length, 1);
        }
    }
}
