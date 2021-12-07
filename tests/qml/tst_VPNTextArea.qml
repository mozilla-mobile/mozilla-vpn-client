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
            vpnTextAreaTest.textInput.text = "a".repeat(vpnTextAreaTest.textInput.maxCharacterCount);
        }

        function test_insertingMultipleCharsSmallerThanLimitDoesNotCauseTruncationOfText() {
            resetTextArea();

            const expected = "abc";
            insertMultipleChars(expected);

            const actual = vpnTextAreaTest.textInput.text;
            verify(expected === actual, `text was ${actual} not ${expected}.`);
        }

        function test_insertingSingleCharExceedingLimitIsRemovedFromText() {
            resetTextArea();
            fillTextInput();

            const expected = vpnTextAreaTest.textInput.text;
            insertSingleChar("b");

            const actual = vpnTextAreaTest.textInput.text;
            verify(expected === actual, `text was ${actual} not ${expected}.`);
        }

        function test_insertingCharsSmallerThanLimitUpdatesCursorPosition() {
            resetTextArea();

            const text = "abc";
            insertMultipleChars(text);

            const expected = text.length;
            const actual = vpnTextAreaTest.textInput.cursorPosition;

            verify(expected === actual, `Cursor position was ${actual} not ${expected}.`);
        }

        function test_resetToInitialCursorPositionAfterCharIsRemovedFromText() {
            resetTextArea();
            fillTextInput();

            const expected = Math.floor(vpnTextAreaTest.textInput.maxCharacterCount / 2);
            vpnTextAreaTest.textInput.cursorPosition = expected;
            insertSingleChar("b");
            const actual = vpnTextAreaTest.textInput.cursorPosition;

            verify(expected === actual, `Cursor position was ${actual} not ${expected}.`);
        }

    }
}
