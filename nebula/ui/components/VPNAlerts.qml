/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.15

import Mozilla.VPN 1.0
import components 0.1

ColumnLayout {
    property bool isWasmViewer: false
    id: notifications
    spacing: VPNTheme.theme.windowMargin / 2
    Layout.maximumWidth: parent.width - VPNTheme.theme.windowMargin
    Layout.alignment: Qt.AlignHCenter
    Layout.fillHeight: false
    visible: VPNSurveyModel.hasSurvey || VPN.updateRecommended || isWasmViewer
    property var stackviewMode: StackView.Immediate
    property string updateURL: ""

    VPNAlert {
        id: updateAlert
        isLayout: true
        alertType: alertTypes.info
        visible: VPN.updateRecommended || isWasmViewer
        //% "New version is available."
        alertText: qsTrId("vpn.updates.newVersionAvailable")
        //% "Update now"
        alertActionText: qsTrId("vpn.updates.updateNow")

        onActionPressed: ()=>{
            mainStackView.push(notifications.updateURL, stackviewMode);
        }
        onClosePressed: ()=>{
             VPN.hideUpdateRecommendedAlert();
        }
    }

    VPNAlert {
        id: surveyAlert
        isLayout: true
        alertType: alertTypes.success

        //% "We’d love your feedback!"
        alertText: qsTrId("vpn.systray.survey.wouldLoveYourFeedback")
        //% "Take Survey"
        alertActionText: qsTrId("vpn.systray.survey.takeSurvey")
        visible: (VPNSurveyModel.hasSurvey && VPNSurveyModel.id !== "mozilla-vpn-survey-2022-05") /* TODO: Remove after 2.9 */ || isWasmViewer

        onActionPressed: ()=>{
            VPNSurveyModel.openCurrentSurvey();
        }
        onClosePressed: ()=>{
            VPNSurveyModel.dismissCurrentSurvey();
        }
    }

    VPNAlert {
        // This survey will be shown in English only for X number of days in
        // the 2.9 release cycle.

        // TODO: Remove in 2.10

        id: interceptSurveyAlert

        isLayout: true
        alertType: alertTypes.success
        visible: VPNSurveyModel.id === "mozilla-vpn-survey-2022-05" &&  VPNSurveyModel.hasSurvey
        alertText: "Share your VPN experience with us. Research participants receive $100."
        alertActionText: "Learn more"
        onActionPressed: ()=>VPNSurveyModel.openCurrentSurvey()
        onClosePressed: ()=>VPNSurveyModel.dismissCurrentSurvey()
    }
}
