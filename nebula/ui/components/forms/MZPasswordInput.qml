/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

import Mozilla.Shared 1.0
import components 0.1
import components.forms 0.1

MZTextField {
    property bool charactersMasked: true
    property bool isValid: true
    property alias button: showHidePasswordButton

    id: passwordInput

    Accessible.passwordEdit: charactersMasked
    // In order for screen readers to respect the `Accessible.ignored` property
    // `Accessible.role` has to be set to `EditableText`. For more infos see
    // QTBUG-82433: https://bugreports.qt.io/browse/QTBUG-82433.
    Accessible.role: Accessible.EditableText

    echoMode: charactersMasked ? TextInput.Password : TextInput.Normal
    hasError: !isValid
    rightPadding: MZTheme.theme.windowMargin * 0.5 + showHidePasswordButton.width

    MZIconButton {
        id: showHidePasswordButton

        accessibleName: passwordInput.charactersMasked
                        ? MZI18n.InAppAuthShowPassword
                        : MZI18n.InAppAuthHidePassword
        anchors {
            right: parent.right
            rightMargin: MZTheme.theme.listSpacing / 2
            verticalCenter: parent.verticalCenter
        }
        height: parent.height - MZTheme.theme.listSpacing
        width: parent.height - MZTheme.theme.listSpacing
        onClicked: passwordInput.charactersMasked = !passwordInput.charactersMasked

        Image {
            anchors.centerIn: showHidePasswordButton
            fillMode: Image.PreserveAspectFit
            source: passwordInput.charactersMasked
              ? "qrc:/nebula/resources/eye-hidden.svg"
              : "qrc:/nebula/resources/eye-visible.svg"
            sourceSize.height: MZTheme.theme.iconSize * 1.5
            sourceSize.width: MZTheme.theme.iconSize * 1.5
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
