/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import components 0.1
import "qrc:/nebula/utils/MZAssetLookup.js" as MZAssetLookup

MZViewBase {
    _menuTitle: "UI Testing"
    _interactive: false
    _viewContentData: ColumnLayout {
        spacing: MZTheme.theme.windowMargin
        Layout.fillWidth: true

        MZSettingsItem {
            settingTitle: "Image playground"
            imageRightSrc: MZAssetLookup.getImageSource("Chevron")
            imageRightMirror: MZLocalizer.isRightToLeft
            onClicked: getHelpStackView.push("qrc:/qt/qml/Mozilla/VPN/screens/getHelp/developerMenu/ViewImagesPlayground.qml")
            Layout.leftMargin: MZTheme.theme.windowMargin / 2
            Layout.rightMargin: MZTheme.theme.windowMargin / 2
        }

        MZSettingsItem {
            settingTitle: "Animations playground"
            imageRightSrc: MZAssetLookup.getImageSource("Chevron")
            imageRightMirror: MZLocalizer.isRightToLeft
            onClicked: getHelpStackView.push("qrc:/qt/qml/Mozilla/VPN/screens/getHelp/developerMenu/ViewAnimationsPlayground.qml")
            Layout.leftMargin: MZTheme.theme.windowMargin / 2
            Layout.rightMargin: MZTheme.theme.windowMargin / 2
        }

        MZSettingsItem {
            settingTitle: "MZStepNavigation"
            imageRightSrc: MZAssetLookup.getImageSource("Chevron")
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
            _interactive: false
            _viewContentData: MZStepNavigation {
                id: stepNav

                Layout.topMargin: 8
                Layout.fillWidth: true

                Component.onCompleted: navbar.visible = false

                views: [
                    ColumnLayout {
                        property string labelText: "OnboardingProgressBarDataUse"
                        property string iconSource: MZAssetLookup.getImageSource("IconLock")

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
                        property string iconSource: MZAssetLookup.getImageSource("IconPrivacyMask")

                        spacing: 0

                        MZHeadline {
                            Layout.topMargin: MZTheme.theme.windowMargin * 1.5
                            Layout.leftMargin: MZTheme.theme.windowMargin * 2
                            Layout.rightMargin: MZTheme.theme.windowMargin * 2

                            Layout.fillWidth: true

                            text: "Get more privacy"
                            horizontalAlignment: Text.AlignLeft
                        }

                        MZInterLabel {
                            Layout.topMargin: 8
                            Layout.leftMargin: MZTheme.theme.windowMargin * 2
                            Layout.rightMargin: MZTheme.theme.windowMargin * 2
                            Layout.fillWidth: true

                            text: "Use these features for more protection. They may cause issues on some sites, so you can turn them off anytime in settings."
                            horizontalAlignment: Text.AlignLeft
                            color: MZTheme.colors.fontColor
                        }

                        MZCheckBoxRow {
                            Layout.topMargin: 24
                            Layout.leftMargin: MZTheme.theme.windowMargin * 2
                            Layout.rightMargin: MZTheme.theme.windowMargin * 2
                            leftMargin: 0

                            labelText: "Block ads"
                            subLabelText: "Reduces the amount of ads you see when you’re using Mozilla VPN"
                            showDivider: false
                        }

                        MZCheckBoxRow {
                            Layout.topMargin: 8
                            Layout.leftMargin: MZTheme.theme.windowMargin * 2
                            Layout.rightMargin: MZTheme.theme.windowMargin * 2
                            leftMargin: 0

                            labelText: "Block trackers"
                            subLabelText: "Fewer harmful domains will be able to track you"
                            showDivider: false
                        }

                        MZCheckBoxRow {
                            Layout.topMargin: 8
                            Layout.leftMargin: MZTheme.theme.windowMargin * 2
                            Layout.rightMargin: MZTheme.theme.windowMargin * 2
                            leftMargin: 0

                            labelText: "Block malware"
                            subLabelText: "Helps protect you from ransomware, phishing, spyware, viruses, and malware"
                            showDivider: false
                        }

                        Item {
                            Layout.fillHeight: true
                            Layout.minimumHeight: 20
                        }

                        MZButton {
                            Layout.leftMargin: MZTheme.theme.windowMargin * 2
                            Layout.rightMargin: MZTheme.theme.windowMargin * 2
                            Layout.fillWidth: true

                            width: undefined
                            text: "Continue"

                            onClicked: {
                                stepNav.next()
                            }
                        }

                        MZLinkButton {
                            Layout.topMargin: 16
                            Layout.leftMargin: MZTheme.theme.windowMargin * 2
                            Layout.rightMargin: MZTheme.theme.windowMargin * 2
                            Layout.bottomMargin: MZTheme.theme.windowMargin
                            Layout.fillWidth: true

                            implicitHeight: MZTheme.theme.rowHeight

                            labelText: "Go back"

                            onClicked: {
                                stepNav.back()
                            }
                        }

                    },
                    ColumnLayout {
                        property string labelText: "OnboardingProgressBarAddDevices"
                        property string iconSource: MZAssetLookup.getImageSource("IconDevices")

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
                        property string labelText: "OnboardingProgressBarGetStarted2"
                        property string iconSource: MZAssetLookup.getImageSource("IconPowerButton")

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

