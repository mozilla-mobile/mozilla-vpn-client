import QtQuick 2.3
import QtTest 1.0

import TestHelper 1.0
import org.mozilla.Glean 0.30
import telemetry 0.30
import ui 0.1

Item {
    Main {
        id: mainTest
    }
    
    TestCase {
        name: "MainTestsGleanEvents"

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
            TestHelper.triggerRecordGleanEvent("authenticationAborted");
            const authenticationAbortedData = awaitOn(Sample.authenticationAborted.testGetValue());
            compare(authenticationAbortedData.length, 1);
            compare(authenticationAbortedData[0]["category"], "sample");
            compare(authenticationAbortedData[0]["name"], "authentication_aborted");
        }

        function test_onSendGleanPingsSubmitsPings() {
            awaitOn(Glean.testResetGlean("mozillavpn", true));
            // First we have no events
            let authenticationAbortedData = awaitOn(Sample.authenticationAborted.testGetValue());
            compare(authenticationAbortedData, undefined);

            // Then we have one event
            TestHelper.triggerRecordGleanEvent("authenticationAborted");
            authenticationAbortedData = awaitOn(Sample.authenticationAborted.testGetValue());
            compare(authenticationAbortedData.length, 1);
            
            // Then we send the ping which should result in us having no events again
            TestHelper.triggerSendGleanPings();
            authenticationAbortedData = awaitOn(Sample.authenticationAborted.testGetValue());
            compare(authenticationAbortedData, undefined);
        }
    }
}