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

            VPNCheckBoxRow {
                id: checkBoxRowTest

                property alias checkBoxRowTest: checkBoxRowTest
                property bool _isChecked: false

                labelText: "Test text"
                isChecked: _isChecked
                onClicked: {
                    _isChecked = !_isChecked
                }
            }
    }

    TestCase {
        name: "VPNCheckBoxRow"
        when: windowShown

        function test_clickingCheckBoxLabelTogglesCheckedState() {
            var checkboxrow = testItem.testComponent.checkBoxRowTest
            //CheckBox starts out unchecked
            var expected = false
            var actual = checkboxrow.isChecked
            verify(expected === actual, `isChecked was ${actual} not ${expected}.`);

            //Simulate clicking in the center of the label of the checkbox. The last parameter (2) represents the topMargin for the VPNInterLabel in VPNCheckBoxRow
            mouseClick(checkboxrow, checkboxrow.width - checkboxrow.labelWidth / 2, 2)
            expected = true
            actual = checkboxrow.isChecked
            verify(expected === actual, `isChecked was ${actual} not ${expected}.`);

            //Doing the same steps again to uncheck the checkbox
            mouseClick(checkboxrow, checkboxrow.width - checkboxrow.labelWidth  / 2, 2)
            expected = false
            actual = checkboxrow.isChecked
            verify(expected === actual, `isChecked was ${actual} not ${expected}.`);
        }

        function init() {
            //Reset component back to default state before each test
            testLoader.active = false
            testLoader.active = true
        }
    }
}
