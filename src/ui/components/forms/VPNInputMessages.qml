/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.6
import QtQuick.Layouts 1.14
import Qt.labs.qmlmodels 1.0

import "../../themes/themes.js" as Theme
import "../../themes/colors.js" as Color

ColumnLayout {
    id: messagesContainer
    property var messages

    anchors.left: parent.left
    anchors.right: parent.right
    anchors.topMargin: Theme.listSpacing
    spacing: Theme.listSpacing / 4

    DelegateChooser {
        id: messagesChooser
        role: "type"

        DelegateChoice {
            roleValue: "info"
            delegate: VPNInputMessage {
                fontColor: Color.informational.default
                iconSrc: "../../resources/connection-info-dark.svg"
            }
        }
        DelegateChoice {
            roleValue: "warning"
            delegate: VPNInputMessage {
                fontColor: Color.warning.default
                iconSrc: "../../resources/warning-orange.svg"
            }
        }
        DelegateChoice {
            roleValue: "error"
            delegate: VPNInputMessage {
                fontColor: Color.error.default
                iconSrc: "../../resources/warning.svg"
            }
        }
    }

    Repeater {
        id: messagesRepeater
        delegate: messagesChooser
        model: messages
    }
}
