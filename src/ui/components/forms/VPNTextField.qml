/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../../themes/themes.js" as Theme
import "../../themes/colors.js" as Colors
import "./../../components"

TextField {
    // TODO Add strings for Accessible.description, Accessible.name
    property bool hasError: false
    property bool loseFocusOnBlur: true

    id: textField

    state: ""
    states: [
        State {
            name: "hover"
            when: hovered && !activeFocus

            PropertyChanges {
                target: textField
                color: Colors.input.hover.text
                placeholderTextColor: Colors.input.hover.placeholder
            }
        },
        State {
            name: "focus"
            when: activeFocus && !hasError

            PropertyChanges {
                target: textField
                color: Colors.input.focus.text
                placeholderTextColor: Colors.input.focus.placeholder
            }
        },
        State {
            name: "error"
            when: hasError && activeFocus

            PropertyChanges {
                target: textField
                color: Colors.input.error.text
                placeholderTextColor: Colors.input.error.placeholder
            }
        },
        State {
            name: "disabled"
            when: !enabled

            PropertyChanges {
                target: textField
                color: Colors.input.disabled.text
                placeholderTextColor: Colors.input.disabled.placeholder
            }
        }
    ]
    background: VPNInputBackground {
        showError: textField.state === "error"
    }
    color: Colors.input.default.text
    placeholderTextColor: Colors.input.default.placeholder

    inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhSensitiveData
    onActiveFocusChanged: if (focus && vpnFlickable.ensureVisible) vpnFlickable.ensureVisible(textField)
    selectByMouse: true
    Layout.preferredHeight: Theme.rowHeight

    Text {
        text: textField.state
        y: 35
    }
}
