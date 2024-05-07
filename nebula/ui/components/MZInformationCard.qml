/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQuick.Window 2.12

import Mozilla.Shared 1.0
import compat 0.1
import components 0.1

Rectangle {
    property alias _infoContent: infoContent.data
    id: card

    enum CardType {
        Info,
        Success,
        Warning,
        Error
    }

    color: MZTheme.theme.white
    radius: 8
    property int cardType: MZInformationCard.CardType.Error

    MZDropShadow {
        anchors.fill: dropShadowSource
        source: dropShadowSource

        horizontalOffset: 1
        verticalOffset: 1
        radius: 8
        color: MZTheme.colors.grey60
        opacity: .15
        transparentBorder: true
        cached: true
    }

    Rectangle {
        id: dropShadowSource
        anchors.fill: card

        color: MZTheme.theme.white
        radius: card.radius
    }

    // This pair of rectangles is a gross hack to only have radius on one side.
    // Qt 6.7 releases support for `topLeftRadius` and `bottomLeftRadius`, we can
    // improve this once we get on that version. (And `clip` doesn't work for
    // parent's radius, so that cannot be used.)
    Rectangle {
        id: colorStripRounded
        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
        }

        width: 16
        radius: card.radius
        color: getStripColor()
    }

    Rectangle {
        id: colorStripSquare
        anchors {
            left: parent.left
            leftMargin: 8
            top: parent.top
            bottom: parent.bottom
        }

        width: 8
        color: MZTheme.theme.white
    }

    function getStripColor() {
        switch (cardType) {
        case MZInformationCard.CardType.Info:
            return MZTheme.colors.blue50
        case MZInformationCard.CardType.Success:
            return MZTheme.colors.green50
        case MZInformationCard.CardType.Warning:
            return MZTheme.colors.yellow50
        case MZInformationCard.CardType.Error:
            return MZTheme.colors.red60
        default:
            return console.error("Unable to create view for info card of type: " + cardType)
        }
    }

    function getIconImage() {
        switch (cardType) {
        case MZInformationCard.CardType.Info:
            return "qrc:/nebula/resources/info.svg"
        case MZInformationCard.CardType.Success:
            return "qrc:/nebula/resources/success.svg"
        case MZInformationCard.CardType.Warning:
            return "qrc:/nebula/resources/warning-gray.svg"
        case MZInformationCard.CardType.Error:
            return "qrc:/nebula/resources/warning-gray.svg"
        default:
            return console.error("Unable to create view for info card of type: " + cardType)
        }
    }

    RowLayout {
        id: info
        spacing: MZTheme.theme.windowMargin * 0.75

        anchors {
            left: parent.left
            right: parent.right
            leftMargin: MZTheme.theme.windowMargin
            rightMargin: MZTheme.theme.windowMargin
            verticalCenter: parent.verticalCenter
        }

        MZIcon {
            source: getIconImage()
            Layout.leftMargin: 4 // to account for the color stripe
            Layout.alignment: Qt.AlignVCenter
        }

        ColumnLayout {
            id: infoContent
            Layout.fillWidth: true
            spacing: 0
        }
    }
}

