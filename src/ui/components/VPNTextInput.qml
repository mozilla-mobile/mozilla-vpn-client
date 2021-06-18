/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.12
import Mozilla.VPN 1.0
import "../themes/themes.js" as Theme


Item {
    id: inputRoot
    property var isEnabled: true
    property var valueChanged: ()=>{}
    property var value: ""
    property var valueInvalid: false
    property var leftPadding: 0
    //% "The entered value is invalid"
    //: Associated to an inputfield for a setting
    property var error: qsTrId("vpn.settings.inputValue.invalid")

    property var innerBorderColor: Theme.fontColorDark
    property var innerBorderSize: 1
    property var textColor: Theme.fontColorDark
    property var outerBorderOpacity: 0.00

    states: [
        State {
            name: "active"
            when: isEnabled && !valueInvalid
            PropertyChanges {
                target: inputRoot
                innerBorderColor: Theme.fontColorDark
                textColor: Theme.fontColorDark
                innerBorderSize: 1
                outerBorderOpacity:0.00
            }
        },
        State{
            name: "error"
            when: isEnabled && valueInvalid
            PropertyChanges {
                target: inputRoot
                innerBorderColor: Theme.red
                textColor: Theme.red
                innerBorderSize: 2
                outerBorderOpacity: 0.66
            }
        },
        State{
            name: "focus"
            when: isEnabled && !valueInvalid && input.activeFocus
            PropertyChanges {
                target: inputRoot
                innerBorderColor: Theme.blue
                textColor: Theme.fontColorDark
                innerBorderSize: 2
                outerBorderOpacity: 0.66
            }
        }
    ]

    transitions: Transition {
        NumberAnimation  { properties: "outerBorderOpacity"; easing.type: Easing.InOutQuad }
    }
    Item{
        id: holder
        anchors.top: inputRoot.top
        anchors.left: inputRoot.left
        anchors.leftMargin: leftPadding
        width: inputRoot.width - leftPadding
        height: Theme.rowHeight+10
        Rectangle{
            radius: Theme.cornerRadius
            opacity: outerBorderOpacity
            anchors.verticalCenter: textInputWrapper.verticalCenter
            anchors.horizontalCenter: textInputWrapper.horizontalCenter
            width: textInputWrapper.width+ 5
            height: textInputWrapper.height+ 5
            visible: true
            color: "#E22850"
            z:1
        }
        Rectangle{
            id: textInputWrapper
            width: parent.width
            height: Theme.rowHeight
            color: Theme.white
            radius: Theme.cornerRadius

            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter

            border.color: innerBorderColor
            border.width: 1
            z:2

            TextInput
            {
                id: input
                width: parent.width
                z:3
                cursorVisible: focus
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.left: parent.left
                anchors.leftMargin:16
                text: value
                color: "#3D3D3D"
                onTextChanged: ()=>{
                    valueChanged(this.text)
                }
                enabled: isEnabled
            }
        }



    }
    VPNCheckBoxAlert {
        anchors.top: holder.bottom
        anchors.topMargin: Theme.windowMargin
        anchors.left: holder.left

        width: holder.width
        visible: valueInvalid
        leftMargin: 0
        errorMessage: error
    }
}
