/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import Mozilla.VPN 1.0

VPNPopup {
    property alias imageSrc: image.source
    property alias imageSize: image.sourceSize
    property alias title: titleText.text
    property alias description: descriptionText.text
    property list<VPNButtonBase> buttons

    popupContentItem.implicitHeight: popupLayout.implicitHeight

    ColumnLayout {
        id: popupLayout

        anchors.fill: parent
        anchors.leftMargin: VPNTheme.theme.popupMargin
        anchors.rightMargin: VPNTheme.theme.popupMargin

        spacing: 0

        Image {
            id: image
            Layout.alignment: Qt.AlignHCenter
            asynchronous: true
        }

        VPNMetropolisLabel {
            id: titleText

            Layout.topMargin: VPNTheme.theme.vSpacingSmall
            Layout.fillWidth: true

            color: VPNTheme.theme.fontColorDark
            font.pixelSize: VPNTheme.theme.fontSizeLarge
            lineHeight: VPNTheme.theme.vSpacingSmall * 2

        }

        VPNInterLabel {
            id: descriptionText

            Layout.topMargin: VPNTheme.theme.vSpacingSmall / 2
            Layout.fillWidth: true

            color: VPNTheme.theme.fontColor
        }

        ColumnLayout {
            Layout.topMargin: VPNTheme.theme.vSpacingSmall * 2

            spacing: 0
            data: buttons
            visible: buttons.length > 0

            Component.onCompleted: {
                for(var i = 0 ; i < buttons.length; i++) {
                    if(i > 0) buttons[i].Layout.topMargin = VPNTheme.theme.vSpacingSmall
                }
            }
        }

        Item {
            Layout.preferredHeight: VPNTheme.theme.vSpacing
        }
    }
}
