/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../themes/themes.js" as Theme

Item {
    id: tour

    signal started()
    signal finished()
    signal close()

    property variant slidesModel: ({})

    implicitHeight: content.implicitHeight
    width: parent.width

    states: [
        State {
            name: "tour-default"
            when: swipeView.currentIndex > 0 && swipeView.currentIndex < swipeView.count - 1

            PropertyChanges {
                target: resumeButton
                text: VPNl18n.tr(VPNl18n.WhatsNewReleaseNotesSupportModalButtonText)
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
                text: VPNl18n.tr(VPNl18n.WhatsNewReleaseNotesTourModalButtonText)
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
                text: VPNl18n.tr(VPNl18n.WhatsNewReleaseNotesDnsModalButtonText)
            }

            PropertyChanges {
                target: slideIndicator
                opacity: 1.0
            }
        }
    ]

    VPNIconButton {
        id: backButton

        anchors.bottom: tour.top
        accessibleName: qsTrId("vpn.main.back")
        enabled: swipeView.currentIndex > 1
        onClicked: {
            swipeView.currentIndex -= 1;
        }
        opacity: swipeView.currentIndex > 1 ? 1 : 0
        x: -Theme.windowMargin
        z: 1

        Image {
            id: backImage

            anchors.centerIn: backButton
            fillMode: Image.PreserveAspectFit
            source: "../resources/back-dark.svg"
            sourceSize.height: Theme.iconSize * 1.5
            sourceSize.width: Theme.iconSize * 1.5
        }

        Behavior on opacity {
            NumberAnimation {
                duration: 100
            }
        }
    }

    ColumnLayout {
        id: content

        spacing: Theme.listSpacing
        width: parent.width

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

                    opacity: slideIndex === swipeView.currentIndex ? 1 : 0
                    spacing: Theme.listSpacing

                    Item {
                        height: parent.width * 0.475
                        width: parent.width

                        Image {
                            anchors.fill: parent
                            source: featureImagePath
                            fillMode: Image.PreserveAspectFit

                            Layout.alignment: Qt.AlignHCenter
                            Layout.bottomMargin: tour.state === "tour-start" ? Theme.listSpacing : Theme.listSpacing * 0.25
                        }
                    }

                    VPNMetropolisLabel {
                        id: popupTitle

                        color: Theme.fontColorDark
                        horizontalAlignment: Text.AlignHCenter
                        font.pixelSize: Theme.fontSizeLarge
                        text: featureName

                        Layout.bottomMargin: Theme.listSpacing
                        Layout.fillWidth: true
                    }

                    VPNTextBlock {
                        horizontalAlignment: Text.AlignHCenter
                        text: featureDescription
                        Layout.fillWidth: true
                    }

                    Behavior on opacity {
                        NumberAnimation {
                            duration: 400
                        }
                    }
                }
            }

            Loader {
                id: initialSlideLoader

                property int slideIndex: 0
                property string featureName: VPNl18n.tr(VPNl18n.WhatsNewReleaseNotesTourModalHeader)
                property string featureDescription: VPNl18n.tr(VPNl18n.WhatsNewReleaseNotesTourModalBodyText)
                property string featureImagePath: "../resources/features/features-tour-hero.png"

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
                    property string featureName: displayName
                    property string featureDescription: description
                    property string featureImagePath: imagePath

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
            spacing: Theme.windowMargin / 2
            visible: swipeView.currentIndex >= 1
            delegate: Rectangle {
                id: circle

                color: index === slideIndicator.currentIndex ? Theme.blue : Theme.greyPressed
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

            radius: Theme.cornerRadius
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignBottom
            Layout.topMargin: slideIndicator.visible ? Theme.listSpacing : Theme.vSpacingSmall

            onClicked: {
                if (tour.state === "tour-start") {
                    tour.started();
                } else if (tour.state === "tour-end") {
                    tour.finished();
                    return;
                }

                swipeView.contentItem.highlightMoveDuration = 250;
                swipeView.currentIndex += 1;
            }

            Image {
                id: buttonIcon

                anchors {
                    right: resumeButton.contentItem.right
                    rightMargin: Theme.windowMargin
                    verticalCenter: resumeButton.verticalCenter
                }
                fillMode: Image.PreserveAspectFit
                source: "../resources/arrow-forward-white.svg"
                sourceSize.height: Theme.iconSize * 1.5
                sourceSize.width: Theme.iconSize * 1.5
                visible: false
            }
        }
    }

    function skipStart() {
        swipeView.contentItem.highlightMoveDuration = 0;
        swipeView.currentIndex = 1;

        tour.started();
    }

    function resetTour() {
        swipeView.contentItem.highlightMoveDuration = 0;
        swipeView.currentIndex = 0;
    }
}
