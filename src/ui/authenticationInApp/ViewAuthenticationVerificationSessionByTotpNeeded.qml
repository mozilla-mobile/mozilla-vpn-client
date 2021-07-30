/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../components"
import "../components/forms"

Item {
    // TODO
    // we are here if the user is completing the session activation flow.
    // The TOTP code is required.
    //
    // After this step we can go to:
    // - all done \o/
    // - error
    //
    // Note that "all done" means IAP! So, more likely, you will receive
    // errors, crashes, a lot of fun things... nothing will work, and more
    // likely the app will implode.
    // In theory, this part should be tested on iOS or on Android when we will
    // have IAP there too.

    Component.onCompleted: console.log("SESSION VERIFICATION BY TOTP")

    ColumnLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: Theme.vSpacing
        anchors.rightMargin: Theme.vSpacing
        anchors.verticalCenter: parent.verticalCenter
        spacing: Theme.windowMargin

        VPNHeadline {
            text: "TOTP verification needed. Code:"

        }

        VPNTextBlock {
            text: "Enter 6-digit code"
            horizontalAlignment: Text.AlignHCenter
            Layout.bottomMargin: Theme.vSpacing
            Layout.alignment: Qt.AlignHCenter

        }
        RowLayout {
            id: inputRow
            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: Theme.windowMargin
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
                        border.color: itemToFocus.activeFocus && showInteractionStates ? showError ? Theme.red : Theme.input.focusBorder : Theme.white                    }
                }
            }
        }

        VPNButton {
            property var code: ["0", "0", "0", "0", "0", "0"]
            id: codeButton
            text: "Verify" // TODO
            Layout.fillWidth: true
            onClicked: VPNAuthInApp.verifySessionTotpCode(codeButton.code.join(''));
        }

    }
}
