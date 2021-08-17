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

    property int startIndex: 0
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
                enabled: true
                opacity: 1.0
                target: backButton
            }

            PropertyChanges {
                target: indicator
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
                enabled: false
                opacity: 0
                target: backButton
            }

            PropertyChanges {
                target: indicator
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
                enabled: true
                opacity: 1.0
                target: backButton
            }

            PropertyChanges {
                target: indicator
                opacity: 1.0
            }
        }
    ]

    VPNIconButton {
        id: backButton

        anchors.bottom: tour.top
        accessibleName: "back button"
        onClicked: {
            swipeView.currentIndex -= 1;
        }
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
            currentIndex: startIndex
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
            id: indicator

            count: swipeView.count - 1
            currentIndex: swipeView.currentIndex - 1
            interactive: false
            spacing: Theme.windowMargin / 2
            delegate: Rectangle {
                id: circle

                color: index === indicator.currentIndex ? Theme.blue : Theme.greyPressed
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
                if (tour.state === "end") {
                    swipeView.currentIndex = 0;
                    tour.finished();
                    return;
                }

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
}
