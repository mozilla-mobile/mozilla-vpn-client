/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14

import Mozilla.VPN 1.0
import components 0.1

Item {
    /* This view exists to avoid creating redundant work for localization as
     * the strings defined below are still in use in ViewSubscriptionNeededIAP.qml */

    ListModel {
        ListElement {
            image: "qrc:/ui/resources/onboarding/onboarding1.svg"
            //% "Device-level encryption"
            headline: qsTrId("vpn.onboarding.headline.1")
            //% "Encrypt your traffic so that it can’t be read by your ISP or eavesdroppers."
            subtitle: qsTrId("vpn.onboarding.subtitle.1")
        }

        ListElement {
            image: "qrc:/ui/resources/onboarding/onboarding2.svg"
            //: The + after the number stands for “more than”. If you change the number of countries here, please update ViewSubscriptionNeededIAP.qml too.
            //% "Servers in 30+ countries"
            headline: qsTrId("vpn.onboarding.headline.2")
            //% "Pick a server in any country you want and hide your location to throw off trackers."
            subtitle: qsTrId("vpn.onboarding.subtitle.2")
        }

        ListElement {
            image: "qrc:/ui/resources/onboarding/onboarding3.svg"
            //% "No bandwidth restrictions"
            headline: qsTrId("vpn.onboarding.headline.3")
            //% "Stream, download, and game without limits, monthly caps or ISP throttling."
            subtitle: qsTrId("vpn.onboarding.subtitle.3")
        }

        ListElement {
            image: "qrc:/ui/resources/onboarding/onboarding4.svg"
            //% "No online activity logs"
            headline: qsTrId("vpn.onboarding.headline.4")
            //% "We are committed to not monitoring or logging your browsing or network history."
            subtitle: qsTrId("vpn.onboarding.subtitle.4")
        }
    }
}
