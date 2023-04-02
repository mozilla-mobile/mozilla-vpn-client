/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import Mozilla.Shared 1.0
import Mozilla.Shared.qmlcomponents 1.0
import components 0.1

GridLayout {
    objectName: "guideLayout"

    property var customFilter: () => true
    property var count: guideRepeater.count

    function guideFilter(addon) {
        return !!addon.as(MZAddon.TypeGuide) && customFilter(addon);
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
            property var addonGuide: addon.as(MZAddon.TypeGuide)

            objectName: addonGuide.id

            Layout.preferredHeight: MZTheme.theme.guideCardHeight
            Layout.fillWidth: true

            imageSrc: addonGuide.image
            title: addonGuide.title

            onClicked:{
                stackview.push("qrc:/ui/screens/settings/ViewGuide.qml", {"guide": addonGuide, "imageBgColor": imageBgColor})
                MZGleanDeprecated.recordGleanEventWithExtraKeys("guideOpened", {
                    "id": addonGuide.id
                });
                Glean.sample.guideOpened.record({ id: addonGuide.id });
            }
        }
    }
}
