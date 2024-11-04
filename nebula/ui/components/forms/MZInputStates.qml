/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0

import Mozilla.Shared 1.0

Item {
    property variant itemToTarget

    states: [
        State {
            name: "disabled"
            when: !itemToTarget.enabled

            PropertyChanges {
                target: itemToTarget
                color: MZTheme.colors.inputState.disabled.text
                placeholderTextColor: MZTheme.colors.inputState.disabled.placeholder
            }

            PropertyChanges {
                target: itemToTarget.background
                border.color: MZTheme.colors.inputState.disabled.border
                border.width: 1
            }
        },
        State {
            name: "empty"
            when: itemToTarget.text === ""
                && itemToTarget.enabled
                && !itemToTarget.activeFocus
                && !itemToTarget.hasError
                && !itemToTarget.hovered

            PropertyChanges {
                target: itemToTarget
                color: MZTheme.colors.inputState.default.text
                placeholderTextColor: MZTheme.colors.inputState.default.placeholder
            }

            PropertyChanges {
                target: itemToTarget.background
                border.color: MZTheme.colors.inputState.default.border
                border.width: 1
            }
        },
        State {
            name: "emptyHovered"
            when: itemToTarget.hovered && itemToTarget.enabled
                && itemToTarget.text === ""
                && !itemToTarget.activeFocus
                && !itemToTarget.hasError

            PropertyChanges {
                target: itemToTarget
                color: MZTheme.colors.inputState.hover.text
                placeholderTextColor: MZTheme.colors.inputState.hover.placeholder
            }

            PropertyChanges {
                target: itemToTarget.background
                border.color: MZTheme.colors.inputState.hover.border
                border.width: 1
            }
        },
        State {
            name: "focused"
            when: itemToTarget.activeFocus && !itemToTarget.hasError

            PropertyChanges {
                target: itemToTarget
                color: MZTheme.colors.inputState.focus.text
                placeholderTextColor: MZTheme.colors.inputState.focus.placeholder
            }

            PropertyChanges {
                target: itemToTarget.background
                border.color: MZTheme.colors.inputState.focus.border
                border.width: 2
            }
        },
        State {
            name: "filled"
            when: (itemToTarget.text && itemToTarget.text.length > 0) && !itemToTarget.hasError

            PropertyChanges {
                target: itemToTarget
                color: MZTheme.colors.inputState.default.text
                placeholderTextColor: MZTheme.colors.inputState.default.placeholder
            }

            PropertyChanges {
                target: itemToTarget.background
                border.color: MZTheme.colors.inputState.default.border
                border.width: 1
            }
        },
        State {
            name: "errorFocused"
            when: itemToTarget.activeFocus && itemToTarget.hasError

            PropertyChanges {
                target: itemToTarget
                color: MZTheme.colors.inputState.error.text
                placeholderTextColor: MZTheme.colors.inputState.error.placeholder
            }

            PropertyChanges {
                target: itemToTarget.background
                border.color: MZTheme.colors.inputState.error.border
                border.width: 2
            }
        },
        State {
            name: "errorFilled"
            when: (itemToTarget.text && itemToTarget.text.length > 0) && itemToTarget.hasError

            PropertyChanges {
                target: itemToTarget
                color: MZTheme.colors.inputState.error.text
                placeholderTextColor: MZTheme.colors.inputState.error.placeholder
            }

            PropertyChanges {
                target: itemToTarget.background
                border.color: MZTheme.colors.inputState.error.border
                border.width: 1
            }
        }
    ]
}
