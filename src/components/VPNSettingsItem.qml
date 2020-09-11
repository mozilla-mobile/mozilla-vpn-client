import QtQuick 2.0
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.11

import "../themes/themes.js" as Theme

RowLayout {
    property var setting
    property var imageLeftSrc
    property var imageRightSrc

    anchors.fill: parent
    anchors.leftMargin: 8
    anchors.rightMargin: 8

    VPNIconAndLabel {
        icon: imageLeftSrc
        title: setting
    }

    Item {
        Layout.fillWidth: true
    }

    VPNIcon {
        id: imageRight
        source: imageRightSrc
    }
}
