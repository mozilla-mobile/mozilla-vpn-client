/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

import Mozilla.VPN 1.0

QtObject {
    id: statusBarModifier

    enum StatusBarTextColor {
        Light,
        Dark
    }
    property int statusBarTextColor: VPNMobileStatusBarModifier.StatusBarTextColor.Dark

    function applyChanges() {
        VPN.setStatusBarTextColor(statusBarModifier.statusBarTextColor);
    }

    function resetDefaults() {
        VPN.setStatusBarTextColor(VPNMobileStatusBarModifier.StatusBarTextColor.Dark);
    }

    Component.onCompleted: applyChanges()
    Component.onDestruction: resetDefaults()
}
