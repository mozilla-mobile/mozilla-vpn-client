/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../components"
import "../components/forms"

import "../themes/themes.js" as Theme

Item {
    // TODO
    // If we are here, we were trying to complete the authentication flow using
    // an existing account. That account has not been verified yet. The user
    // needs to insert the 6-digit code.
    // After this step, call `verifyEmailCode()` with the code. If the code is
    // not valid, an error will be signaled.
    // The next steps are:
    // - Sign-in again.
    // - errors.
    // TODO: resend the verify-email-code. (please keep this TODO!)

    Component.onCompleted: console.log("EMAIL VERIFICATION")

    ColumnLayout {
        id: col
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: -Theme.vSpacing
        spacing: Theme.windowMargin

        VPNHeadline {
            id: msg
            text: "Email verification needed. Code:"
            Layout.fillWidth: true
        }

        RowLayout {
            id: inputRow
            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: Theme.windowMargin
            Layout.fillWidth: true
            spacing: 4
            Repeater {
                model: 6
                delegate: VPNTextField {
                    id: index

                    Layout.preferredHeight: Theme.rowHeight
                    Layout.preferredWidth: Theme.rowHeight
                    leftPadding: 16
                    validator:  RegExpValidator {
                        regExp: /[0-9]{1}/
                    }
                    onTextChanged: {
                        if (length === 1) {
                            codeButton.code[modelData] = text
                            nextItemInFocusChain().forceActiveFocus();
                        }
                    }

                    background: VPNInputBackground {
                        border.color: itemToFocus.activeFocus && showInteractionStates ? showError ? Theme.red : Theme.input.focusBorder : Theme.greyHovered                   }
                }
            }
        }

        VPNButton {
            property var code: ["0", "0", "0", "0", "0", "0"]
            id: codeButton
            text: "Verify" // TODO
            onClicked: VPNAuthInApp.verifyEmailCode(codeButton.code.join());
            Layout.fillWidth: true
        }

    }

}
