/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.5
import Mozilla.VPN 1.0

VPNAlert {
    id: surveyAlertBox

    visible: false
    state: VPN.alert
    states: [
        State {
            when: !VPNSurveyModel.hasSurvey

            PropertyChanges {
                target: surveyAlertBox
                opacity: 0
                visible: false
            }

        },
        State {
            when: VPNSurveyModel.hasSurvey

            PropertyChanges {
                target: surveyAlertBox
                // TODO
                alertType: "survey"
                //% "TBD"
                alertText: qsTrId("vpn.systray.survey.title")
                //% "TBD"
                alertLinkText: qsTrId("vpn.systray.survey.message")
                opacity: 1
                visible: true
            }

        }
    ]
}
