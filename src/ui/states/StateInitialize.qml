/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14

import components 0.1

Item {
    VPNStackView {
        id: stackview

        anchors.fill: parent
        initialItem: {
            if (Qt.platform.os === "android" || Qt.platform.os === "ios")
                return "qrc:/ui/views/ViewMobileOnboarding.qml";
            return "qrc:/ui/views/ViewInitialize.qml"
        }
    }
}
