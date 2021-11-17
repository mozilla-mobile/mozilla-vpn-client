import QtQuick 2.3
import QtTest 1.0

import TestHelper 1.0
import org.mozilla.Glean 0.24
import ui 0.1

Item {
    Main {
        id: mainTestGlean
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

            return {
                resolved: promiseResolved,
                data: promiseData
            }
        }

        function test_onRecordGleanEventRecordsAppropriateSample() {
            TestHelper.triggerRecordGleanEvent("authenticationAborted")
            const { data: authenticationAbortedData } = awaitOn(Sample.authenticationAborted.testGetValue());
            compare(authenticationAbortedData, "some data")
        }
        /*
        function test_onSendGleanPingsSubmitsPings() {
            compare(true, false)
        }
        */
    }
}