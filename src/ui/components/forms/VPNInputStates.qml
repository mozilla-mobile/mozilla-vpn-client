import QtQuick 2.0
import "../../themes/colors.js" as Color

Item {
    property variant targetItem

    states: [
        State {
            name: "empty"
            when: targetItem.text.length === 0
                && targetItem.enabled
                && !targetItem.activeFocus
                && !targetItem.hasError
                && !targetItem.hovered

            PropertyChanges {
                target: targetItem
                color: Color.input.default.text
                placeholderTextColor: Color.input.default.placeholder
            }

            PropertyChanges {
                target: targetItem.background
                border.color: Color.input.default.border
                border.width: 1
            }
        },
        State {
            name: "emptyHovered"
            when: targetItem.hovered
                && targetItem.text.length === 0
                && !targetItem.activeFocus
                && !targetItem.hasError

            PropertyChanges {
                target: targetItem
                color: Color.input.hover.text
                placeholderTextColor: Color.input.hover.placeholder
            }

            PropertyChanges {
                target: targetItem.background
                border.color: Color.input.hover.border
                border.width: 1
            }
        },
        State {
            name: "focused"
            when: targetItem.activeFocus && !targetItem.hasError

            PropertyChanges {
                target: targetItem
                color: Color.input.focus.text
                placeholderTextColor: Color.input.focus.placeholder
            }

            PropertyChanges {
                target: targetItem.background
                border.color: Color.input.focus.border
                border.width: 2
            }
        },
        State {
            name: "filled"
            when: targetItem.text.length > 0 && !targetItem.hasError

            PropertyChanges {
                target: targetItem
                color: Color.input.default.text
                placeholderTextColor: Color.input.default.placeholder
            }

            PropertyChanges {
                target: targetItem.background
                border.color: Color.input.default.border
                border.width: 1
            }
        },
        State {
            name: "errorFocused"
            when: targetItem.activeFocus && targetItem.hasError

            PropertyChanges {
                target: targetItem
                color: Color.input.error.text
                placeholderTextColor: Color.input.error.placeholder
            }

            PropertyChanges {
                target: targetItem.background
                border.color: Color.input.error.border
                border.width: 2
            }
        },
        State {
            name: "errorFilled"
            when: targetItem.text.length > 0 && targetItem.hasError

            PropertyChanges {
                target: targetItem
                color: Color.input.error.text
                placeholderTextColor: Color.input.error.placeholder
            }

            PropertyChanges {
                target: targetItem.background
                border.color: Color.input.error.border
                border.width: 1
            }
        },
        State {
            name: "disabled"
            when: !targetItem.enabled

            PropertyChanges {
                target: targetItem
                color: Color.input.disabled.text
                placeholderTextColor: Color.input.disabled.placeholder
            }

            PropertyChanges {
                target: targetItem.background
                border.color: Color.input.disabled.border
                border.width: 1
            }
        }
    ]
}
