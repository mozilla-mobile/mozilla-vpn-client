/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1
import Mozilla.VPN.qmlcomponents 1.0

VPNViewBase {
    id: vpnFlickable
    objectName: "messageInboxView"

    property bool isEmptyState
    property bool isEditing: false
    property var dismissedMessages: []
    property Component rightMenuButton: Component {
        VPNLinkButton {
            id: editLink

            property bool isEditing: false

            horizontalPadding: VPNTheme.theme.hSpacing / 5
            enabled: !isEmptyState
            labelText: !vpnFlickable.isEditing || isEmptyState ? VPNl18n.InAppMessagingEditButton : VPNl18n.InAppSupportWorkflowSupportResponseButton
            onClicked: {
                vpnFlickable.isEditing = !vpnFlickable.isEditing
            }
        }
    }

    signal editModeChanged

    //Weird workaround to fix VPN-2895
    onIsEditingChanged: editModeChanged()

    _menuTitle: VPNl18n.InAppMessagingMenuTitle

    onVisibleChanged: if (!visible) resetPage()

    function resetPage() {
        vpnFlickable.isEditing = false
        closeAllSwipes()
    }

    function anySwipesOpen() {
        for(let index = 0; index < listView.count; index++) {
            if(listView.itemAtIndex(index).children[0].isSwipeOpen) {
                return true
            }
        }
        return false
    }

    function allSwipesOpen() {
        for(let index = 0; index < listView.count; index++) {
            if(!listView.itemAtIndex(index).children[0].isSwipeOpen) {
                return false
            }
        }
        return true
    }

    function closeAllSwipes() {
        for(let index = 0; index < listView.count; index++) {
            if(listView.itemAtIndex(index).children[0] instanceof SwipeDelegate) {
                listView.contentItem.children[index].children[0].swipe.close()
            }
        }
    }

    _viewContentData: ColumnLayout {
        id: layout

        spacing: 0

        VPNSearchBar {
            id: searchBar

            Layout.fillWidth: true
            Layout.topMargin: VPNTheme.theme.listSpacing
            Layout.leftMargin: VPNTheme.theme.windowMargin * 1.5
            Layout.rightMargin: VPNTheme.theme.windowMargin * 1.5
            enabled: !vpnFlickable.isEmptyState
            onEnabledChanged: if (!enabled) clearText()

            _searchBarPlaceholderText: VPNl18n.InAppMessagingSearchBarPlaceholderText
            _searchBarHasError: !vpnFlickable.isEmptyState && listView.count === 0

            _filterProxySource: VPNAddonManager
            _filterProxyCallback: obj => obj.addon.type === "message" && obj.addon.containsSearchString(getSearchBarText())
            _sortProxyCallback: (obj1, obj2) => obj1.addon.date > obj2.addon.date
            _editCallback: () => { vpnFlickable.isEditing = false }
        }

        Image {
            Layout.topMargin: VPNTheme.theme.vSpacingSmall * 2
            Layout.alignment: Qt.AlignHCenter

            source: "qrc:/ui/resources/messages-empty.svg"
            sourceSize.width: 184
            sourceSize.height: 184
            visible: vpnFlickable.isEmptyState

        }

        VPNHeadline {
            Layout.topMargin: VPNTheme.theme.vSpacingSmall
            Layout.leftMargin: VPNTheme.theme.windowMargin * 1.5
            Layout.rightMargin: VPNTheme.theme.windowMargin * 1.5
            Layout.fillWidth: true

            text: VPNl18n.InAppMessagingEmptyStateTitle
            visible: vpnFlickable.isEmptyState
        }

        VPNInterLabel {
            Layout.topMargin: VPNTheme.theme.vSpacingSmall / 2
            Layout.leftMargin: VPNTheme.theme.windowMargin * 1.5
            Layout.rightMargin: VPNTheme.theme.windowMargin * 1.5
            Layout.fillWidth: true

            text: VPNl18n.InAppMessagingEmptyStateDescription
            visible: vpnFlickable.isEmptyState
            color: VPNTheme.theme.fontColor
        }

        Rectangle {
            Layout.topMargin: VPNTheme.theme.vSpacing
            Layout.fillWidth: true
            Layout.preferredHeight: 1

            color: VPNTheme.colors.grey10
            visible: !vpnFlickable.isEmptyState
        }

        ListView {
            id: listView

            interactive: false
            Layout.fillWidth: true
            Layout.preferredHeight: childrenRect.height
            contentHeight: childrenRect.height
            spacing: 0

            add: Transition{
                NumberAnimation{
                    property:"opacity"
                    from: 0
                    to: 1
                    duration: 300
                    easing.type: Easing.InOutQuad
                }
            }

            addDisplaced: Transition{
                NumberAnimation{
                    property:"y"
                    duration: 300
                    easing.type: Easing.InOutQuad
                }
            }

            removeDisplaced: Transition{
                NumberAnimation{
                    property:"y"
                    duration: 300
                    easing.type: Easing.InOutQuad
                }
            }

            model: searchBar.getProxyModel()
            delegate: ColumnLayout {
                //See https://bugreports.qt.io/browse/QTBUG-81976
                width: ListView.view.width

                spacing: 0

                VPNSwipeDelegate {
                    id: swipeDelegate

                    property real deleteLabelWidth: 0.0

                    //avoids qml warnings when addon messages get disabled via condition
                    property string title: typeof(addon) !== "undefined" ? addon.title : ""
                    property string formattedDate: typeof(addon) !== "undefined" ? addon.formattedDate : ""
                    property string subtitle: typeof(addon) !== "undefined" ? addon.subtitle : ""

                    Layout.fillWidth: true
                    Layout.preferredHeight: content.item.implicitHeight
                    Accessible.name: swipeDelegate.title + ". " + swipeDelegate.formattedDate + ". " +  swipeDelegate.subtitle

                    onSwipeOpen: () => {
                                     deleteLabelWidth = swipe.leftItem.width
                                     if (vpnFlickable.allSwipesOpen() && !vpnFlickable.isEditing) vpnFlickable.isEditing = true
                                 }

                    onIsSwipeOpenChanged: {
                        if(!isSwipeOpen && !vpnFlickable.anySwipesOpen() && vpnFlickable.isEditing) vpnFlickable.isEditing = false
                    }

                    onClicked: {
                        if (vpnFlickable.anySwipesOpen()) vpnFlickable.closeAllSwipes()
                        else {
                            addon.markAsRead()
                            stackview.push("qrc:/ui/screens/messaging/ViewMessage.qml", {"message": addon})
                        }
                    }

                    swipe.left: VPNSwipeAction {
                        id: deleteSwipeAction

                        activeFocusOnTab: swipeDelegate.isSwipeOpen
                        bgColor: VPNTheme.theme.redHovered
                        content: Image {
                            anchors.centerIn: parent
                            source: "qrc:/nebula/resources/delete-white.svg"
                        }
                        Accessible.name: VPNl18n.InAppMessagingDeleteMessage

                        SwipeDelegate.onClicked: {
                            swipeDelegate.swipe.close() // prevents weird iOS animation bug
                            swipeDelegate.isSwipeOpen = false
                            divider.visible = false
                            if(index === listView.count - 1) {
                                dismissMessageAnimation.start()
                            }
                            else {
                                dismissAddon()
                            }
                        }

                        function dismissAddon() {
                            addon.dismiss()
                            //Since opening up all (even if there is just 1) visible messages swipes turns on edit mode, make sure to turn it off if there are no more visible messages
                            if (searchBar.getProxyModel().rowCount() === 0) {
                                vpnFlickable.isEditing = false
                            }
                        }

                        PropertyAnimation {
                            id: dismissMessageAnimation
                            target: swipeDelegate.content.item
                            property: "implicitHeight"
                            to: 0
                            easing.type: Easing.InOutQuad
                            onFinished: dismissAddon()
                        }
                    }

                    content.sourceComponent: ColumnLayout {
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.right: parent.right

                        spacing: 0

                        RowLayout {
                            Layout.topMargin: VPNTheme.theme.windowMargin
                            Layout.leftMargin: VPNTheme.theme.windowMargin
                            Layout.rightMargin: VPNTheme.theme.windowMargin

                            spacing: VPNTheme.theme.listSpacing

                            Rectangle {
                                id: dot
                                Layout.preferredHeight: 8
                                Layout.preferredWidth: 8

                                opacity: addon.isRead? 0 : 1
                                radius: Layout.preferredHeight / 2
                                color: VPNTheme.theme.blue
                            }

                            VPNBoldInterLabel {
                                id: title
                                Layout.fillWidth: true

                                text: swipeDelegate.title
                                font.pixelSize: VPNTheme.theme.fontSize
                                lineHeight: VPNTheme.theme.labelLineHeight
                                verticalAlignment: Text.AlignVCenter
                                elide: Text.ElideRight
                                maximumLineCount: 1
                            }

                            VPNInterLabel {
                                text: swipeDelegate.formattedDate
                                font.pixelSize: VPNTheme.theme.fontSizeSmall
                                lineHeight: 21
                                color: VPNTheme.theme.fontColor
                                horizontalAlignment: Text.AlignRight
                            }
                        }

                        VPNInterLabel {
                            Layout.leftMargin: VPNTheme.theme.windowMargin * 2
                            Layout.bottomMargin: VPNTheme.theme.windowMargin
                            Layout.maximumWidth: title.width

                            text: swipeDelegate.subtitle
                            font.pixelSize: VPNTheme.theme.fontSizeSmall
                            lineHeight: 21
                            color: VPNTheme.theme.fontColor
                            horizontalAlignment: Text.AlignLeft
                            elide: Text.ElideRight
                            maximumLineCount: 1
                        }
                    }

                    Connections {
                        target: vpnFlickable
                        function onEditModeChanged() {
                            if(vpnFlickable.isEditing) {
                                swipeDelegate.swipe.open(SwipeDelegate.Left)
                            }
                            else {
                                swipeDelegate.swipe.close()
                            }
                        }
                    }

                }

                Rectangle {
                    id: divider
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1

                    color: VPNTheme.colors.grey10
                }
            }
        }
    }

    VPNFilterProxyModel {
        id: messagesModel
        source: VPNAddonManager
        filterCallback: obj => { return obj.addon.type === "message" }
        Component.onCompleted: {
            vpnFlickable.isEmptyState = Qt.binding(() => { return messagesModel.count === 0} )
        }
    }
}
