/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1

ListView {
    id: listView
    objectName: "deviceList"

    property bool isDeviceLimit: false

    signal isEditing(isEditing: bool)
    signal removeItem(name: string, publicKey: string)

    interactive: false
    Layout.fillWidth: true
    Layout.preferredHeight: childrenRect.height
    contentHeight: childrenRect.height
    spacing: 0

    onVisibleChanged: closeAllSwipes()

    removeDisplaced: Transition{
        NumberAnimation{
            property:"y"
            duration: 300
            easing.type: Easing.InOutQuad
        }
    }

    function anySwipesOpen() {
        for(let index = 1; index < listView.count; index++) {
            if(listView.itemAtIndex(index).children[0].isSwipeOpen) {
                return true
            }
        }
        return false
    }

    function allSwipesOpen() {
        for(let index = 1; index < listView.count; index++) {
            if(!listView.itemAtIndex(index).children[0].isSwipeOpen) {
                return false
            }
        }
        return true
    }

    function openAllSwipes(startingIndex: int) {
        for(let index = startingIndex; index < listView.count; index++) {
            if(listView.itemAtIndex(index).children[0] instanceof SwipeDelegate) {
               listView.itemAtIndex(index).children[0].swipe.open(SwipeDelegate.Left)
            }
        }
    }

    function closeAllSwipes() {
        for(let index = 1; index < listView.count; index++) {
            if(listView.itemAtIndex(index).children[0] instanceof SwipeDelegate) {
               listView.itemAtIndex(index).children[0].swipe.close()
            }
        }
    }

    function isEditingChanged(isEditing: bool) {
        if(isEditing) openAllSwipes(1)
        else closeAllSwipes()
    }

    model: VPNDeviceModel
    delegate: ColumnLayout {
        objectName: "deviceListLayout"
        //See https://bugreports.qt.io/browse/QTBUG-81976
        width: ListView.view.width
        spacing: 0

        VPNDeviceListItem {
            id: swipeDelegate
            objectName: "device-" + name

            Layout.fillWidth: true

            blockClose: isDeviceLimit

            onSwipeOpen: () => { if (listView.allSwipesOpen()) listView.isEditing(true) }
            onIsSwipeOpenChanged: if(!listView.anySwipesOpen()) listView.isEditing(false)
            onClicked: if (listView.anySwipesOpen() && !isDeviceLimit) listView.closeAllSwipes()

            onRemoveItem: (name, publicKey) => { listView.removeItem(name, publicKey) }


        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1

            color: MZTheme.colors.grey10
        }
    }

    Component.onCompleted: {
        if(isDeviceLimit) {
            delayTimer.start()
        }
    }

    Timer {
        id: delayTimer
        interval: 300
        onTriggered: openAllSwipes(0)
    }
}
