/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import Mozilla.VPN.qmlcomponents 1.0
import components 0.1

Item {
    id: root

    property var guide
    property alias imageBgColor: imageBg.color
    property int safeAreaHeight: window.safeAreaHeightByDevice()
    readonly property double timeOfOpen: new Date().getTime()

    ColumnLayout {
        anchors.fill: parent

        spacing: 0

        Rectangle {
            id: imageBg

            Layout.fillWidth: true
            Layout.preferredHeight: window.height * 0.33
            Layout.topMargin: -safeAreaHeight //we want to cover the safe area with the image background color

            MZIconButton {
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.topMargin: root.safeAreaHeight + 8
                anchors.leftMargin: MZTheme.theme.listSpacing
                buttonColorScheme: {
                    'defaultColor': MZTheme.theme.transparent,
                    'buttonHovered': "#1AFFFFFF",
                    'buttonPressed': '#33FFFFFF',
                    'focusOutline': MZTheme.theme.transparent,
                    'focusBorder': MZTheme.theme.lightFocusBorder
                }
                skipEnsureVisible: true
                //% "Back"
                //: Go back
                accessibleName: qsTrId("vpn.main.back")

                onClicked: {
                    statusBarModifier.resetDefaults()
                    stackview.pop()
                }

                Image {
                    objectName: "backArrow"

                    anchors.centerIn: parent

                    source: "qrc:/nebula/resources/arrow-back-white.svg"
                    sourceSize.width: MZTheme.theme.iconSize * 1.5
                    fillMode: Image.PreserveAspectFit
                }
            }

            //Center image in background not including the iOS safe area
            ColumnLayout {
                anchors.fill: parent
                anchors.topMargin: root.safeAreaHeight

                spacing: 0

                Item {
                    Layout.fillHeight: true
                }

                Image {
                    Layout.alignment: Qt.AlignHCenter

                    source: guide.image
                    sourceSize.height: imageBg.height * 0.66
                    fillMode: Image.PreserveAspectFit
                }

                Item {
                    Layout.fillHeight: true
                }
            }

            MZMobileStatusBarModifier {
                id: statusBarModifier
                statusBarTextColor: MZTheme.StatusBarTextColorLight
            }
        }

        MZFlickable {
            id: vpnFlickable
            Layout.fillWidth: true
            Layout.fillHeight: true

            flickContentHeight: layout.implicitHeight + layout.anchors.topMargin

            Column {
                id: layout
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.topMargin: MZTheme.theme.vSpacingSmall
                anchors.leftMargin: MZTheme.theme.windowMargin
                anchors.rightMargin: MZTheme.theme.windowMargin

                ColumnLayout {
                    anchors.left: parent.left
                    anchors.right: parent.right

                    spacing: 0

                    MZBoldInterLabel {
                        Layout.fillWidth: true

                        text: guide.advanced ? MZI18n.TipsAndTricksAdvancedTipsGuideViewTitle : MZI18n.TipsAndTricksQuickTipsGuideViewTitle
                        font.pixelSize: MZTheme.theme.fontSize
                        lineHeight: MZTheme.theme.labelLineHeight
                        color: MZTheme.theme.fontColor
                    }

                    MZBoldLabel {
                        Layout.topMargin: MZTheme.theme.vSpacingSmall
                        Layout.fillWidth: true

                        text: guide.title
                        lineHeightMode: Text.FixedHeight
                        lineHeight: MZTheme.theme.vSpacing
                        wrapMode: Text.Wrap
                        verticalAlignment: Text.AlignVCenter
                    }

                    MZInterLabel {
                        Layout.topMargin: MZTheme.theme.listSpacing
                        Layout.fillWidth: true

                        visible: guide.subtitle

                        text: guide.subtitle
                        font.pixelSize: MZTheme.theme.fontSizeSmall
                        color: MZTheme.theme.fontColor
                        horizontalAlignment: Text.AlignLeft
                    }

                    Rectangle {
                        Layout.topMargin: MZTheme.theme.vSpacingSmall
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1

                        visible: guide.subtitle

                        color: MZTheme.colors.grey10
                    }

                    MZComposerView {
                        addon: guide
                        view: MZComposerView.View.Guide
                    }
                }
            }
        }
    }

    Connections {
        function onGoBack(item) {
            if (item === root && root.visible)
                stackview.pop();
        }

        target: MZNavigator
    }

    Component.onCompleted: MZNavigator.addView(VPN.ScreenSettings, root)

    Component.onDestruction: {
        MZGleanDeprecated.recordGleanEventWithExtraKeys("guideClosed",{
                                          "id": guide.id,
                                          "duration_ms": new Date().getTime() - timeOfOpen
        });
        Glean.sample.guideClosed.record({
            "id": guide.id,
            "duration_ms": new Date().getTime() - timeOfOpen
        });
    }
}
