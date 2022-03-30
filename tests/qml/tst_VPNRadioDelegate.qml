import QtQuick 2.15
import QtQuick.Controls 2.15
import QtTest 1.0

import Mozilla.VPN 1.0
import components 0.1

Item {
    id: testItem

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
            //CheckBox starts out unchecked
            var expected = false
            var actual = radioDelegate.checked
            verify(expected === actual, `checked was ${actual} not ${expected}.`);

            //Simulate clicking in the center of the label of the checkbox. The last parameter (2) represents the topMargin for the VPNInterLabel in VPNCheckBoxRow
            mouseClick(radioDelegate, 50, 2)
            expected = true
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
