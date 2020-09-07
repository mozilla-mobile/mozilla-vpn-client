import QtQuick 2.0
import QtQuick.Controls 2.15
import Mozilla.VPN 1.0


import "../components"
Item {
    VPNHeadline {
        id: headline
        text: qsTr("Waiting for sign in and subscription confirmation...")
        anchors.top: parent.top
        anchors.topMargin: 32
    }

    Image {
        id: spinner
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: headline.bottom
        anchors.topMargin: 96
        sourceSize.height: 80
        fillMode: Image.PreserveAspectFit
        source: "../resources/spinner.svg"

        ParallelAnimation {
            id: startSpinning
            running: true
            PropertyAnimation {target: spinner; property: "opacity"; from: 0; to: 1; duration: 300 }
            PropertyAnimation { target: spinner; property: "scale"; from: .7; to: 1; duration: 300 }
            PropertyAnimation { target: spinner; property: "rotation"; from: 0; to: 360; duration: 8000; loops: Animation.Infinite}
        }
    }



    VPNFooterLink {
        id: getHelp
        text: qsTr("Cancel and try again")
        onClicked: VPN.cancelAuthentication()
    }

    Component.onCompleted: fade.start()
    PropertyAnimation on opacity {
        id: fade
        from: 0
        to: 1
        duration: 1000
    }
}
