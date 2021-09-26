/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.14
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../themes/themes.js" as Theme
import "../components"

VPNFlickable {
    id: vpnFlickable

    ColumnLayout {
        id: column
        width: vpnFlickable.width
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        spacing: Theme.windowMargin * 1.25

        VPNVerticalSpacer {
            Layout.fillWidth: true
            height: Theme.windowMargin
        }

        Repeater {
            model: VPNModuleModel

            delegate: ColumnLayout {
                id: container

                Layout.leftMargin: Theme.windowMargin / 2
                Layout.rightMargin: Theme.windowMargin
                Layout.bottomMargin: Theme.vSpacingSmall
                Layout.topMargin: Theme.vSpacingSmall

                Component.onCompleted: {
                  let component = Qt.createComponent(widget);

                  function attachWidget() {
                    switch (component.status) {
                      case Component.Ready:
                        let widget = component.createObject(container, {x: 0, y: 0});
                        if (widget == null) {
                          console.log("Error creating object");
                        }
                        break;

                      case Component.Loading:
                        component.statusChanged.connect(attachWidget);
                        break;

                      default:
                        console.error("Error loading widget:", widget, component.errorString());
                        break;
                    }
                  }

                  attachWidget();
                }
            }
        }
    }
}
