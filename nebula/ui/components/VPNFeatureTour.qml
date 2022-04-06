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
    signal close()

    property variant slidesModel: ({})
    property string currentFeatureID: ""

    states: [
        State {
            name: "tour-default"
            when: swipeView.currentIndex > 0 && swipeView.currentIndex < swipeView.count - 1

            PropertyChanges {
                target: resumeButton
                text: VPNl18n.WhatsNewReleaseNotesSupportModalButtonText
            }

            PropertyChanges {
                target: slideIndicator
                opacity: 1.0
            }
        },
        State {
            name: "tour-start"
            when: swipeView.currentIndex === 0

            PropertyChanges {
                target: resumeButton
                text: VPNl18n.WhatsNewReleaseNotesTourModalButtonText
            }

            PropertyChanges {
                target: slideIndicator
                opacity: 0
            }

            PropertyChanges {
                target: buttonIcon
                visible: true
            }
        },
        State {
            name: "tour-end"
            when: swipeView.currentIndex === swipeView.count - 1

            PropertyChanges {
                target: resumeButton
                text: VPNl18n.WhatsNewReleaseNotesDnsModalButtonText
            }

            PropertyChanges {
                target: slideIndicator
                opacity: 1.0
            }
        }
    ]

    VPNIconButton {
        id: backButton

        Layout.alignment: Qt.AlignTop
        Layout.preferredHeight: VPNTheme.theme.rowHeight
        Layout.preferredWidth: VPNTheme.theme.rowHeight
        Layout.margins: -VPNTheme.theme.windowMargin
        visible: true
        accessibleName: qsTrId("vpn.main.back")
        enabled: swipeView.currentIndex > 1
        onClicked: {
            swipeView.decrementCurrentIndex();
        }
        opacity: swipeView.currentIndex > 1 ? 1 : 0
        z: 1

        Image {
            id: backImage

            anchors.centerIn: backButton
            fillMode: Image.PreserveAspectFit
            source: "qrc:/nebula/resources/back-dark.svg"
            sourceSize.height: VPNTheme.theme.iconSize * 1.5
            sourceSize.width: VPNTheme.theme.iconSize * 1.5
        }

        Behavior on opacity {
            NumberAnimation {
                duration: 100
            }
        }
    }

    ColumnLayout {
        id: content
        spacing: VPNTheme.theme.listSpacing

        SwipeView {
            id: swipeView

            clip: true
            currentIndex: 0
            interactive: true

            Layout.fillHeight: true
            Layout.fillWidth: true

            Component {
                id: slide

                ColumnLayout {
                    id: slideContent

                    opacity: isCurrentSlide() ? 1 : 0

                    ColumnLayout {
                        spacing: VPNTheme.theme.listSpacing
                        Item {
                            Layout.alignment: Qt.AlignHCenter
                            Layout.preferredHeight: 120
                            Layout.preferredWidth: 120

                            Image {
                                anchors.fill: parent
                                source: featureImagePath
                                fillMode: Image.PreserveAspectFit

                                anchors.horizontalCenter: parent.horizontalCenter
                                anchors.bottom: parent.bottom
                                anchors.bottomMargin: tour.state === "tour-start" ? VPNTheme.theme.listSpacing * 0.5 : VPNTheme.theme.listSpacing * 0.25
                            }
                        }

                        VPNMetropolisLabel {
                            id: popupTitle

                            color: VPNTheme.theme.fontColorDark
                            horizontalAlignment: Text.AlignHCenter
                            font.pixelSize: VPNTheme.theme.fontSizeLarge

                            Layout.bottomMargin: VPNTheme.theme.listSpacing
                            Layout.fillWidth: true
                        }

                        VPNTextBlock {
                            id: popupText

                            horizontalAlignment: Text.AlignHCenter
                            Layout.fillWidth: true
                            Layout.preferredWidth: parent.width
                        }
                    }

                    Behavior on opacity {
                        NumberAnimation {
                            duration: 400
                        }
                    }

                    function isCurrentSlide() {
                        return slideIndex === swipeView.currentIndex;
                    }

                    function updateSlideContent() {
                        popupTitle.text = featureID !== ""
                            ? VPNFeatureList.get(featureID).displayName
                            : VPNl18n.WhatsNewReleaseNotesTourModalHeader;
                        popupText.text = featureID !== ""
                            ? VPNFeatureList.get(featureID).description
                            : VPNl18n.WhatsNewReleaseNotesTourModalBodyText;
                    }

                    Connections {
                        target: tour
                        function onStarted() {
                            if (isCurrentSlide()) {
                                updateSlideContent();
                            }
                        }
                    }

                    Connections {
                        target: swipeView

                        function onCurrentIndexChanged() {
                            if (!isCurrentSlide()) {
                                return;
                            }

                            const featureIdChanged = tour.currentFeatureID !== featureID;
                            if (featureIdChanged) {
                                tour.currentFeatureID = featureID;
                                updateSlideContent();
                            }
                        }
                    }
                }
            }

            Loader {
                id: initialSlideLoader

                property int slideIndex: 0
                property string featureID: ""
                property string featureName: VPNl18n.WhatsNewReleaseNotesTourModalHeader
                property string featureDescription: VPNl18n.WhatsNewReleaseNotesTourModalBodyText
                property string featureImagePath: "qrc:/nebula/resources/features/features-tour-hero.png"

                active: SwipeView.isCurrentItem | SwipeView.isPreviousItem | SwipeView.isNextItem
                asynchronous: true
                sourceComponent: slide
                visible: initialSlideLoader.status === Loader.Ready
            }

            Repeater {
                id: slidesRepeater
                model: slidesModel

                Loader {
                    id: featureSlidesLoader

                    property int slideIndex: index + 1
                    property string featureID: feature.id
                    property string featureName: feature.displayName
                    property string featureDescription: feature.description
                    property string featureImagePath: feature.imagePath

                    active: SwipeView.isCurrentItem | SwipeView.isPreviousItem | SwipeView.isNextItem
                    asynchronous: true
                    sourceComponent: slide
                    visible: featureSlidesLoader.status === Loader.Ready
                }
            }
        }

        PageIndicator {
            id: slideIndicator

            count: swipeView.count - 1
            currentIndex: swipeView.currentIndex - 1
            interactive: false
            spacing: VPNTheme.theme.windowMargin / 2
            visible: swipeView.currentIndex >= 1 && count > 1
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

            Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom

            Behavior on opacity {
                NumberAnimation {
                    duration: 100
                }
            }
        }

        VPNButton {
            id: resumeButton

            radius: VPNTheme.theme.cornerRadius
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignBottom
            Layout.topMargin: slideIndicator.visible ? VPNTheme.theme.listSpacing : VPNTheme.theme.vSpacingSmall

            onClicked: {
                if (tour.state === "tour-start") {
                    tour.started();
                } else if (tour.state === "tour-end") {
                    tour.finished();
                    return;
                }

                swipeView.contentItem.highlightMoveDuration = 250;
                swipeView.incrementCurrentIndex();
            }

            Image {
                id: buttonIcon

                anchors {
                    right: resumeButton.contentItem.right
                    rightMargin: VPNTheme.theme.windowMargin
                    verticalCenter: resumeButton.verticalCenter
                }
                fillMode: Image.PreserveAspectFit
                source: "qrc:/nebula/resources/arrow-forward-white.svg"
                sourceSize.height: VPNTheme.theme.iconSize * 1.5
                sourceSize.width: VPNTheme.theme.iconSize * 1.5
                visible: false
            }
        }

        VPNLinkButton {
            id: linkButton

            labelText: VPNl18n.SplittunnelInfoLinkText
            visible: hasFeatureLinkUrl()
            Layout.fillWidth: true
            Layout.topMargin: VPNTheme.theme.listSpacing * 0.5

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

    function skipStart() {
        swipeView.contentItem.highlightMoveDuration = 0;
        swipeView.setCurrentIndex(1);

        tour.started();
    }

    function resetTour() {
        swipeView.contentItem.highlightMoveDuration = 0;
        swipeView.setCurrentIndex(0);
    }
}
