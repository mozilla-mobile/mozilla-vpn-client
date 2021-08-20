/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import "../themes/themes.js" as Theme

Item {
    id: tour

    signal started()
    signal finished()
    signal close()

    property variant slidesData: ({})

    implicitHeight: content.implicitHeight
    width: parent.width

    states: [
        State {
            name: "default"
            when: swipeView.currentIndex > 0 && swipeView.currentIndex < swipeView.count - 1

            PropertyChanges {
                target: resumeButton
                text: "Next"
            }

            PropertyChanges {
                target: slideIndicator
                opacity: 1.0
            }
        },
        State {
            name: "start"
            when: swipeView.currentIndex === 0

            PropertyChanges {
                target: resumeButton
                text: "Take the tour"
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
            name: "end"
            when: swipeView.currentIndex === swipeView.count - 1

            PropertyChanges {
                target: resumeButton
                text: "Done"
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
        accessibleName: "back button"
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
                    id: content

                    opacity: slideIndex === swipeView.currentIndex ? 1 : 0
                    spacing: Theme.vSpacingSmall

                    Image {
                        source: slideData.imageSrc
                        sourceSize.height: parent.height / 2.5
                        sourceSize.width: parent.height / 2.5

                        Layout.alignment: Qt.AlignHCenter
                        Layout.topMargin: Theme.listSpacing
                    }

                    VPNMetropolisLabel {
                        id: popupTitle

                        color: Theme.fontColorDark
                        horizontalAlignment: Text.AlignHCenter
                        font.pixelSize: Theme.fontSizeLarge
                        text: slideData.title

                        Layout.fillWidth: true
                    }

                    VPNTextBlock {
                        horizontalAlignment: Text.AlignHCenter
                        text: slideData.text
                        Layout.fillWidth: true
                    }

                    Behavior on opacity {
                        NumberAnimation {
                            duration: 400
                        }
                    }
                }
            }

            Repeater {
                id: slidesRepeater
                model: slidesData

                Loader {
                    id: slideLoader

                    property int slideIndex: index
                    property variant slideData: modelData

                    active: SwipeView.isCurrentItem | SwipeView.isPreviousItem | SwipeView.isNextItem
                    asynchronous: true
                    sourceComponent: slide
                    visible: slideLoader.status === Loader.Ready
                }
            }
        }

        PageIndicator {
            id: slideIndicator

            count: swipeView.count - 1
            currentIndex: swipeView.currentIndex - 1
            interactive: false
            spacing: Theme.windowMargin / 2
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
            Layout.bottomMargin: Theme.listSpacing

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

            onClicked: {
                if (tour.state === "start") {
                } else if (tour.state === "end") {
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
    }

    function resetTour() {
        swipeView.contentItem.highlightMoveDuration = 0;
        swipeView.currentIndex = 0;
    }
}
