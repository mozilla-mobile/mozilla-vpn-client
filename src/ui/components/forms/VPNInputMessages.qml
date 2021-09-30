/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.14
import Qt.labs.qmlmodels 1.0
import Mozilla.VPN 1.0
import "../../themes/themes.js" as Theme
import "../../themes/colors.js" as Color

ColumnLayout {
  id: messagesContainer
  spacing: 8

  ListModel {
    id: messagesModel

    ListElement {
      message: "Please help"
      type: "error"
      visible: true
    }

    ListElement {
      message: "Ignore me"
      type: "warning"
      visible: true
    }

    ListElement {
      message: "This is just informational"
      type: "info"
      visible: true
    }
  }

  Component {
    id: messageItem

    Button {
      text: message
    }
  }

  DelegateChooser {
    id: messagesChooser
    role: "type"

    DelegateChoice {
      roleValue: "info"
      delegate: messageItem
    }
    DelegateChoice {
      roleValue: "warning"
      delegate: messageItem
    }
    DelegateChoice {
      roleValue: "error"
      delegate: messageItem
    }
  }

  Repeater {
    id: messagesRepeater
    model: messagesModel
    delegate: messagesChooser
  }
}
