import QtQuick 2.3
import QtTest 1.0

import components.forms 0.1

Item {
    width: 600;
    height: 800;

    VPNTextArea {
        id: vpnTextAreaTest
    }

    TestCase {
        name: "VPNTextArea"
        when: windowShown

        function resetTextArea() {
            vpnTextAreaTest.textInput.text = "";
            vpnTextAreaTest.textInput.forceActiveFocus();
        }

        function insertSingleChar(character) {
            keyClick(character);
        }

        function insertMultipleChars(chars) {
            [...chars].forEach(character => {
                insertSingleChar(character);
            });
        }

        function fillTextInput() {
            const maxCharLength = vpnTextAreaTest.textInput.maxCharacterCount;
            const initialString = "a".repeat(maxCharLength);
            vpnTextAreaTest.textInput.text = initialString;
        }

        // Insert single char within limit
        function test_insertSingleChar() {
            resetTextArea();

            const expected = "a";
            insertSingleChar(expected);

            const actual = vpnTextAreaTest.textInput.text;
            verify(expected === actual, `text was ${actual} not ${expected}.`);
        }

        // Insert multiple chars within limit
        function test_insertMultipleChars() {
            resetTextArea();

            const expected = "abc";
            insertMultipleChars(expected);

            const actual = vpnTextAreaTest.textInput.text;
            verify(expected === actual, `text was ${actual} not ${expected}.`);
        }

        // Insert single char exceeding limit
        function test_exceedsCharLimitSingle() {
            resetTextArea();
            fillTextInput();

            const expected = vpnTextAreaTest.textInput.text;
            const inputString = "b";
            insertSingleChar(inputString);

            const actual = vpnTextAreaTest.textInput.text;
            verify(expected === actual, `text was ${actual} not ${expected}.`);
        }

        // Insert multiple char exceeding limit
        function test_exceedsCharLimitMultiple() {
            resetTextArea();
            fillTextInput();

            const expected = vpnTextAreaTest.textInput.text;
            const inputString = "abc";
            insertMultipleChars(inputString);

            const actual = vpnTextAreaTest.textInput.text;
            verify(expected === actual, `text was ${actual} not ${expected}.`);
        }

    }
}
