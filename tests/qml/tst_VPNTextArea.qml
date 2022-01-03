/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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

            const expected = "a".repeat(vpnTextAreaTest.textInput.maxCharacterCount);
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

        function test_textLengthMatchesUnicodesLenghtInJsAndQml() {
            resetTextArea();

            const expected = "🙋🏾".repeat(vpnTextAreaTest.textInput.maxCharacterCount);
            vpnTextAreaTest.textInput.text = expected;

            const actual = vpnTextAreaTest.textInput.text;

            verify(expected.length === actual.length, `text length was ${actual} not ${expected}.`);
        }

    }
}
