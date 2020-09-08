import QtQuick 2.0
import QtQuick.Layouts 1.0

import "../themes/themes.js" as Theme

// VPNChevron
Image {
    source: "../resources/chevron.svg"
    Layout.leftMargin: Theme.iconSize
    Layout.rightMargin: Theme.hSpacing
    Layout.preferredWidth: 8
    fillMode: Image.PreserveAspectFit
}
