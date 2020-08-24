import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.11

import "../themes/themes.js" as Theme

// BoldLabel
Label {
    font.pixelSize: Theme.fontSize
    font.family: vpnFont.name
    font.weight: Font.Bold
    height: 22
    color: Theme.fontColorDark
}
