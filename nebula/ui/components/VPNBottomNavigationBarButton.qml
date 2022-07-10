import QtQuick 2.0

import Mozilla.VPN 1.0

VPNIconButton {
    id: root

    property alias source: image.source
    property bool hasNotification: true

    height: 48
    width: 48
    backgroundRadius: height / 2
    buttonColorScheme: VPNTheme.theme.iconButtonDarkBackground
    uiStatesVisible: !checked

    onClicked: {
        checked = true
    }

    Rectangle {
        id: circleBackground

        anchors.fill: parent

        visible: checked
        radius: parent.height / 2
        opacity: .2
        color: VPNTheme.theme.white
    }

    Image {
        id: image
        anchors.centerIn: parent
        sourceSize.height: 32
        sourceSize.width: 32
    }
}
