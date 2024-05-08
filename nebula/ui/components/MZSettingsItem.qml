/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0

MZClickableRow {
    id: root

    property var settingTitle
    property alias imageLeftSrc: icon.source
    property alias imageRightSrc: imageRight.source
    property alias imageRightMirror: imageRight.mirror
    property string fontColor: MZTheme.theme.fontColorDark
    property bool _loaderVisible: false

    accessibleName: settingTitle

    anchors.left: undefined
    anchors.right: undefined

    Layout.alignment: Qt.AlignHCenter
    Layout.minimumHeight: MZTheme.theme.rowHeight
    Layout.fillWidth: true
    canGrowVertical: true
    Layout.preferredHeight: title.lineCount > 1 ? title.implicitHeight + MZTheme.theme.windowMargin : MZTheme.theme.rowHeight

    RowLayout {
        id: row
        spacing: MZTheme.theme.windowMargin
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        width: root.width - MZTheme.theme.windowMargin

        Rectangle {
            Layout.preferredHeight: MZTheme.theme.rowHeight
            Layout.preferredWidth: icon.width
            Layout.alignment: Qt.AlignTop
            color: MZTheme.theme.transparent

            MZIcon {
                id: icon
                anchors.centerIn: parent
            }
        }

        MZBoldLabel {
            id: title
            text: settingTitle
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
            Layout.fillWidth: true
            lineHeightMode: Text.FixedHeight
            lineHeight: MZTheme.theme.labelLineHeight
            wrapMode: Text.WordWrap
            topPadding: 1
            elide: Text.ElideRight
        }

        Rectangle {
            id: iconButton
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            Layout.preferredHeight: MZTheme.theme.rowHeight
            Layout.preferredWidth: MZTheme.theme.rowHeight
            color: MZTheme.theme.transparent

            MZIcon {
                id: imageRight
                fillMode: Image.PreserveAspectFit
                anchors.centerIn: iconButton
                visible: !_loaderVisible
            }

            MZButtonLoader {
                id: loader

                color: MZTheme.theme.transparent
                iconUrl: "qrc:/nebula/resources/spinner.svg"
                state: _loaderVisible ? "active" : "inactive"
            }
        }
    }
}
