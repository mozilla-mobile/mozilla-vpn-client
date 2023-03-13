/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// IMPORTANT: this file is used only for mobile builds.

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1

Item {
    id: logs

    objectName: "viewLogs"
    width: window.width
    height: window.safeContentHeight

    MZMenu {
        id: menu

        //% "View Logs"
        title: qsTrId("vpn.viewlogs.title")
        _menuOnBackClicked: () => VPNNavigator.requestPreviousScreen()
    }

    MZFlickable {
        id: logScrollView

        height: logs.height - menu.height - copyClearWrapper.height
        flickContentHeight: logText.height + MZTheme.theme.windowMargin
        width: logs.width - MZTheme.theme.windowMargin
        anchors.top: menu.bottom
        anchors.leftMargin: MZTheme.theme.windowMargin
        anchors.horizontalCenter: logs.horizontalCenter
        contentWidth: width - (MZTheme.theme.windowMargin * 2)

        MZTextBlock {
            id: logText
            y: MZTheme.theme.windowMargin
            font.pixelSize: 11
            lineHeight: 16
            width: parent.width

            Connections {
                target: MZLog
                function onLogsReady(logs) {
                    logText.text = logs;
                }
            }

            Component.onCompleted: {
                MZLog.retrieveLogs();
            }
        }
    }

    Rectangle {
        id: div

        width: parent.width
        height: 1
        color: MZTheme.theme.divider
        Layout.alignment: Qt.AlignRight
        anchors.bottom: copyClearWrapper.top
    }

    Rectangle {
        id: copyClearWrapper

        color: MZTheme.theme.bgColor
        height: MZTheme.theme.rowHeight * 1.5
        anchors.bottom: parent.bottom
        width: logs.width

        RowLayout {
            spacing: 0
            height: parent.height
            width: copyClearWrapper.width

            MZLogsButton {
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
                color: MZTheme.theme.divider
                Layout.alignment: Qt.AlignRight
            }

            MZLogsButton {
                //% "Clear"
                buttonText: qsTrId("vpn.logs.clear")
                iconSource: "qrc:/nebula/resources/delete.svg"
                onClicked: {
                    MZLog.flushLogs();
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
}
