/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.6
import QtQuick.Layouts 1.14
import Qt.labs.qmlmodels 1.0

import Mozilla.Shared 1.0
import components.forms 0.1

ColumnLayout {
    id: messagesContainer
    property var messages

    spacing: MZTheme.theme.listSpacing / 4

    DelegateChooser {
        id: messagesChooser
        role: "type"

        DelegateChoice {
            roleValue: "warning"
            delegate: MZContextualAlert {
                fontColor: MZTheme.colors.fontColorWarning
                iconSrc: "qrc:/nebula/resources/warning-dark-orange.svg"
            }
        }
        DelegateChoice {
            roleValue: "error"
            delegate: MZContextualAlert {
                fontColor: MZTheme.colors.errorAccentLight
                iconSrc: "qrc:/nebula/resources/warning.svg"
            }
        }
    }

    Repeater {
        id: messagesRepeater
        delegate: messagesChooser
        model: messages
    }
}
