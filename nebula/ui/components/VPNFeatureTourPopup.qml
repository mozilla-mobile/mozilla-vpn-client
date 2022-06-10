/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import compat 0.1

VPNPopup {
    id: popup

    closeButtonObjectName: "featureTourPopupCloseButton"
    startContentBeneathCloseButton: false
    bottomPadding: VPNTheme.theme.windowMargin

    popupContentItem.implicitHeight: layout.implicitHeight

    ColumnLayout {
        id: layout

        anchors.fill: parent

        spacing: 0

        RowLayout {
            spacing: 0

            // Back button
            VPNIconButton {
                id: backButton

                property bool showBackButton: featureTour.currentIndex !== 0

                Layout.leftMargin: VPNTheme.theme.windowMargin / 2
                Layout.topMargin: VPNTheme.theme.windowMargin / 2
                Layout.preferredHeight: VPNTheme.theme.rowHeight
                Layout.preferredWidth: VPNTheme.theme.rowHeight
                Layout.alignment: Qt.AlignRight

                objectName: "backButton"

                accessibleName: qsTrId("vpn.main.back")
                onClicked: featureTour.goBack()
                enabled: showBackButton

                Accessible.ignored: !showBackButton

                Image {
                    anchors.centerIn: parent

                    visible: parent.showBackButton
                    fillMode: Image.PreserveAspectFit
                    source: "qrc:/nebula/resources/back-dark.svg"
                    sourceSize.height: VPNTheme.theme.iconSize * 1.5
                    sourceSize.width: VPNTheme.theme.iconSize * 1.5
                }
            }

            Item {
                Layout.fillWidth: true
            }
        }

        VPNFeatureTour {
            id: featureTour

            Layout.leftMargin: VPNTheme.theme.windowMargin * 1.5
            Layout.rightMargin: VPNTheme.theme.windowMargin * 1.5

            slidesModel: VPNWhatsNewModel

            onFinished: popup.close()
            onStarted: VPNWhatsNewModel.markFeaturesAsSeen()
        }
    }

    function startTour() {
        featureTour.resetTour()
        open()
    }
}

