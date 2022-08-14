/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.14

import Mozilla.VPN 1.0
import components 0.1

StackLayout {
    id: mainStackLayout

    property int selectedTab: StateMain.Tab.Home

    enum Tab {
        Home,
        Messages,
        Settings
    }

    function unwindAllStacks() {
        homeStack.unwindToInitialItem()
        messagesStack.unwindToInitialItem()
        settingsStack.unwindToInitialItem()
    }

    objectName: "ViewMainStackView"

    anchors.fill: parent

    currentIndex: selectedTab

    Connections {
        target: window
        function onShowServerList() {
            // We get here after the user clicks the "Choose new location" button in VPNServerUnavailablePopup {}
            // We need to (maybe) unwind the stack back to ViewMain.qml and then push the server list.
            if (homeStack.currentItem.objectName === "viewServers") {
                // User is already on server list view so we stay put
                return;
            }
            homeStack.unwindToInitialItem();
            homeStack.push("qrc:/ui/views/ViewServers.qml", StackView.Immediate)
        }
        function onShowHomeStack(popToRoot) {
            if(mainStackLayout.selectedTab === StateMain.Tab.Home || popToRoot) {
                homeStack.unwindToInitialItem()
                mainStackView.unwindToInitialItem()
            }
            mainStackLayout.selectedTab = StateMain.Tab.Home
        }
        function onShowMessagesStack(popToRoot) {
            if(mainStackLayout.selectedTab === StateMain.Tab.Messages || popToRoot) {
                messagesStack.unwindToInitialItem()
                mainStackView.unwindToInitialItem()
            }
            mainStackLayout.selectedTab = StateMain.Tab.Messages
        }
        function onShowSettingsStack(popToRoot) {
            if(mainStackLayout.selectedTab === StateMain.Tab.Settings || popToRoot)  {
                settingsStack.Layout.topMargin = 0
                settingsStack.z = 0
                settingsStack.unwindToInitialItem()
                mainStackView.unwindToInitialItem()
            }
            mainStackLayout.selectedTab = StateMain.Tab.Settings
        }
        function onDeepLinkToSettingsView(src) {
            settingsStack.push(src)
        }
    }

    VPNStackView {
        id: homeStack

        anchors.fill: undefined
    }

    VPNStackView {
        id: messagesStack

        anchors.fill: undefined
    }

    ColumnLayout {
        id: settings

        spacing: 0

        VPNMenu {
            id: settingsStackMenu

            Layout.fillWidth: true
            width: undefined

            objectName: "settingsBackButton"
            _menuOnBackClicked: () => {
                VPNProfileFlow.reset();
                if (settingsStack.depth > 1) {
                    settingsStack.pop();
                }
            }
            _iconButtonSource: "qrc:/nebula/resources/back.svg"
            iconButtonVisible: settingsStack.depth > 1

            //Developer menu unlock
            VPNMouseArea {
                property int unlockCounter: 0

                anchors.fill: settingsStackMenu.titleComponent
                enabled: settingsStack.currentItem.objectName === "getHelp"
                cursorShape: Qt.ArrowCursor
                hoverEnabled: true
                onMouseAreaClicked: function() {
                    if (unlockCounter >= 5) {
                        unlockCounter = 0
                        VPNSettings.developerUnlock = true
                    }
                    else if (!VPNSettings.developerUnlock) {
                        unlockTimeout.restart()
                        unlockCounter = unlockCounter + 1
                    }
                }

                Timer {
                    id: unlockTimeout
                    repeat: false
                    running: false
                    interval: 10000
                    onTriggered: unlockCounter = 0
                }
            }

        }

        VPNStackView {
            id: settingsStack
            objectName: "settingsStack"

            anchors.fill: undefined
            Layout.fillHeight: true
            Layout.fillWidth: true

            onCurrentItemChanged: settingsStackMenu.title = Qt.binding(() => currentItem._menuTitle || qsTrId("vpn.main.settings"));
        }
    }

    Component.onCompleted: {
        const homeComponent = Qt.createComponent("qrc:/ui/views/ViewMain.qml")
        const homeObject = homeComponent.createObject(homeStack, {height: parent.height, width: parent.width})
        homeStack.push(homeObject)
        window.showHomeStack(true)

        const messagesComponent = Qt.createComponent("qrc:/ui/messages/ViewMessagesInbox.qml")
        const messagesObject = messagesComponent.createObject(messagesStack, {height: parent.height, width: parent.width})
        messagesStack.push(messagesObject)

        const settingsComponent = Qt.createComponent("qrc:/ui/settings/ViewSettingsMenu.qml")
        const settingsObject = settingsComponent.createObject(settingsStack, {height: parent.height, width: parent.width})
        settingsStack.push(settingsObject)

    }
}

