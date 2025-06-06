/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import "qrc:/nebula/utils/MZAssetLookup.js" as MZAssetLookup

MZSwipeDelegate {
    id: swipeDelegate

    signal removeItem(name: string, publicKey: string)

    hasUiStates: false
    activeFocusOnTab: false

    //% "%1 %2"
    //: Example: "deviceName deviceDescription"
    Accessible.name: qsTrId("vpn.devices.deviceAccessibleName").arg(name).arg(deviceSubtitle())
    Accessible.role: Accessible.ListItem
    Accessible.ignored: !visible

    function deviceSubtitle() {
        if (currentOne) {
            return MZI18n.DevicesListItemDescriptionCurrent
        }

        const diff = (Date.now() - createdAt.valueOf()) / 1000;
        if (diff < 3600) {
            return MZI18n.DevicesListItemDescriptionAddedOneHourAgo
        }

        if (diff < 86400) {
            return MZI18n.DevicesListItemDescriptionAddedFewHoursAgo.arg(Math.floor(diff / 3600))
        }

        //: Note: there is currently no support for proper plurals
        return MZI18n.DevicesListItemDescriptionAddedDaysAgo.arg(Math.floor(diff / 86400))
    }

    content.sourceComponent: Column {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        spacing: 0

        Item {
            height: 16
            anchors.left: parent.left
            anchors.right: parent.right
        }

        RowLayout {
            id: layout
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: MZTheme.theme.windowMargin
            anchors.rightMargin: MZTheme.theme.windowMargin

            spacing: 8

            MZIcon {
                Layout.alignment: Qt.AlignTop | Qt.AlignLeft

                source: MZAssetLookup.getImageSource("IconDevices")
                fillMode: Image.PreserveAspectFit
            }

            ColumnLayout {
                spacing: 0

                MZInterLabel {
                    Layout.fillWidth: true

                    color: MZTheme.colors.fontColorDark
                    text: name
                    maximumLineCount: 1
                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignLeft
                }

                MZTextBlock {
                    Layout.fillWidth: true

                    text: deviceSubtitle()
                    maximumLineCount: 1
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }

        Item {
            height: 16
            anchors.left: parent.left
            anchors.right: parent.right
        }
    }

    swipe.left: Loader {
        objectName: "swipeActionLoader"
        active: !currentOne
        height: parent.height
        sourceComponent: MZSwipeAction {
            objectName: "swipeActionDelete"

            activeFocusOnTab: swipeDelegate.isSwipeOpen
            bgColor: MZTheme.colors.destructiveButton.defaultColor
            content: Image {
                anchors.centerIn: parent
                source: MZAssetLookup.getImageSource("TrashCanWhite")
            }

            SwipeDelegate.onClicked: removeItem(name, publicKey)

            Accessible.name: MZI18n.DevicesListItemRemoveDeviceAccessibility.arg(name)
        }
    }
}
