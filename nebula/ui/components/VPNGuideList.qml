/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import Mozilla.VPN 1.0
import Mozilla.VPN.qmlcomponents 1.0
import components 0.1

GridLayout {
    objectName: "guideLayout"

    property var customFilter: () => true
    property var count: guideRepeater.count

    function guideFilter(addon) {
        return addon.type === "guide" && customFilter(addon);
    }

    VPNFilterProxyModel {
        id: guideModel
        source: VPNAddonManager
        filterCallback: ({ addon }) => guideFilter(addon)
    }

    columns: width < VPNTheme.theme.tabletMinimumWidth ? 2 : 3
    columnSpacing: VPNTheme.theme.vSpacingSmall
    rowSpacing: VPNTheme.theme.vSpacingSmall

    Repeater {
        id: guideRepeater
        model: guideModel

        delegate: VPNGuideCard {
            objectName: addon.id

            Layout.preferredHeight: VPNTheme.theme.guideCardHeight
            Layout.fillWidth: true

            imageSrc: addon.image
            title: addon.title

            onClicked:{
                stackview.push("qrc:/ui/screens/settings/ViewGuide.qml", {"guide": addon, "imageBgColor": imageBgColor})
                VPN.recordGleanEventWithExtraKeys("guideOpened", {
                    "id": addon.id
                });
            }
        }
    }
}

