import QtQuick 2.0
import QtQuick.Controls 2.15
import Mozilla.VPN 1.0

Item {
    Text {
        id: getHelp
        color: "#0a68e3"
        text: qsTr("Get help")
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: 16
        anchors.rightMargin: 16
        MouseArea {
            onClicked: VPN.openLink("getHelp")
            cursorShape: Qt.PointingHandCursor
            anchors.fill: parent
            hoverEnabled: true
        }
    }

    Image {
        id: logo
        x: 100
        y: 70
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        source: "resources/logo.png"
    }

    Text {
        id: logoTitle
        x: 168
        text: qsTr("Mozilla VPN")
        font.family: vpnFont.name
        horizontalAlignment: Text.AlignHCenter
        anchors.top: parent.top
        anchors.topMargin: 205
        anchors.horizontalCenterOffset: 1
        anchors.horizontalCenter: logo.horizontalCenter
        font.pixelSize: 18
    }

    Text {
        id: logoSubtitle
        x: 169
        y: 255
        text: qsTr("A fast, secure and easy to use VPN.")
        anchors.horizontalCenter: parent.horizontalCenter
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 12
        font.family: vpnFont.name
    }

    Text {
        id: logoSubtitle2
        x: 169
        y: 276
        text: qsTr("BUilt by the makers of Firefox.")
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        font.pixelSize: 12
        font.family: vpnFont.name
    }

    RoundButton {
        id: getStarted
        x: 130
        y: 347
        width: 282
        height: 40
        text: qsTr("Get started")
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        font.weight: Font.ExtraLight
        enabled: true
        focusPolicy: Qt.NoFocus
        radius: 5
        onClicked: VPN.authenticate()
    }


    Text {
        id: learnMore
        y: 422
        color: "#0a68e3"
        text: qsTr("Learn more")
        anchors.horizontalCenter: parent.horizontalCenter

        MouseArea {
            onClicked: VPN.openLink("learnMore")
            cursorShape: Qt.PointingHandCursor
            anchors.fill: parent
            hoverEnabled: true
        }
    }
}
