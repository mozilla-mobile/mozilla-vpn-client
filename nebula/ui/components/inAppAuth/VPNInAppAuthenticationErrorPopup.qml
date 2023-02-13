/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.15

import Mozilla.VPN 1.0
import components 0.1

VPNSimplePopup {
    id: authError

    anchors.centerIn: Overlay.overlay
    imageSrc: "qrc:/ui/resources/updateRequired.svg"
    imageSize: Qt.size(80, 80)
    title: VPNI18n.InAppAuthSignInFailedPopupTitle
    description: ""
    buttons: [
        VPNButton {
            objectName: authError.objectName + "-button";
            text: VPNI18n.CaptivePortalAlertButtonTextPreActivation
            onClicked: {
                authError.close()
            }
        }
    ]

    Connections {
        target: VPNAuthInApp

        function retryAfterSecToMin(retryAfterSec) {
            if (retryAfterSec <= 0) {
                // This should not happen.
                console.log("Invalid retryAfter value", retryAfterSec);
                return 15;
            }

            return Math.ceil(retryAfterSec / 60);
        }

        function showGenericAuthError() {
            authError.description = VPNI18n.InAppSupportWorkflowSupportErrorText
            authError.open()
        }

        function onErrorOccurred(e, retryAfterSec) {
            switch(e) {
            case VPNAuthInApp.ErrorAccountAlreadyExists:
                showGenericAuthError();
                break;

            case VPNAuthInApp.ErrorUnknownAccount:
                showGenericAuthError();
                break;

            case VPNAuthInApp.ErrorEmailCanNotBeUsedToLogin:
                showGenericAuthError();
                break;

            case VPNAuthInApp.ErrorEmailTypeNotSupported:
                authError.description = VPNI18n.InAppAuthProblemEmailTypeNotSupported
                authError.open()
                break;
            case VPNAuthInApp.ErrorConnectionTimeout:
                authError.description = qsTrId("vpn.alert.noInternet")
                authError.open()
                break;
            case VPNAuthInApp.ErrorFailedToSendEmail:
                authError.description = VPNI18n.InAppAuthProblemSendingEmailErrorMessage
                authError.open()
                break;

            case VPNAuthInApp.ErrorServerUnavailable:
                showGenericAuthError();
                break;

            case VPNAuthInApp.ErrorTooManyRequests:
                const retryAfterMin = retryAfterSecToMin(retryAfterSec);
                if (retryAfterMin === 1) {
                    authError.description = VPNI18n.InAppAuthSignInBlockedForOneMinute;
                } else {
                    authError.description = VPNI18n.InAppAuthSignInFailedPopupDescription
                }
                authError.open()
                break;
            }
        }
    }
}
