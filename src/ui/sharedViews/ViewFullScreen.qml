/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15


import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1

import "qrc:/nebula/utils/MZUiUtils.js" as MZUiUtils

//View that spaces out content to take up the entire screen, but scrolls if necessary (typically on smaller devices)
//Supports different spacing options for desktop and mobile becase
//Desktop - we like to position the buttons right beneath the content
//Mobile - we like to position the buttons at the bottom of the screen

//USAGE: pass a component (recommended) or array of components (not recommended) to the content and buttons property

/*
    ViewFullScreen {
        content: ColumnLayout {
            ...
        }
        buttons: ColumnLayout {
            ...
        }
    }
*/


MZFlickable {
    id: flickable

    property bool isMobile: MZUiUtils.isMobile()
    property alias content: content.data
    property alias buttons: buttons.data

    Layout.fillWidth: true

    //Weirdly get a "parent is null" warning when popping of a stackview without checking if parent is null
    implicitHeight: parent ? parent.height : 0

    ColumnLayout {
        id: layout

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: flickable.isMobile ? 0 : 24
        anchors.leftMargin: 24
        anchors.rightMargin: 24

        height: flickable.implicitHeight

        spacing: 0

        Item {
            Layout.minimumHeight: 24
            Layout.fillHeight: true

            visible: flickable.isMobile
        }

        ColumnLayout {
            id: content

            spacing: 0
        }

        Item {
            Layout.minimumHeight: 24
            Layout.fillHeight: true

            visible: flickable.isMobile
        }

        ColumnLayout {
            id: buttons

            Layout.topMargin: flickable.isMobile ? 0 : 40
            Layout.bottomMargin: flickable.isMobile ? 36 : 48 //58 for iPhones with safe area
            spacing: 0
        }

         Item {
            visible: flickable.isMobile && VPN.UserAuthenticated
            Layout.minimumHeight: MZTheme.theme.navBarBottomMargin + MZTheme.theme.navBarHeight
        }

        Item {
            visible: !flickable.isMobile
            Layout.fillHeight: true
        }
    }
}
