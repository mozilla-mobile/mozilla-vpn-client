import QtQuick 2.0
import "../../themes/colors.js" as Color

Item {
    property Item container;

    states: [
        State {
            name: "empty"
            when: container.text.length === 0
                && container.enabled
                && !container.activeFocus
                && !container.hasError
                && !container.hovered

            PropertyChanges {
                target: textField
                color: Color.input.default.text
                placeholderTextColor: Color.input.default.placeholder
            }

            PropertyChanges {
                target: container.background
                border.color: Color.input.default.border
                border.width: 1
            }
        },
        State {
            name: "emptyHovered"
            when: container.hovered
                && container.text.length === 0
                && !container.activeFocus
                && !container.hasError

            PropertyChanges {
                target: textField
                color: Color.input.hover.text
                placeholderTextColor: Color.input.hover.placeholder
            }

            PropertyChanges {
                target: container.background
                border.color: Color.input.hover.border
                border.width: 1
            }
        },
        State {
            name: "focused"
            when: container.activeFocus && !container.hasError

            PropertyChanges {
                target: textField
                color: Color.input.focus.text
                placeholderTextColor: Color.input.focus.placeholder
            }

            PropertyChanges {
                target: container.background
                border.color: Color.input.focus.border
                border.width: 2
            }
        },
        State {
            name: "filled"
            when: container.text.length > 0 && !container.hasError

            PropertyChanges {
                target: textField
                color: Color.input.default.text
                placeholderTextColor: Color.input.default.placeholder
            }

            PropertyChanges {
                target: container.background
                border.color: Color.input.default.border
                border.width: 1
            }
        },
        State {
            name: "errorFocused"
            when: container.activeFocus && container.hasError

            PropertyChanges {
                target: textField
                color: Color.input.error.text
                placeholderTextColor: Color.input.error.placeholder
            }

            PropertyChanges {
                target: container.background
                border.color: Color.input.error.border
                border.width: 2
            }
        },
        State {
            name: "errorFilled"
            when: container.text.length > 0 && container.hasError

            PropertyChanges {
                target: textField
                color: Color.input.error.text
                placeholderTextColor: Color.input.error.placeholder
            }

            PropertyChanges {
                target: container.background
                border.color: Color.input.error.border
                border.width: 1
            }
        },
        State {
            name: "disabled"
            when: !container.enabled

            PropertyChanges {
                target: textField
                color: Color.input.disabled.text
                placeholderTextColor: Color.input.disabled.placeholder
            }

            PropertyChanges {
                target: container.background
                border.color: Color.input.disabled.border
                border.width: 1
            }
        }
    ]
}
