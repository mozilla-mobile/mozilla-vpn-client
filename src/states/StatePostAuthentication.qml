import QtQuick 2.0
import QtQuick.Controls 2.15
import Mozilla.VPN 1.0


import "../components"
Item {
    VPNHeadline {
        id: headline
        text: qsTr("Quick access")
        anchors.top: parent.top
        anchors.topMargin: 32
    }

    VPNButton {
        width: 282
        text: qsTr("Continue")
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        radius: 5
        onClicked: VPN.postAuthenticationCompleted()
    }

    Component.onCompleted: fade.start()
    PropertyAnimation on opacity {
        id: fade
        from: 0
        to: 1
        duration: 1000
    }
}
