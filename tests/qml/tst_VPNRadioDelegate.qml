/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtTest 1.0

import Mozilla.VPN 1.0
import components 0.1

Item {
    id: testItem
    height: 100
    width: 200

    property alias testComponent: testLoader.item

    Loader {
        id: testLoader

        sourceComponent: testComponent
    }

    Component {
        id: testComponent

        Item {

            property alias radioDelegateTest: radioDelegateTest

            width: 100
            height: 5

            ButtonGroup {
                id: radioButtonGroup
            }

            VPNRadioDelegate {
                id: radioDelegateTest

                property bool _isChecked: false

                radioButtonLabelText: "Test text"
                checked: _isChecked
                onClicked: {
                    _isChecked = !_isChecked
                }
            }
        }
    }

    TestCase {
        name: "VPNRadioDelegate"
        when: windowShown

        function test_clickingRadioDelegateLabelSelectsIt() {
            var radioDelegate = testItem.testComponent.radioDelegateTest
            //Radio delegate starts out unselected
            var expected = false
            var actual = radioDelegate.checked
            verify(expected === actual, `checked was ${actual} not ${expected}.`);

            //Simulate clicking the label of the radio delegate.
            mouseClick(radioDelegate, 50, 0)
            expected = true
            actual = radioDelegate.checked
            verify(expected === actual, `checked was ${actual} not ${expected}.`);
        }

        function test_clickingRadioDelegateLabelTrailingSpaceDoesNotSelectIt() {
            var radioDelegate = testItem.testComponent.radioDelegateTest
            //Radio delegate starts out unselected
            var expected = false
            var actual = radioDelegate.checked
            verify(expected === actual, `checked was ${actual} not ${expected}.`);

            //Simulate clicking the trailing space after the label of the radio delegate.
            mouseClick(radioDelegate, testItem.width, 0)
            expected = false
            actual = radioDelegate.checked
            verify(expected === actual, `checked was ${actual} not ${expected}.`);
        }

        function init() {
            //Reset component back to default state before each test
            testLoader.active = false
            testLoader.active = true
        }
    }
}
