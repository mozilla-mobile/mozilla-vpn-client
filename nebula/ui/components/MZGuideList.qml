/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1

GridLayout {
    objectName: "guideLayout"

    property var customFilter: () => true
    property var count: guideRepeater.count

    function guideFilter(addon) {
        return addon.type === "guide" && customFilter(addon);
    }

    MZFilterProxyModel {
        id: guideModel
        source: MZAddonManager
        filterCallback: ({ addon }) => guideFilter(addon)
    }

    columns: width < MZTheme.theme.tabletMinimumWidth ? 2 : 3
    columnSpacing: MZTheme.theme.vSpacingSmall
    rowSpacing: MZTheme.theme.vSpacingSmall

    Repeater {
        id: guideRepeater
        model: guideModel

        delegate: MZGuideCard {
            objectName: addon.id

            Layout.preferredHeight: MZTheme.theme.guideCardHeight
            Layout.fillWidth: true

            imageSrc: addon.image
            title: addon.title

            onClicked:{
                stackview.push("qrc:/ui/screens/settings/ViewGuide.qml", {"guide": addon, "imageBgColor": imageBgColor})
                Glean.sample.guideOpened.record({ id: addon.id });
            }
        }
    }
}
