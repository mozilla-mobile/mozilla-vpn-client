import QtQuick 2.0

Rectangle {
    id: listHeader
    width: deviceList.width
    color: "transparent"

    Rectangle {
        id: deviceLimitAlert
        anchors.fill: parent
        anchors.topMargin: 32
        color: "transparent"

        Image {
            id: alertImg
            source: "../resources/devicesLimit.svg"
            sourceSize.width: 80
            fillMode: Image.PreserveAspectFit
            anchors.horizontalCenter: parent.horizontalCenter
        }

        VPNHeadline {
            id: alertHeadline
            anchors.top: alertImg.bottom
            anchors.topMargin: 24
            text: qsTr("Remove a device")
        }

        VPNSubtitle {
            anchors.top: alertHeadline.bottom
            anchors.topMargin: 8
            text: qsTr("You've reached your limit. To install the VPN on this device, you'll need to remove one.")
        }
    }
}
