/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.3
import QtTest 1.0

import components.forms 0.1

Item {
    width: 600;
    height: 800;

    VPNPasswordInput {
        id: vpnPasswordInputTest
    }

    TestCase {
        name: "VPNPasswordInput"
        when: windowShown
        
        // Display options for text inputs.
        // https://doc.qt.io/qt-6/qml-qtquick-textinput.html#echoMode-prop
        // - TextInput.Normal
        // - TextInput.NoEcho
        // - TextInput.Password
        // - TextInput.PasswordEchoOnEdit

        function test_hasMaskedCharactersByDefault() {
            const expected = TextInput.Password;
            const actual = vpnPasswordInputTest.echoMode;

            verify(expected === actual, `echoMode was ${actual} not ${expected}.`);
        }

        function test_revealingMaskedCharactersWorks() {
            const expected = TextInput.Normal;
            mouseClick(vpnPasswordInputTest.button);
            const actual = vpnPasswordInputTest.echoMode;

            verify(expected === actual, `echoMode was ${actual} not ${expected}.`);
        }

        function test_hidingMaskedCharactersWorks() {
            const expected = TextInput.Password;
            mouseClick(vpnPasswordInputTest.button);
            mouseClick(vpnPasswordInputTest.button);
            const actual = vpnPasswordInputTest.echoMode;

            verify(expected === actual, `echoMode was ${actual} not ${expected}.`);
        }

        // This function is called after each test function that is executed
        // and resets the password input to its initial state
        function resetPasswordInput() {
            vpnPasswordInputTest.charactersMasked = true;
        }
        
        function cleanup() {
            resetPasswordInput();
        }
    }
}
