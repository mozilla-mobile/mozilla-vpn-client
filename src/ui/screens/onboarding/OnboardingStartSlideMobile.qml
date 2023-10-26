/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import "qrc:/nebula/utils/MZUiUtils.js" as MZUiUtils

ColumnLayout {
    id: root

    signal nextClicked()
    signal backClicked()

    spacing: 0

    MZHeadline {
        Layout.topMargin: MZTheme.theme.windowMargin * 1.5
        Layout.leftMargin: MZTheme.theme.windowMargin * 2
        Layout.rightMargin: MZTheme.theme.windowMargin * 2

        Layout.fillWidth: true

        text: MZI18n.OnboardingStartSlideMobileHeader
        horizontalAlignment: Text.AlignLeft
    }

    MZInterLabel {
        Layout.topMargin: 8
        Layout.leftMargin: MZTheme.theme.windowMargin * 2
        Layout.rightMargin: MZTheme.theme.windowMargin * 2
        Layout.fillWidth: true

        text: MZI18n.OnboardingStartSlideMobileBody
        horizontalAlignment: Text.AlignLeft
        color: MZTheme.theme.fontColor
    }

    Item {
        Layout.fillHeight: true
        Layout.minimumHeight: 16
    }

    Image {
        Layout.alignment: Qt.AlignHCenter

        sourceSize: MZUiUtils.isLargePhone() ? Qt.size(209,144) : Qt.size(151,104)
        source: "qrc:/ui/resources/link.svg"
    }

    Item {
        Layout.fillHeight: true
        Layout.minimumHeight: 16
    }

    MZButton {

        Layout.leftMargin: MZTheme.theme.windowMargin * 2
        Layout.rightMargin: MZTheme.theme.windowMargin * 2
        Layout.fillWidth: true

        width: undefined
        text: MZI18n.OnboardingStartSlideAllowButtonLabel

        onClicked: {
            //Not actually activating the VPN, but going through the motions of activating it
            //so we can add the VPN configuration to the system
            VPN.activate()
        }
    }

    MZLinkButton {

        Layout.topMargin: 16
        Layout.leftMargin: MZTheme.theme.windowMargin * 2
        Layout.rightMargin: MZTheme.theme.windowMargin * 2
        Layout.bottomMargin: MZTheme.theme.windowMargin * 2
        Layout.fillWidth: true

        implicitHeight: MZTheme.theme.rowHeight

        labelText: MZI18n.GlobalGoBack

        onClicked: root.backClicked()
    }
}
