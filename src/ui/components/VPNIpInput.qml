import QtQuick 2.0
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.15
import Mozilla.VPN 1.0
import "../themes/themes.js" as Theme


ColumnLayout {
    property var labelText :"Heading"
    property var subLabelText
    property var isEnabled: true

    property var valueChanged: ()=>{}
    property var value: ""
    property var valueInavlid: false

    VPNInterLabel {
        id: label
        Layout.alignment: Qt.AlignLeft
        Layout.fillWidth: true
        text: labelText
        color: Theme.fontColorDark
        horizontalAlignment: Text.AlignLeft
    }

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
          inputMask:  "000.000.000.000; "
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

        //% "The entered value is invalid"
        //: Associated to an inputfield for a setting
        errorMessage: qsTrId("vpn.settings.inputValue.invalid")
    }
}
