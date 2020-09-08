import QtQuick 2.0
import QtGraphicalEffects 1.0

import Mozilla.VPN 1.0

import "../components"
import "../themes/themes.js" as Theme

Item {
    VPNControllerView {
        id: box
    }

    DropShadow {
        anchors.fill: box
        horizontalOffset: 0
        verticalOffset: 1
        radius: 4
        color: "#0C0C0D1E"
        source: box
    }

    VPNControllerServer {
        y: box.y + box.height + Theme.iconSize
    }

    VPNControllerDevice {
        y: box.y + box.height + Theme.iconSize + (Theme.vSpacing * 2)
    }
}
