import QtQuick 2.0
import QtQuick.Controls 2.5
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.11

import "../themes/themes.js" as Theme

// VPNIconAndLabel
RowLayout {
    property alias icon: icon.source
    property alias title: title.text

    spacing: 0
    height: parent.height

    VPNIcon {
        id: icon
        Layout.alignment: Qt.AlignHCenter
    }

    VPNBoldLabel {
        id: title
        Layout.leftMargin: 14
        Layout.alignment: Qt.AlignVCenter
    }
}
