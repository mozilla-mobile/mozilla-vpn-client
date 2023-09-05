/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.15

import Mozilla.Shared 1.0
import components 0.1
import compat 0.1

ColumnLayout {
    id: root

    signal nextClicked()
    signal backClicked()

    spacing: 0

    MZHeadline {
        Layout.topMargin: MZTheme.theme.windowMargin * 1.5
        Layout.leftMargin: MZTheme.theme.windowMargin * 2
        Layout.rightMargin: MZTheme.theme.windowMargin * 2

        Layout.fillWidth: true

        text: MZI18n.OnboardingDevicesSlideHeader
        horizontalAlignment: Text.AlignLeft
    }

    MZInterLabel {
        Layout.topMargin: 8
        Layout.leftMargin: MZTheme.theme.windowMargin * 2
        Layout.rightMargin: MZTheme.theme.windowMargin * 2
        Layout.fillWidth: true

        text: MZI18n.OnboardingDevicesSlideBody
        horizontalAlignment: Text.AlignLeft
        color: MZTheme.theme.fontColor
    }

//Segmented Navigation Implementation
//    MZSegmentedNavigation {

//        Layout.topMargin: 25
//        Layout.leftMargin: 16
//        Layout.rightMargin: 16

//        showDivider: false
//        toggleWidth: 108

//        segmentedToggleButtonsModel: ListModel {
//            ListElement {
//                segmentIconPath: "qrc:/ui/resources/android.svg"
//                //Android icon is too detailed for how small we are presenting it here (24x24) causing it to appear poorly/grainy when behind a ColorOverlay
//                //so we use an alternate svg with the color built in
//                selectedSegmentIconPath: "qrc:/ui/resources/android-active.svg"
//                segmentLabelStringId: "OnboardingDevicesSlideDeviceTypeAndroid"
//                segmentButtonId: "tabAndroid"
//            }
//            ListElement {
//                segmentIconPath: "qrc:/ui/resources/apple.svg"
//                //When ColorOverlay is applied to the apple icon, it "thickens" a little bit which is a somewhat jarring when the transition from unselected -> selected happens
//                //so we use an alternate svg with the color built in
//                selectedSegmentIconPath: "qrc:/ui/resources/apple-active.svg"
//                segmentLabelStringId: "OnboardingDevicesSlideDeviceTypeApple"
//                segmentButtonId: "tabApple"
//            }
//        }

//        //Views in the stack layout must be in the same order as items in the above list model
//        stackContent: [
//            Rectangle {
//                color: MZTheme.theme.white

//                anchors.centerIn: parent
//                Layout.preferredHeight: 152
//                Layout.preferredWidth: 152
//                Layout.maximumHeight: 152
//                Layout.maximumWidth: 152

//                MZDropShadow {
//                    anchors.fill: parent
//                    source: parent

//                    verticalOffset: 1
//                    horizontalOffset: 1
//                    color: MZTheme.colors.grey60
//                    opacity: .1
//                    transparentBorder: true
//                    cached: true
//                }

//                Image {
//                    width: 152
//                    height: 152

//                    source: "qrc:/ui/resources/qrcodes/play-store-qrcode.png"
//                    fillMode: Image.PreserveAspectFit
//                }
//            },

//            Rectangle {
//                color: MZTheme.theme.white

//                anchors.centerIn: parent
//                Layout.preferredHeight: 152
//                Layout.preferredWidth: 152
//                Layout.maximumHeight: 152
//                Layout.maximumWidth: 152

//                MZDropShadow {
//                    anchors.fill: parent
//                    source: parent

//                    verticalOffset: 1
//                    horizontalOffset: 1
//                    color: MZTheme.colors.grey60
//                    opacity: .1
//                    transparentBorder: true
//                    cached: true
//                }

//                Image {
//                    width: 152
//                    height: 152

//                    source: "qrc:/ui/resources/qrcodes/app-store-qrcode.png"
//                    fillMode: Image.PreserveAspectFit
//                }
//            }
//        ]
//    }

//Segmented Toggle Implementation
    RowLayout {
        Layout.topMargin: 25
        Layout.leftMargin: MZTheme.theme.windowMargin * 2
        Layout.rightMargin: MZTheme.theme.windowMargin * 2

        MZBoldInterLabel {
            Layout.alignment: Qt.AlignVCenter

            text: MZI18n.OnboardingDevicesSlideDeviceTypeLabel
            font.pixelSize: MZTheme.theme.fontSize
            lineHeight: MZTheme.theme.labelLineHeight
            verticalAlignment: Text.AlignVCenter
        }

        Item {
            Layout.fillWidth: true
        }

        MZSegmentedToggle {
            id: deviceTypeToggle

            Layout.preferredWidth: 108

            model: ListModel {
                ListElement {
                    segmentIconPath: "qrc:/ui/resources/android.svg"
                    //Android icon is too detailed for how small we are presenting it here (24x24) causing it to appear poorly/grainy when behind a ColorOverlay
                    //so we use an alternate svg with the color built in
                    selectedSegmentIconPath: "qrc:/ui/resources/android-active.svg"
                    segmentLabelStringId: "OnboardingDevicesSlideDeviceTypeAndroid"
                    segmentButtonId: "tabAndroid"
                }
                ListElement {
                    segmentIconPath: "qrc:/ui/resources/apple.svg"
                    //When ColorOverlay is applied to the apple icon, it "thickens" a little bit which is a somewhat jarring when the transition from unselected -> selected happens
                    //so we use an alternate svg with the color built in
                    selectedSegmentIconPath: "qrc:/ui/resources/apple-active.svg"
                    segmentLabelStringId: "OnboardingDevicesSlideDeviceTypeApple"
                    segmentButtonId: "tabApple"
                }
            }
        }
    }

    //Views in the stack layout must be in the same order as items in the above list model
    StackLayout {
        Layout.topMargin: MZTheme.theme.vSpacingSmall
        currentIndex: deviceTypeToggle.selectedIndex

        Rectangle {
            color: MZTheme.theme.white

            anchors.centerIn: parent
            Layout.preferredHeight: 152
            Layout.preferredWidth: 152
            Layout.maximumHeight: 152
            Layout.maximumWidth: 152

            MZDropShadow {
                anchors.fill: parent
                source: parent

                verticalOffset: 1
                horizontalOffset: 1
                color: MZTheme.colors.grey60
                opacity: .1
                transparentBorder: true
                cached: true
            }

            Image {
                width: 152
                height: 152

                source: "qrc:/ui/resources/qrcodes/play-store-qrcode.png"
                fillMode: Image.PreserveAspectFit
            }
        }

        Rectangle {
            color: MZTheme.theme.white

            anchors.centerIn: parent
            Layout.preferredHeight: 152
            Layout.preferredWidth: 152
            Layout.maximumHeight: 152
            Layout.maximumWidth: 152

            MZDropShadow {
                anchors.fill: parent
                source: parent

                verticalOffset: 1
                horizontalOffset: 1
                color: MZTheme.colors.grey60
                opacity: .1
                transparentBorder: true
                cached: true
            }

            Image {
                width: 152
                height: 152

                source: "qrc:/ui/resources/qrcodes/app-store-qrcode.png"
                fillMode: Image.PreserveAspectFit
            }
        }
    }

    Item {
        Layout.fillHeight: true
        Layout.minimumHeight: 32
    }

    MZButton {
        Layout.leftMargin: MZTheme.theme.windowMargin * 2
        Layout.rightMargin: MZTheme.theme.windowMargin * 2
        Layout.fillWidth: true

        width: undefined
        text: MZI18n.GlobalNext

        onClicked: root.nextClicked()
    }

    MZLinkButton {
        Layout.topMargin: 16
        Layout.leftMargin: MZTheme.theme.windowMargin * 2
        Layout.rightMargin: MZTheme.theme.windowMargin * 2
        Layout.bottomMargin: MZTheme.theme.windowMargin
        Layout.fillWidth: true

        implicitHeight: MZTheme.theme.rowHeight

        labelText: MZI18n.GlobalGoBack

        onClicked: root.backClicked()
    }
}
