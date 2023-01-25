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
            //% "Device-level encryption"
            headline: qsTrId("vpn.onboarding.headline.1")
        }

        ListElement {
            //: The + after the number stands for “more than”. If you change the number of countries here, please update ViewSubscriptionNeededIAP.qml too.
            //% "Servers in 30+ countries"
            headline: qsTrId("vpn.onboarding.headline.2")
        }

        ListElement {
            //% "No bandwidth restrictions"
            headline: qsTrId("vpn.onboarding.headline.3")
        }

        ListElement {
            //% "No online activity logs"
            headline: qsTrId("vpn.onboarding.headline.4")
        }
    }
}
