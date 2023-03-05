/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1

MZFlickable {
    id: vpnFlickable

    readonly property bool isMobile: window.fullscreenRequired()

    flickContentHeight: content.implicitHeight

    Component.onCompleted: {
        fade.start();
    }

    ColumnLayout {
        id: content

        spacing: MZTheme.theme.windowMargin * 1.25  
        height: vpnFlickable.height
        width: Math.min(vpnFlickable.width, MZTheme.theme.maxHorizontalContentWidth)
  

        Image {
            id: mainIcon
            source: "qrc:/ui/resources/warning-crash.svg"

            Layout.fillWidth: true
            Layout.topMargin: isMobile ? parent.height * 0.20 : parent.height * 0.12
            
            horizontalAlignment: Image.AlignHCenter
            fillMode: Image.PreserveAspectFit
        }

        MZHeadline {
            id: mainHeading
            text: qsTrId("vpn.crashreporter.mainHeading")
            horizontalAlignment: Text.AlignHCenter

            Layout.fillWidth: true
            Layout.topMargin: MZTheme.theme.vSpacingSmall
            Layout.leftMargin: MZTheme.theme.windowMargin * 2
            Layout.rightMargin: MZTheme.theme.windowMargin * 2
        }

        MZTextBlock{
            id: description
            
            horizontalAlignment: Text.AlignHCenter
            text: qsTrId("vpn.crashreporter.description")

            Layout.fillWidth: true
            Layout.leftMargin: MZTheme.theme.windowMargin * 2
            Layout.rightMargin: MZTheme.theme.windowMargin * 2

            font.pixelSize: MZTheme.theme.fontSize
           
        }
        Item {
            Layout.fillHeight: isMobile
        }

        MZButton {
            text: qsTrId("vpn.crashreporter.sendButtonLabel");
            
            onClicked: {
                VPNCrashReporter.allowCrashReporting()
                VPNNavigator.requestPreviousScreen();
            }

            Layout.fillWidth: true
            Layout.topMargin: MZTheme.theme.vSpacing
            Layout.leftMargin: MZTheme.theme.windowMargin * 2
            Layout.rightMargin: MZTheme.theme.windowMargin * 2
        }
        MZLinkButton {
            labelText: qsTrId("vpn.crashreporter.dontSendButton");
        
            onClicked: {
                VPNCrashReporter.declineCrashReporting()
                VPNNavigator.requestPreviousScreen();
            }
            Layout.fillWidth: true
            Layout.bottomMargin: 58
        }
        Item {
            Layout.fillHeight: !isMobile
        }
    }

    PropertyAnimation on opacity {
        id: fade

        from: 0
        to: 1
        duration: 300
    }

}
