/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.14

import Mozilla.VPN 1.0


StackView {
    id: stackView

    Component.onCompleted: function(){
        if(!currentItem && initialItem) {
            // We don't show anything right now and inital item is set, 
            // On android if initialItem is anything But a component
            // it will totaly parse that into garbage values and fail
            // 
            // See https://github.com/mozilla-mobile/mozilla-vpn-client/pull/2638
            console.error("Using the initialItem property does not work on some platforms. Use Component.onCompleted: stackview.push(someURI)");
            VPN.exitForUnrecoverableError("Setting initialItem on a StackView is illegal. See previous logs for more information.");

        }

    }

    Connections {
        target: VPNNavigator
        function onGoBack(item) {
            if (item === stackView) {
                stackView.pop();
            }
        }
    }
}
