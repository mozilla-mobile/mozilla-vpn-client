/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import components 0.1
import components.forms 0.1

ColumnLayout {
    id: layout

    spacing: MZTheme.theme.vSpacing

    Item {
        Layout.fillHeight: window.fullscreenRequired()
    }

    MZPanel {
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

    MZButton {
        Layout.fillWidth: true
        Layout.leftMargin: MZTheme.theme.windowMargin * 2
        Layout.rightMargin: MZTheme.theme.windowMargin * 2
        Layout.bottomMargin: navbar.visible ? MZTheme.theme.navBarHeightWithMargins : 34

        //% "Done"
        text: qsTrId("vpn.feedbackform.done")
        onClicked: getHelpStackView.pop(null, StackView.Immediate)

    }

    Item {
        Layout.fillHeight: !window.fullscreenRequired()
    }
}
