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
    id: base
    property string _emailAddress: ""

    spacing: 24

    VPNPanel {
        anchors.horizontalCenter: undefined
        Layout.topMargin: base.height * (window.fullscreenRequired() ? 0.3 :  0.2)
        Layout.preferredHeight: height
        Layout.preferredWidth: width

        logo: "qrc:/ui/resources/heart-check.svg"
        logoTitle: VPNl18n.InAppSupportWorkflowSupportResponseHeader
        logoSubtitle: VPNl18n.InAppSupportWorkflowSupportResponseBody.arg(base._emailAddress)
    }

    Item {
        Layout.fillHeight: window.fullscreenRequired()
    }

    VPNButton {
        Layout.fillWidth: true
        Layout.leftMargin: VPNTheme.theme.windowMargin * 2
        Layout.rightMargin: VPNTheme.theme.windowMargin * 2
        Layout.bottomMargin: navbar.visible ? VPNTheme.theme.navBarHeightWithMargins : 34

        text: VPNl18n.InAppSupportWorkflowSupportResponseButton

        onClicked: {
            getHelpStackView.pop();
            getHelpStackView.pop();
        }
    }

    Item {
        Layout.fillHeight: !window.fullscreenRequired()
    }
}
