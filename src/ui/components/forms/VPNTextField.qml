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
    property bool isPassword: false
    property bool charactersMasked: isPassword

    id: textField

    background: VPNInputBackground {
        id: textFieldBackground
    }
    color: Color.input.default.text
    echoMode: charactersMasked ? TextInput.Password : TextInput.Normal
    inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhSensitiveData
    onActiveFocusChanged: if (focus && typeof(vpnFlickable) !== "undefined" && vpnFlickable.ensureVisible) vpnFlickable.ensureVisible(textField)
    selectByMouse: true
    Layout.preferredHeight: Theme.rowHeight

    VPNIconButton {
        id: iconButton

        accessibleName: "Toggle password visibility" // TODO: Add localized string
        anchors {
            right: parent.right
            rightMargin: Theme.listSpacing / 2
            verticalCenter: parent.verticalCenter
        }
        height: parent.height - Theme.listSpacing
        visible: isPassword
        width: parent.height - Theme.listSpacing
        onClicked: {
            charactersMasked = !charactersMasked
        }

        Image {
            id: backImage

            anchors.centerIn: iconButton
            fillMode: Image.PreserveAspectFit
            source: charactersMasked ? "../../resources/faces/good.svg" : "../../resources/faces/poor.svg" // TODO: Replace with correct icons when available
            sourceSize.height: Theme.iconSize
            sourceSize.width: Theme.iconSize
        }
    }

    VPNInputStates {
        id: textFieldState
        itemToTarget: textField
    }
}
