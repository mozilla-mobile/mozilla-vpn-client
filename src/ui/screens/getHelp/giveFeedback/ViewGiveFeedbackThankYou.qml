/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1

ColumnLayout {
    id: layout

    spacing: VPNTheme.theme.vSpacing

    Item {
        Layout.fillHeight: window.fullscreenRequired()
    }

    VPNPanel {
        anchors.horizontalCenter: undefined
        Layout.topMargin: window.fullscreenRequired() ? 0 : layout.height * 0.2
        Layout.preferredHeight: height
        Layout.preferredWidth: width
        Layout.alignment: Qt.AlignHCenter

        logo: "qrc:/ui/resources/heart-check.svg"
        //% "Thank you!"
        logoTitle: qsTrId("vpn.feedbackForm.thankyou")
        //% "We appreciate your feedback. You’re helping us improve Mozilla VPN."
        logoSubtitle: qsTrId("vpn.feedbackForm.thankyouSubtitle")
    }

    Item {
        Layout.fillHeight: window.fullscreenRequired()
    }

    VPNButton {
        Layout.fillWidth: true
        Layout.leftMargin: VPNTheme.theme.windowMargin * 2
        Layout.rightMargin: VPNTheme.theme.windowMargin * 2
        Layout.bottomMargin: navbar.visible ? VPNTheme.theme.navBarHeightWithMargins : 34

        //% "Done"
        text: qsTrId("vpn.feedbackform.done")
        onClicked: getHelpStackView.pop(null, StackView.Immediate)

    }

    Item {
        Layout.fillHeight: !window.fullscreenRequired()
    }
}
