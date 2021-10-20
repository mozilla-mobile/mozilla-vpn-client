/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// IMPORTANT: this file is used only for mobile builds.

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import themes 0.1

Item {
    id: logs

    width: window.width
    height: window.safeContentHeight
    Component.onCompleted: VPNCloseEventHandler.addView(logs)

    VPNMenu {
        id: menu

        isMainView: true
        //% "View Logs"
        title: qsTrId("vpn.viewlogs.title")
    }

    ScrollView {
        id: logScrollView

        height: logs.height - menu.height - copyClearWrapper.height
        width: logs.width
        anchors.top: menu.bottom
        anchors.horizontalCenter: logs.horizontalCenter
        contentWidth: width - (Theme.windowMargin * 2)
        clip: true
        topInset: Theme.rowHeight
        bottomInset: Theme.rowHeight * 2
        leftPadding: Theme.windowMargin
        rightPadding: Theme.windowMargin
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        ScrollBar.vertical.policy: ScrollBar.AlwaysOn

        VPNTextBlock {
            id: logText
            y: Theme.windowMargin
            font.pixelSize: 11
            lineHeight: 16
            width: parent.width

            Connections {
                target: VPN
                function onLogsReady(logs) {
                    logText.text = logs;
                }
            }

            Component.onCompleted: {
                VPN.retrieveLogs();
            }
        }
    }

    Rectangle {
        id: div

        width: parent.width
        height: 1
        color: Theme.divider
        Layout.alignment: Qt.AlignRight
        anchors.bottom: copyClearWrapper.top
    }

    Rectangle {
        id: copyClearWrapper

        color: Theme.bgColor
        height: Theme.rowHeight * 1.5
        anchors.bottom: parent.bottom
        width: logs.width

        RowLayout {
            spacing: 0
            height: parent.height
            width: copyClearWrapper.width

            VPNLogsButton {
                //% "Copy"
                buttonText: qsTrId("vpn.logs.copy")
                iconSource: "qrc:/ui/resources/copy.svg"
                onClicked: {
                    VPN.storeInClipboard(logText.text);
                    //% "Copied!"
                    buttonText = qsTrId("vpn.logs.copied");
                }
            }

            Rectangle {
                Layout.preferredHeight: parent.height
                Layout.preferredWidth: 1
                color: Theme.divider
                Layout.alignment: Qt.AlignRight
            }

            VPNLogsButton {
                //% "Clear"
                buttonText: qsTrId("vpn.logs.clear")
                iconSource: "qrc:/ui/resources/delete.svg"
                onClicked: {
                    VPN.cleanupLogs();
                    logText.text = "";
                }
            }

        }

        Rectangle {
            id: divider

            height: 1
            width: parent.width
            anchors.bottom: parent.bottom
            color: "#0C0C0D0A"
        }

    }

    Connections {
        function onGoBack(item) {
            if (item === logs)
                mainStackView.pop();

        }

        target: VPNCloseEventHandler
    }

}
