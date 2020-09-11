import QtQuick 2.0
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.11
import Mozilla.VPN 1.0

import "../themes/themes.js" as Theme

// VPNExternalLinkListItem

VPNClickableRow {
    property alias title: title.text
    backgroundColor: Theme.externalLinkRow

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: Theme.windowMargin / 2
        anchors.rightMargin: Theme.windowMargin / 2

        VPNBoldLabel {
            id: title
        }

        Item {
            Layout.fillWidth: true
        }

        VPNIcon {
            source: "../resources/externalLink.svg"
        }
    }
}
