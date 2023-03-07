/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import Mozilla.Shared 1.0

MZPopup {
    property alias imageSrc: image.source
    property alias imageSize: image.sourceSize
    property alias title: titleText.text
    property alias description: descriptionText.text
    property list<MZButtonBase> buttons

    popupContentItem.implicitHeight: popupLayout.implicitHeight

    ColumnLayout {
        id: popupLayout

        anchors.fill: parent
        anchors.leftMargin: MZTheme.theme.popupMargin
        anchors.rightMargin: MZTheme.theme.popupMargin

        spacing: 0

        Image {
            id: image
            Layout.alignment: Qt.AlignHCenter
            asynchronous: true
        }

        MZMetropolisLabel {
            id: titleText

            Layout.topMargin: MZTheme.theme.vSpacingSmall
            Layout.fillWidth: true

            color: MZTheme.theme.fontColorDark
            font.pixelSize: MZTheme.theme.fontSizeLarge
            lineHeight: MZTheme.theme.vSpacingSmall * 2

        }

        MZInterLabel {
            id: descriptionText

            Layout.topMargin: MZTheme.theme.vSpacingSmall / 2
            Layout.fillWidth: true

            color: MZTheme.theme.fontColor
        }

        ColumnLayout {
            Layout.topMargin: MZTheme.theme.vSpacingSmall * 2

            spacing: 0
            data: buttons
            visible: buttons.length > 0

            Component.onCompleted: {
                for(var i = 0 ; i < buttons.length; i++) {
                    if(i > 0) buttons[i].Layout.topMargin = MZTheme.theme.vSpacingSmall
                }
            }
        }

        Item {
            Layout.preferredHeight: MZTheme.theme.vSpacing
        }
    }
}
