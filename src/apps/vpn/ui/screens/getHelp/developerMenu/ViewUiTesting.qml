/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import components 0.1

MZViewBase {
    _menuTitle: "UI Testing"
    _viewContentData: ColumnLayout {
        spacing: MZTheme.theme.windowMargin
        Layout.fillWidth: true

        MZSettingsItem {
            settingTitle: "MZStepNavigation"
            imageRightSrc: "qrc:/nebula/resources/chevron.svg"
            imageRightMirror: MZLocalizer.isRightToLeft
            onClicked: getHelpStackView.push(stepNavComponent)
            Layout.leftMargin: MZTheme.theme.windowMargin / 2
            Layout.rightMargin: MZTheme.theme.windowMargin / 2
        }
    }

    Component {
        id: stepNavComponent
        MZViewBase {
            _menuTitle: "MZStepNavigation"
            _viewContentData: ColumnLayout {

                spacing: 32

                MZStepNavigation {
                    id: stepNav

                    views: [
                        ColumnLayout {

                            property string labelText: "OnboardingProgressBarDataUse"
                            property string iconSource: "qrc:/nebula/resources/lock.svg"

                            spacing: 24

                            Item {
                                Layout.fillHeight: true
                            }

                            Text {
                                Layout.alignment: Qt.AlignHCenter
                                text: "Slide 1"
                            }

                            MZButton {
                                text: "Next"
                                onClicked: {
                                    stepNav.next()
                                }
                            }

                            Item {
                                Layout.fillHeight: true
                            }

                        },
                        ColumnLayout {

                            property string labelText: "OnboardingProgressBarMorePrivacy"
                            property string iconSource: "qrc:/ui/resources/settings/privacy.svg"

                            spacing: 24

                            Item {
                                Layout.fillHeight: true
                            }

                            Text {
                                Layout.alignment: Qt.AlignHCenter
                                text: "Slide 2"
                            }

                            MZButton {
                                text: "Next"
                                onClicked: {
                                    stepNav.next()
                                }
                            }

                            MZLinkButton {
                                Layout.fillWidth: true
                                labelText: "Back"
                                onClicked: {
                                    stepNav.back()
                                }
                            }

                            Item {
                                Layout.fillHeight: true
                            }

                        },
                        ColumnLayout {

                            property string labelText: "OnboardingProgressBarAddDevices"
                            property string iconSource: "qrc:/ui/resources/devices.svg"

                            spacing: 24

                            Item {
                                Layout.fillHeight: true
                            }

                            Text {
                                Layout.alignment: Qt.AlignHCenter
                                text: "Slide 3"
                            }

                            MZButton {
                                text: "Next"
                                onClicked: {
                                    stepNav.next()
                                }
                            }

                            MZLinkButton {
                                Layout.fillWidth: true
                                labelText: "Back"
                                onClicked: {
                                    stepNav.back()
                                }
                            }

                            Item {
                                Layout.fillHeight: true
                            }

                        },
                        ColumnLayout {

                            property string labelText: "OnboardingProgressBarGetStarted"
                            property string iconSource: "qrc:/nebula/resources/startup.svg"

                            spacing: 24

                            Item {
                                Layout.fillHeight: true
                            }

                            Text {
                                Layout.alignment: Qt.AlignHCenter
                                text: "Slide 4"
                            }

                            MZButton {
                                text: "Back"
                                onClicked: {
                                    stepNav.back()
                                }
                            }

                            Item {
                                Layout.fillHeight: true
                            }
                        }
                    ]
                }

            }
        }
    }
}

