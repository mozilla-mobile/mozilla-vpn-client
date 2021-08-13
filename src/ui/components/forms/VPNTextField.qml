/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../../themes/themes.js" as Theme
import "../../themes/colors.js" as Color
import "./../../components"

TextField {
    // TODO Add strings for Accessible.description, Accessible.name
    property bool hasError: false
    property bool showInteractionStates: true
    property bool forceBlurOnOutsidePress: true

    id: textField

    background: VPNInputBackground {
        id: textFieldBackground
    }
    color: Color.input.default.text
    inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhSensitiveData
    onActiveFocusChanged: if (focus && typeof(vpnFlickable) !== "undefined" && vpnFlickable.ensureVisible) vpnFlickable.ensureVisible(textField)
    selectByMouse: true
    Layout.preferredHeight: Theme.rowHeight

    VPNInputStates {
        id: textFieldState
        itemToTarget: textField
    }
}
