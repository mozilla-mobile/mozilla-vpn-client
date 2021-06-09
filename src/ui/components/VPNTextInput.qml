import QtQuick 2.0
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.12
import Mozilla.VPN 1.0
import "../themes/themes.js" as Theme


ColumnLayout {
    property var isEnabled: true

    property var valueChanged: ()=>{}
    property var value: ""
    property var valueInavlid: false
    //% "The entered value is invalid"
    //: Associated to an inputfield for a setting
    property var error: qsTrId("vpn.settings.inputValue.invalid")

    Rectangle{
        width: parent.width
        height: Theme.rowHeight
        color: Theme.white
        radius: Theme.cornerRadius

        TextInput
        {
          id: input
          width: parent.width
          cursorVisible: focus
          anchors.verticalCenter: parent.verticalCenter
          anchors.horizontalCenter: parent.horizontalCenter
          anchors.left: parent.left
          anchors.leftMargin: Theme.vSpacing
          text: value
          onTextChanged: ()=>{
            valueChanged(this.text)
          }
          enabled: isEnabled
        }
    }
    VPNCheckBoxAlert {
        width: parent.width
        visible: valueInavlid
        leftMargin: 0
        errorMessage: error
    }
}
