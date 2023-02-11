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
    property alias button: showHidePasswordButton

    id: passwordInput

    Accessible.ignored: charactersMasked
    Accessible.passwordEdit: charactersMasked
    // In order for screen readers to respect the `Accessible.ignored` property
    // `Accessible.role` has to be set to `EditableText`. For more infos see
    // QTBUG-82433: https://bugreports.qt.io/browse/QTBUG-82433.
    Accessible.role: Accessible.EditableText

    echoMode: charactersMasked ? TextInput.Password : TextInput.Normal
    hasError: !isValid
    rightPadding: VPNTheme.theme.windowMargin * 0.5 + showHidePasswordButton.width

    VPNIconButton {
        id: showHidePasswordButton

        accessibleName: passwordInput.charactersMasked
                        ? VPNI18n.InAppAuthShowPassword
                        : VPNI18n.InAppAuthHidePassword
        anchors {
            right: parent.right
            rightMargin: VPNTheme.theme.listSpacing / 2
            verticalCenter: parent.verticalCenter
        }
        height: parent.height - VPNTheme.theme.listSpacing
        width: parent.height - VPNTheme.theme.listSpacing
        onClicked: passwordInput.charactersMasked = !passwordInput.charactersMasked

        Image {
            anchors.centerIn: showHidePasswordButton
            fillMode: Image.PreserveAspectFit
            source: passwordInput.charactersMasked
              ? "qrc:/nebula/resources/eye-hidden.svg"
              : "qrc:/nebula/resources/eye-visible.svg"
            sourceSize.height: VPNTheme.theme.iconSize * 1.5
            sourceSize.width: VPNTheme.theme.iconSize * 1.5
        }

        // workaround for QTBUG-78813: TextInput prevents touch events
        // from reaching other MouseAreas.
        // https://bugreports.qt.io/browse/QTBUG-78813
        MouseArea {
            onPressed: showHidePasswordButton.clicked()
            anchors.fill: parent
            enabled: ["android", "ios"].includes(Qt.platform.os)
        }
    }
}
