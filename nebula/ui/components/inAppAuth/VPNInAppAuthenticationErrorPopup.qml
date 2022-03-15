/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1

VPNPopup {
    id: authError
    anchors.centerIn: parent
    width: Math.min(parent.width * 0.73, VPNTheme.theme.maxHorizontalContentWidth)
    maxWidth: Math.min(parent.width * 0.83, VPNTheme.theme.maxHorizontalContentWidth)
    _popupContent:
        ColumnLayout {
            id: authErrorContent
            spacing: VPNTheme.theme.vSpacing

                Image {
                    source: "qrc:/ui/resources/updateRequired.svg"
                    antialiasing: true
                    sourceSize.height: 80
                    sourceSize.width: 80
                    Layout.alignment: Qt.AlignHCenter
                }
            VPNTextBlock {
                id: authErrorMessage
                text: ""
                horizontalAlignment: Text.AlignHCenter
                Layout.preferredWidth: parent.width
                Layout.fillWidth: true
                width: undefined
            }

            VPNButton {
                text: "Okay"
                onClicked: authError.close()
            }
        }


    Connections {
        target: VPNAuthInApp

        function openErrorModalAndForceFocus() {
            authError.open();
            authErrorContent.forceActiveFocus();
        }

        function retryAfterSecToMin(retryAfterSec) {
            if (retryAfterSec <= 0) {
                // This should not happen.
                console.log("Invalid retryAfter value", retryAfterSec);
                return 15;
            }

            return Math.ceil(retryAfterSec / 60);
        }

        function onErrorOccurred(e, retryAfterSec) {
            switch(e) {
            case VPNAuthInApp.ErrorEmailCanNotBeUsedToLogin:
                authErrorMessage.text = "email can not be used to log in";
                openErrorModalAndForceFocus();
                break;

            case VPNAuthInApp.ErrorEmailTypeNotSupported:
                authErrorMessage.text = "email type not supported";
                openErrorModalAndForceFocus();
                break;

            case VPNAuthInApp.ErrorFailedToSendEmail:
                authErrorMessage.text = "Error - failed to send email"
                openErrorModalAndForceFocus();
                break;

            case VPNAuthInApp.ErrorTooManyRequests:
                const retryAfterMin = retryAfterSecToMin(retryAfterSec);
                if (retryAfterMin === 1) {
                    authErrorMessage.text = "Too many login attempts, hold off for just one minute";
                } else {
                    authErrorMessage.text = "Too many login attempts, hold off for " + retryAfterMin + " minutes";
                }
                openErrorModalAndForceFocus();
                break;
            }
        }
    }
}
