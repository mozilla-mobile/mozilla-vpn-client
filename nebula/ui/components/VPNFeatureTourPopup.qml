/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import compat 0.1

Popup {
    id: popup

    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    enabled: true
    focus: true
    modal: true
    //width should have the same horizontal margins from the window as the content behind it for phones. Anything bigger than 500 is probably a tablet
    width: Math.min(window.width - (VPNTheme.theme.windowMargin * 2), 500)
    topPadding: VPNTheme.theme.windowMargin / 2
    bottomPadding: VPNTheme.theme.windowMargin
    horizontalPadding: VPNTheme.theme.windowMargin / 2

    contentItem: Item {

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            RowLayout {
                spacing: 0

                // Back button
                VPNIconButton {
                    objectName: "backButton"

                    accessibleName: qsTrId("vpn.main.back")
                    visible: featureTour.currentIndex !== 0
                    onClicked: featureTour.goBack()

                    Layout.preferredHeight: VPNTheme.theme.rowHeight
                    Layout.preferredWidth: VPNTheme.theme.rowHeight
                    Layout.alignment: Qt.AlignRight

                    Image {
                        anchors.centerIn: parent
                        fillMode: Image.PreserveAspectFit
                        source: "qrc:/nebula/resources/back-dark.svg"
                        sourceSize.height: VPNTheme.theme.iconSize * 1.5
                        sourceSize.width: VPNTheme.theme.iconSize * 1.5
                    }
                }

                Item {
                    Layout.fillWidth: true
                }

                // Close button
                VPNIconButton {
                    objectName: "closeButton"

                    accessibleName: qsTrId("menubar.file.close")
                    onClicked: {
                        popup.close();
                    }

                    Layout.preferredHeight: VPNTheme.theme.rowHeight
                    Layout.preferredWidth: VPNTheme.theme.rowHeight
                    Layout.alignment: Qt.AlignRight

                    Image {
                        anchors.centerIn: parent
                        fillMode: Image.PreserveAspectFit
                        source: "qrc:/nebula/resources/close-darker.svg"
                        sourceSize.height: VPNTheme.theme.iconSize
                        sourceSize.width: VPNTheme.theme.iconSize
                    }
                }

            }

            ColumnLayout {
                id: contentLayout
                Layout.leftMargin: VPNTheme.theme.windowMargin
                Layout.rightMargin: VPNTheme.theme.windowMargin
                Layout.fillHeight: true

                VPNFeatureTour {
                    id: featureTour

                    slidesModel: VPNWhatsNewModel

                    onFinished: {
                        popup.close();
                    }
                    onStarted: {
                        VPNWhatsNewModel.markFeaturesAsSeen();
                    }
                }
            }
        }
    }

    background: Rectangle {
        id: popupBackground

        anchors.fill: parent
        color: VPNTheme.theme.bgColor
        radius: 8

        Rectangle {
            id: popUpShadowSource
            anchors.fill: popupBackground
            radius: popupBackground.radius
            z: -1
        }

        VPNDropShadow {
            id: popupShadow

            anchors.fill: popUpShadowSource
            cached: true
            color: "black"
            opacity: 0.2
            radius: 16
            source: popUpShadowSource
            spread: 0.1
            transparentBorder: true
            verticalOffset: 4
            z: -1
        }
    }

    enter: Transition {
        NumberAnimation {
            property: "opacity"
            duration: 120
            from: 0.0
            to: 1.0
            easing.type: Easing.InOutQuad
        }
    }

    exit: Transition {
        NumberAnimation {
            property: "opacity"
            duration: 120
            from: 1.0
            to: 0.0
            easing.type: Easing.InOutQuad
        }
    }

    Overlay.modal: Rectangle {
        id: overlayBackground

        color: VPNTheme.theme.overlayBackground

        Behavior on opacity {
            NumberAnimation {
                duration: 175
            }
        }
    }

    function startTour() {
        featureTour.resetTour()
        open()
    }
}

