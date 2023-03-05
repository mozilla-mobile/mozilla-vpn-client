/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.15
import QtTest 1.0

import components 0.1

TestCase {
    id: testCase
    name: "MZMouseArea"

    property alias testComponent: testLoader.item

    Loader {
        id: testLoader

        sourceComponent: testComponent
    }

    Component {
        id: testComponent

        Button {
            id: buttonTest

            property alias mouseArea: mouseArea
            property alias spyTestButtonClicked: spyTestButtonClicked

            MZMouseArea {
                id: mouseArea
                qmlUnitTestWorkaround: true
            }

            SignalSpy {
                id: spyTestButtonClicked
                target: buttonTest
                signalName: "clicked"
            }
        }
    }


    function test_clickDoesPropagateToParentByDefault() {
        //Ensures 'propagateClickToParent' property is set to true by default
        const expected = true
        const actual = testCase.testComponent.mouseArea.propagateClickToParent
        verify(expected === actual, `propagateClickToParent was ${actual} not ${expected}.`);

        //Ensures parent gets 'clicked' event on MZMouseArea 'release' signal
        testCase.testComponent.mouseArea.released(TestEvent)
        compare(testCase.testComponent.spyTestButtonClicked.count, 1)
    }

    function test_clickDoesNotPropagateToParent() {
        //Ensures 'propagateClickToParent' property gets set to false
        testCase.testComponent.mouseArea.propagateClickToParent = false
        const expected = false
        const actual = testCase.testComponent.mouseArea.propagateClickToParent
        verify(expected === actual, `propagateClickToParent was ${actual} not ${expected}.`);

        //Ensures parent does not get `clicked' events on MZMouseArea 'release' signal
        testCase.testComponent.mouseArea.released(TestEvent)
        compare(testCase.testComponent.spyTestButtonClicked.count, 0)
    }

    function init() {
        //Reset component back to default state before each test
        testLoader.active = false
        testLoader.active = true
    }
}
