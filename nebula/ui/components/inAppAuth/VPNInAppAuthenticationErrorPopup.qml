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
    width: Math.min(parent.width - VPNTheme.theme.windowMargin * 2, VPNTheme.theme.maxHorizontalContentWidth)
    height: Math.min(parent.height - VPNTheme.theme.windowMargin * 2, implicitHeight + VPNTheme.theme.windowMargin * 2)
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

            VPNHeadline {
                text: VPNl18n.InAppAuthSignInFailedPopupTitle
                width: undefined
                Layout.fillWidth: true
            }

            VPNTextBlock {
                id: authErrorMessage
                text: ""
                horizontalAlignment: Text.AlignHCenter
                Layout.preferredWidth: parent.width
                Layout.fillWidth: true
            }

            VPNButton {
                text: VPNl18n.CaptivePortalAlertButtonTextPreActivation
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

        function showGenericAuthError() {
            authErrorMessage.text = VPNl18n.InAppSupportWorkflowSupportErrorText
            openErrorModalAndForceFocus();
        }

        function onErrorOccurred(e, retryAfterSec) {
            switch(e) {
            case VPNAuthInApp.ErrorAccountAlreadyExists:
                showGenericAuthError();
                break;

            case VPNAuthInApp.ErrorAccountUnknown:
                showGenericAuthError();
                break;

            case VPNAuthInApp.ErrorEmailCanNotBeUsedToLogin:
                showGenericAuthError();
                break;

            case VPNAuthInApp.ErrorEmailTypeNotSupported:
                authErrorMessage.text = VPNl18n.InAppAuthProblemEmailTypeNotSupported
                openErrorModalAndForceFocus();
                break;

            case VPNAuthInApp.ErrorFailedToSendEmail:
                authErrorMessage.text =VPNl18n.InAppAuthProblemSendingEmailErrorMessage
                openErrorModalAndForceFocus();
                break;

            case VPNAuthInApp.ErrorServerUnavailable:
                showGenericAuthError();
                break;

            case VPNAuthInApp.ErrorTooManyRequests:
                const retryAfterMin = retryAfterSecToMin(retryAfterSec);
                if (retryAfterMin === 1) {
                    authErrorMessage.text = VPNl18n.InAppAuthSignInBlockedForOneMinute;
                } else {
                    authErrorMessage.text = VPNl18n.InAppAuthSignInFailedPopupDescription
                }
                openErrorModalAndForceFocus();
                break;
            }
        }
    }
}
