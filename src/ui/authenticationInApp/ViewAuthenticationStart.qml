/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../components"
import "../components/forms"
import "../themes/themes.js" as Theme


Item {
    property var uiState: Theme.uiState
    opacity: 0

    Behavior on opacity {
        PropertyAnimation {
            to: 1
            duration: 200
        }
    }

    // TODO
    // This view shows the authentication form. We need to collect an email
    // address and a password.
    // At this point we do not know if we are going to sign-in or sign-up.
    // The next step is to call VPNAuthInApp::sigInOrUp();

    Component.onCompleted: {
        opacity = 1;
        emailInput.forceActiveFocus();
        console.log("START");
    }


    ColumnLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        spacing: Theme.windowMargin


        VPNHeadline {
            text: "Enter your email"
            Layout.bottomMargin: Theme.windowMargin
        }

        VPNTextField {
            id: emailInput
            Layout.fillWidth: true
            placeholderText: "Enter email" // TODO
            onTextChanged: stateError = false
            errorMessageText: "Enter valid email.... TODO"
            Keys.onReturnPressed: btn.state = uiState.statePressed
            Keys.onReleased: {
                if (event.key === Qt.Key_Return) {
                    btn.clicked();
                }
            }

            function validEmail() {
                const email = emailInput.text;
                const emailValidator = /^(([^<>()[\]\.,;:\s@\"]+(\.[^<>()[\]\.,;:\s@\"]+)*)|(\".+\"))@(([^<>()[\]\.,;:\s@\"]+\.)+[^<>()[\]\.,;:\s@\"]{2,})$/i;
                return emailValidator.test(email);
            }
        }

        VPNButton {
            id: btn
            text: "Continue" // TODO
            Layout.fillWidth: true
            onClicked: {
                if (emailInput.validEmail()) {
                    return VPNAuthInApp.checkAccount(emailInput.text);
                }

                return emailInput.stateError = true;
            }
        }

    }



}
