/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.14

import Mozilla.VPN 1.0

Item {
    property variant itemToTarget

    states: [
        State {
            name: "empty"
            when: itemToTarget.text === ""
                && itemToTarget.enabled
                && !itemToTarget.activeFocus
                && !itemToTarget.hasError
                && !itemToTarget.hovered

            PropertyChanges {
                target: itemToTarget
                color: VPNTheme.colors.input.default.text
                placeholderTextColor: VPNTheme.colors.input.default.placeholder
            }

            PropertyChanges {
                target: itemToTarget.background
                border.color: VPNTheme.colors.input.default.border
                border.width: 1
            }
        },
        State {
            name: "emptyHovered"
            when: itemToTarget.hovered
                && itemToTarget.text === ""
                && !itemToTarget.activeFocus
                && !itemToTarget.hasError

            PropertyChanges {
                target: itemToTarget
                color: VPNTheme.colors.input.hover.text
                placeholderTextColor: VPNTheme.colors.input.hover.placeholder
            }

            PropertyChanges {
                target: itemToTarget.background
                border.color: VPNTheme.colors.input.hover.border
                border.width: 1
            }
        },
        State {
            name: "focused"
            when: itemToTarget.activeFocus && !itemToTarget.hasError

            PropertyChanges {
                target: itemToTarget
                color: VPNTheme.colors.input.focus.text
                placeholderTextColor: VPNTheme.colors.input.focus.placeholder
            }

            PropertyChanges {
                target: itemToTarget.background
                border.color: VPNTheme.colors.input.focus.border
                border.width: 2
            }
        },
        State {
            name: "filled"
            when: (itemToTarget.text && itemToTarget.text.length > 0) && !itemToTarget.hasError

            PropertyChanges {
                target: itemToTarget
                color: VPNTheme.colors.input.default.text
                placeholderTextColor: VPNTheme.colors.input.default.placeholder
            }

            PropertyChanges {
                target: itemToTarget.background
                border.color: VPNTheme.colors.input.default.border
                border.width: 1
            }
        },
        State {
            name: "errorFocused"
            when: itemToTarget.activeFocus && itemToTarget.hasError

            PropertyChanges {
                target: itemToTarget
                color: VPNTheme.colors.input.error.text
                placeholderTextColor: VPNTheme.colors.input.error.placeholder
            }

            PropertyChanges {
                target: itemToTarget.background
                border.color: VPNTheme.colors.input.error.border
                border.width: 2
            }
        },
        State {
            name: "errorFilled"
            when: (itemToTarget.text && itemToTarget.text.length > 0) && itemToTarget.hasError

            PropertyChanges {
                target: itemToTarget
                color: VPNTheme.colors.input.error.text
                placeholderTextColor: VPNTheme.colors.input.error.placeholder
            }

            PropertyChanges {
                target: itemToTarget.background
                border.color: VPNTheme.colors.input.error.border
                border.width: 1
            }
        },
        State {
            name: "disabled"
            when: !itemToTarget.enabled

            PropertyChanges {
                target: itemToTarget
                color: VPNTheme.colors.input.disabled.text
                placeholderTextColor: VPNTheme.colors.input.disabled.placeholder
            }

            PropertyChanges {
                target: itemToTarget.background
                border.color: VPNTheme.colors.input.disabled.border
                border.width: 1
            }
        }
    ]
}
