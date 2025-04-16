/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.6
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import components.forms 0.1

ColumnLayout {
    id: messagesContainer
    property var messages

    spacing: MZTheme.theme.listSpacing / 4

    Repeater {
        id: messagesRepeater
        delegate: MZContextualAlert {
                alertType: modelData.type
                messageString: modelData.message
                visible: modelData.visible
            }
        model: messages
    }
}
