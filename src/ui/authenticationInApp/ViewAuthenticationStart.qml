/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import components.inAppAuth 0.1

VPNInAppAuthenticationBase {

    // TODO
    // This is step 1 in the authentication/account creation flow.
    // We need to collect an email address.
    // At this point we do not know if we are going to sign-in or sign-up.
    // On submit, user is routed to password entry or creation based on account status

    // TODOs (likely there are more)
    // Add final content when available and images
    // Form interaction polish
    // Show form error messages

    id: viewAuthenticationStart

    _menuButtonOnClick: () => {VPN.cancelAuthentication() }
    _menuButtonImageSource: "qrc:/nebula/resources/back.svg"
    _menuButtonAccessibleName: "Back"
    _headlineText: "Mozilla VPN"
    _subtitleText: "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut"
    _imgSource: "qrc:/ui/resources/logo.svg"
    _inputLabel: "Email address"

    _inputs: VPNInAppAuthenticationInputs {
        _buttonEnabled: VPNAuthInApp.state === VPNAuthInApp.StateStart && activeInput().text.length !== 0 && !activeInput().hasError
        _buttonOnClicked: (inputText) => {
                              if (!VPNAuthInApp.validateEmailAddress(inputText)) {
                                  activeInput().hasError = true;
                                  _inputErrorMessage = "invalid email address, try again"
                                  return activeInput().forceActiveFocus();
                              }
                              VPNAuthInApp.checkAccount(inputText);
                          }
        _buttonText: "Continue"
        _inputMethodHints: Qt.ImhEmailCharactersOnly | Qt.ImhNoAutoUppercase
        _inputPlaceholderText: "Enter email"
        _itemToPan: viewAuthenticationStart
    }

    _disclaimers: RowLayout {
        spacing: VPNTheme.theme.vSpacing / 2
        Rectangle {
            Layout.alignment: Qt.AlignTop
            Layout.topMargin: 2
            height: VPNTheme.theme.vSpacing
            width: VPNTheme.theme.vSpacing
            color: "transparent"

            VPNIcon {
                source: "qrc:/ui/resources/logo.svg"
                anchors.centerIn: parent
            }
        }

        VPNTextBlock {
            id: txt
            text: "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua."
            Layout.fillWidth: true
        }
    }

}
