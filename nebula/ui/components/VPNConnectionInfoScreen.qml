import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0

Rectangle {
    property bool isVisible: false
    property bool loading: true

    id: root

    state: "hidden"
    states: [
        State {
            name: "hidden"
            when: !isVisible

            PropertyChanges {
                target: root
                opacity: 0.2
            }
        },
        State {
            name: "loading"
            when: isVisible && loading

            PropertyChanges {
                target: root
                opacity: 0.9
            }
        },
        State {
            name: "visible"
            when: isVisible && !loading

            PropertyChanges {
                target: root
                opacity: 0.9
            }
        }
    ]

    Behavior on opacity {
        NumberAnimation {
            target: root
            property: "opacity"
            duration: 1000
        }
    }

    color: VPNTheme.colors.primary
    opacity: 0.2
    width: parent.width

    ColumnLayout {
        x: 0
        y: 0

        Text {
            text: "Visble: " + (isVisible ? "true" : "false")
        }
        Text {
            text: "Loading: " + (loading ? "true" : "false")
        }
        Text {
            text: "State: " + state
        }
    }
}