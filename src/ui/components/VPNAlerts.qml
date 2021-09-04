/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import Mozilla.VPN 1.0
import QtQuick.Layouts 1.14
import "./../components"
import "./../themes/themes.js" as Theme

ColumnLayout {
    property bool isWasmViewer: false
    id: notifications
    spacing: Theme.windowMargin / 2
    Layout.maximumWidth: parent.width - Theme.windowMargin
    Layout.alignment: Qt.AlignHCenter
    Layout.fillHeight: false
    visible: VPNSurveyModel.hasSurvey || VPN.updateRecommended || isWasmViewer

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
            stackview.push("../views/ViewUpdate.qml", StackView.Immediate);
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
        visible: VPNSurveyModel.hasSurvey || isWasmViewer

        onActionPressed: ()=>{
            VPNSurveyModel.openCurrentSurvey();
        }
        onClosePressed: ()=>{
            VPNSurveyModel.dismissCurrentSurvey();
        }
    }

}
