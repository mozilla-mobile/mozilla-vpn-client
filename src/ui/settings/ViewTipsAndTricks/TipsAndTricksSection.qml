/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import Mozilla.VPN 1.0
import components 0.1

ColumnLayout {
    id: tipsAndTricksSection

    property var parentWidth

    property string title
    property string description

    property bool hasGuides
    property var guidesModel

    property bool hasTutorials
    property var tutorialsModel

    visible: hasGuides || hasTutorials

    // Title
    VPNBoldLabel {
        Layout.fillWidth: true

        text: title
        elide: Text.ElideRight
        lineHeightMode: Text.FixedHeight
        lineHeight: 24
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        maximumLineCount: 2
        wrapMode: Text.WordWrap

        Accessible.role: Accessible.StaticText
        Accessible.name: text
    }

    // Subtitle
    VPNTextBlock {
        Layout.topMargin: 4
        Layout.fillWidth: true

        text: description
    }

    // Guides
    Loader {
        id: guideLoader
        Layout.topMargin: 0
        Layout.fillWidth: true

        active: hasGuides
        visible: hasGuides

        sourceComponent: ColumnLayout {
            spacing: 0

            Flow {
                objectName: "guideLayout"
                Layout.topMargin: VPNTheme.theme.vSpacingSmall
                Layout.fillWidth: true
                spacing: 16

                Repeater {
                    id: guideRepeater
                    model: guidesModel
                    delegate: VPNGuideCard {
                        objectName: addon.id

                        height: 172
                        width: parentWidth < VPNTheme.theme.tabletMinimumWidth ? (parent.width - parent.spacing) / 2 : (parent.width - (parent.spacing * 2)) / 3

                        imageSrc: addon.image
                        title: qsTrId(addon.titleId)

                        onClicked:{
                            mainStackView.push("qrc:/ui/settings/ViewGuide.qml", {"guide": addon, "imageBgColor": imageBgColor})
                            VPN.recordGleanEventWithExtraKeys("guideOpened",{
                                                                "id": addon.id
                            });
                        }
                    }
                }
            }
        }
    }

    // Tutorials
    Loader {
        Layout.topMargin: guideLoader.active ? 32 : 16
        Layout.fillWidth: true

        active: hasTutorials
        visible: hasTutorials

        sourceComponent: Flow {
            spacing: 16

            Repeater {
                model: tutorialsModel

                delegate: VPNTutorialCard {
                    width: parentWidth < VPNTheme.theme.tabletMinimumWidth ? parent.width : (parent.width - parent.spacing) / 2
                    height: VPNTheme.theme.tutorialCardHeight

                    imageSrc: addon.image
                    title: qsTrId(addon.titleId)
                    description: qsTrId(addon.subtitleId)
                    onClicked: {
                        VPNTutorial.play(addon);
                        VPNCloseEventHandler.removeAllStackViews();
                    }
                }
            }
        }
    }
}
