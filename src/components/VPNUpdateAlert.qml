import QtQuick 2.0

import Mozilla.VPN 1.0

Rectangle {
    id: alertUpdateRecommendedBox
    visible: VPN.updateRecommended

    color: "#0060DF"
    height: 40
    width: parent.width - 16
    y: 10
    x: 8
    radius: 4
    anchors.margins: 8

    Text {
        id: alertUpdateRecommendedText
        font.pixelSize: 13
        color: "#FFFFFF"
        anchors.centerIn: alertUpdateRecommendedBox
        text: qsTr("New version is available. <u><b>Upgrade now</b></ul>")
    }

    MouseArea {
        anchors.fill: alertUpdateRecommendedText
        onClicked: mainStackView.push("../views/ViewUpdate.qml")
    }

    Image {
        id: alertUpdateRecommendedBoxClose
        source: "../resources/close-white.svg"
        sourceSize.width: 12
        sourceSize.height: 12
        anchors.right: alertUpdateRecommendedBox.right
        anchors.top: alertUpdateRecommendedBox.top
        anchors.topMargin: 14
        anchors.rightMargin: 14
    }

    MouseArea {
        anchors.fill: alertUpdateRecommendedBoxClose
        onClicked: VPN.hideUpdateRecommendedAlert()
    }
}
