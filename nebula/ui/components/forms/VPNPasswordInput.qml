/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1

VPNTextField {
    property bool charactersMasked: true
    property bool isValid: true
    property alias button: toggleButton
    property alias placeholder: passwordInput._placeholderText

    id: passwordInput
    echoMode: charactersMasked ? TextInput.Password : TextInput.Normal
    hasError: !isValid
    height: VPNTheme.theme.rowHeight
    width: parent.width

    VPNIconButton {
        id: toggleButton

        // TODO: Add accesibleName string
        accessibleName: ""
        anchors {
            right: parent.right
            rightMargin: VPNTheme.theme.listSpacing / 2
            verticalCenter: parent.verticalCenter
        }
        height: parent.height - VPNTheme.theme.listSpacing
        width: parent.height - VPNTheme.theme.listSpacing
        onClicked: {
            toggleVisibility();
        }

        Image {
            anchors.centerIn: toggleButton
            fillMode: Image.PreserveAspectFit
            source: passwordInput.charactersMasked
              ? "qrc:/nebula/resources/eye-hidden.svg"
              : "qrc:/nebula/resources/eye-visible.svg"
            sourceSize.height: VPNTheme.theme.iconSize * 1.5
            sourceSize.width: VPNTheme.theme.iconSize * 1.5
        }

        function toggleVisibility() {
            passwordInput.charactersMasked = !passwordInput.charactersMasked
        }
    }
}
