/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1

MZScreenBase {
    objectName: "settings"
    _menuIconVisibility: getStack().depth > 1

    Component.onCompleted: () => {
        MZNavigator.addStackView(VPN.ScreenSettings, getStack())
        getStack().push("qrc:/qt/qml/Mozilla/VPN/screens/settings/ViewSettingsMenu.qml")
        maybeOpenObfuscationSettings()
    }

    onVisibleChanged: {
        if (visible) {
            maybeOpenObfuscationSettings()
        }
    }

    // Show obfuscation settings when requested from the server unavailable popup
    function maybeOpenObfuscationSettings() {
        if (!window.pendingShowObfuscationSettings) {
            return;
        }
        window.pendingShowObfuscationSettings = false;

        if (getStack().currentItem.objectName === "obfuscationSettingsView") {
            return;
        }

        getStack().push("qrc:/qt/qml/Mozilla/VPN/screens/settings/privacy/ViewPrivacy.qml");
        getStack().push("qrc:/qt/qml/Mozilla/VPN/screens/settings/privacy/obfuscation/ViewObfuscation.qml");
    }
}
