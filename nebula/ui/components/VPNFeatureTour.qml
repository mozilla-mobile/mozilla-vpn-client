/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0

ColumnLayout {
    id: tour

    signal started()
    signal finished()

    property variant slidesModel
    property string currentFeatureID: ""
    readonly property int currentIndex: swipeView.currentIndex

    onStarted: {
        //set the feature ID (specifically for getting the link if the feature has one)
        if(slidesModel.rowCount() > 0) currentFeatureID = slidesRepeater.itemAt(0).featureID
    }

    ColumnLayout {
        id: content
        spacing: 0

        SwipeView {
            id: swipeView

            objectName: "featureSwipeView"

            clip: true
            currentIndex: 0
            interactive: true

            Layout.fillWidth: true

            onCurrentIndexChanged: {
                tour.currentFeatureID = slidesRepeater.itemAt(currentIndex).featureID
            }


            Repeater {
                id: slidesRepeater
                model: slidesModel

                delegate: Loader {

                    property string featureID: feature.id

                    asynchronous: true
                    visible: status == Loader.Ready
                    opacity: index === swipeView.currentIndex ? 1 : 0

                    Behavior on opacity {
                        NumberAnimation {
                            duration: 400
                        }
                    }

                    sourceComponent: ColumnLayout {
                        id: slideContent

                        ColumnLayout {
                            spacing: VPNTheme.theme.listSpacing

                            Image {
                                Layout.alignment: Qt.AlignHCenter

                                source: feature.imagePath
                                sourceSize.width: 104
                                sourceSize.height: 120
                            }

                            VPNMetropolisLabel {
                                id: popupTitle

                                text: feature.displayName
                                color: VPNTheme.theme.fontColorDark
                                horizontalAlignment: Text.AlignHCenter
                                font.pixelSize: VPNTheme.theme.fontSizeLarge

                                Layout.bottomMargin: VPNTheme.theme.listSpacing
                                Layout.fillWidth: true

                                Accessible.ignored: index !== swipeView.currentIndex
                            }

                            VPNTextBlock {
                                id: popupText

                                text: feature.description
                                horizontalAlignment: Text.AlignHCenter
                                Layout.fillWidth: true

                                Accessible.ignored: index !== swipeView.currentIndex
                            }
                        }

                        Item {
                            Layout.fillHeight: true
                        }
                    }
                }
            }
        }

        PageIndicator {
            id: slideIndicator

            Layout.topMargin: VPNTheme.theme.vSpacingSmall
            Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom

            count: swipeView.count
            currentIndex: swipeView.currentIndex
            interactive: false
            spacing: VPNTheme.theme.windowMargin / 2
            visible: count > 1
            delegate: Rectangle {
                id: circle

                color: index === slideIndicator.currentIndex ? VPNTheme.theme.blue : VPNTheme.theme.greyPressed
                height: 6
                width: 6
                radius: 6

                Behavior on color {
                    ColorAnimation {
                        duration: 400
                    }
                }
            }

            Behavior on opacity {
                NumberAnimation {
                    duration: 100
                }
            }
        }

        VPNButton {
            id: resumeButton

            objectName: "featureTourPrimaryButton"

            property bool isLastFeature: swipeView.currentIndex === swipeView.count - 1

            text: isLastFeature ? VPNl18n.WhatsNewReleaseNotesDnsModalButtonText : VPNl18n.WhatsNewReleaseNotesSupportModalButtonText
            radius: VPNTheme.theme.cornerRadius
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignBottom
            Layout.topMargin: slideIndicator.visible ? VPNTheme.theme.vSpacing : VPNTheme.theme.vSpacingSmall
            Layout.bottomMargin: linkButton.visible ? VPNTheme.theme.vSpacingSmall / 2 : VPNTheme.theme.windowMargin / 2

            onClicked: {
                if(isLastFeature) {
                    tour.finished()
                    return
                }

                swipeView.contentItem.highlightMoveDuration = 250;
                swipeView.incrementCurrentIndex();
            }
        }

        VPNLinkButton {
            id: linkButton

            objectName: "featureTourSecondaryButton"

            labelText: VPNl18n.SplittunnelInfoLinkText
            visible: hasFeatureLinkUrl()
            Layout.fillWidth: true

            onClicked: {
                const featureLinkUrl = VPNFeatureList.get(currentFeatureID).linkUrl
                VPN.openLinkUrl(featureLinkUrl);
            }

            function hasFeatureLinkUrl() {
                const isFeature = tour.currentFeatureID && tour.currentFeatureID.length > 0;
                if (!isFeature) {
                    return false;
                }

                const currentFeature = VPNFeatureList.get(currentFeatureID);
                return currentFeature && currentFeature.linkUrl;
            }
        }
    }

    function resetTour() {
        swipeView.contentItem.highlightMoveDuration = 0;
        swipeView.setCurrentIndex(0);

        tour.started();

        calculateTallestSlideHeight()
    }

    //Calculate which slide is the tallest so we can set that height to the swipe view, this way the modal does not change size
    function calculateTallestSlideHeight() {
        var tallestSlideHeight = 0
        for(var i = 0; i < slidesRepeater.count; i++) {
            slidesRepeater.itemAt(i).active = false
            slidesRepeater.itemAt(i).active = true
            const slideHeight = slidesRepeater.itemAt(i).implicitHeight
            if (slideHeight > tallestSlideHeight) tallestSlideHeight = slideHeight
        }
        swipeView.Layout.preferredHeight = Qt.binding(function() { return linkButton.visible ? tallestSlideHeight - linkButton.height : tallestSlideHeight })
    }

    function goBack() {
        swipeView.contentItem.highlightMoveDuration = 250;
        swipeView.decrementCurrentIndex();
    }

    //Hacky workaround to recalculate tallest slide height after a language change
    Timer {
        id: calculateTallestSlideTimer
        interval: 50
        repeat: false
        onTriggered: {
            for(var i = 0; i < slidesRepeater.count; i++) {
                slidesRepeater.itemAt(i).active = false
                slidesRepeater.itemAt(i).active = true
            }
            calculateTallestSlideHeight()
        }
    }

    onVisibleChanged: if(visible) calculateTallestSlideTimer.start()
}
