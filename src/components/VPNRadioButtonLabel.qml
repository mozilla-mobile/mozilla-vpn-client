import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.11

import "../themes/themes.js" as Theme

// VPNRadioButtonLabel
Label {
    anchors.left: radioButton.right
    anchors.leftMargin: Theme.hSpacing

    text: modelData
    font.family: vpnFont.name
    font.pointSize: Theme.fontSize
    color: Theme.fontColor

    states: State {
        when: control.checked
        PropertyChanges {
            target: cityName
            color: Theme.buttonColor
        }
    }

    transitions: Transition {
        ColorAnimation {
            target: cityName
            properties: "color"
            duration: 100
        }
    }
}
