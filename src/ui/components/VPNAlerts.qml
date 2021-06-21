/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import Mozilla.VPN 1.0
import QtQuick.Layouts 1.11
import "./../components"
import "./../themes/themes.js" as Theme

ColumnLayout {
    id: notifications
    spacing: Theme.windowMargin / 2
    Layout.maximumWidth: col.width - Theme.windowMargin
    Layout.alignment: Qt.AlignHCenter
    Layout.fillHeight: false
    visible: VPNSurveyModel.hasSurvey || updateAlert.visible

    VPNAlert {
        id: updateAlert

        state: VPN.updateRecommended ? "recommended" : ""
        alertType: "update"
        alertColor: Theme.blueButton
        visible: state === "recommended"
        //% "New version is available."
        alertText: qsTrId("vpn.updates.newVersionAvailable")
        //% "Update now"
        alertLinkText: qsTrId("vpn.updates.updateNow")
        isLayout: true
    }

    VPNAlert {
        id: surveyAlert

        isLayout: true
        alertType: "survey"
        //% "We’d love your feedback!"
        alertText: qsTrId("vpn.systray.survey.wouldLoveYourFeedback")
        //% "Take Survey"
        alertLinkText: qsTrId("vpn.systray.survey.takeSurvey")
        alertColor: Theme.greenAlert
        textColor: Theme.fontColorDark
        visible: VPNSurveyModel.hasSurvey
    }

}
