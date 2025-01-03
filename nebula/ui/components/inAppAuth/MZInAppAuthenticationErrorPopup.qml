/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import components 0.1
import "qrc:/nebula/utils/MZAssetLookup.js" as MZAssetLookup

MZSimplePopup {
    id: authError

    anchors.centerIn: Overlay.overlay
    imageSrc: MZAssetLookup.getImageSource("LockWithWarning")
    imageSize: Qt.size(80, 80)
    title: MZI18n.InAppAuthSignInFailedPopupTitle
    description: ""
    buttons: [
        MZButton {
            objectName: authError.objectName + "-button";
            text: MZI18n.CaptivePortalAlertButtonTextPreActivation
            Layout.fillWidth: true
            onClicked: {
                authError.close()
            }
        }
    ]

    Connections {
        target: MZAuthInApp

        function retryAfterSecToMin(retryAfterSec) {
            if (retryAfterSec <= 0) {
                // This should not happen.
                console.log("Invalid retryAfter value", retryAfterSec);
                return 15;
            }

            return Math.ceil(retryAfterSec / 60);
        }

        function showGenericAuthError() {
            authError.description = MZI18n.InAppSupportWorkflowSupportErrorText
            authError.open()
        }

        function onErrorOccurred(e, retryAfterSec) {
            switch(e) {
            case MZAuthInApp.ErrorAccountAlreadyExists:
                showGenericAuthError();
                break;

            case MZAuthInApp.ErrorUnknownAccount:
                showGenericAuthError();
                break;

            case MZAuthInApp.ErrorEmailCanNotBeUsedToLogin:
                showGenericAuthError();
                break;

            case MZAuthInApp.ErrorEmailTypeNotSupported:
                authError.description = MZI18n.InAppAuthProblemEmailTypeNotSupported
                authError.open()
                break;
            case MZAuthInApp.ErrorConnectionTimeout:
                authError.description = MZI18n.GlobalNoInternetConnection
                authError.open()
                break;
            case MZAuthInApp.ErrorFailedToSendEmail:
                authError.description = MZI18n.InAppAuthProblemSendingEmailErrorMessage
                authError.open()
                break;

            case MZAuthInApp.ErrorServerUnavailable:
                showGenericAuthError();
                break;

            case MZAuthInApp.ErrorTooManyRequests:
                const retryAfterMin = retryAfterSecToMin(retryAfterSec);
                if (retryAfterMin === 1) {
                    authError.description = MZI18n.InAppAuthSignInBlockedForOneMinute;
                } else {
                    authError.description = MZI18n.InAppAuthSignInFailedPopupDescription
                }
                authError.open()
                break;
            }
        }
    }
}
