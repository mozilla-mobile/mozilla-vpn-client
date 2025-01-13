/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import Mozilla.Shared 1.0
import components 0.1
import compat 0.1
import "qrc:/nebula/utils/MZUiUtils.js" as MZUiUtils
import "qrc:/nebula/utils/MZAssetLookup.js" as MZAssetLookup

ColumnLayout {
    id: root
    objectName: "onboardingDevicesSlide"

    property string telemetryScreenId: "install_on_5_devices"

    signal nextClicked()
    signal backClicked()

    spacing: 0

    MZHeadline {
        Layout.topMargin: MZTheme.theme.windowMargin * 1.5
        Layout.leftMargin: MZTheme.theme.windowMargin * 1.5
        Layout.rightMargin: MZTheme.theme.windowMargin * 1.5

        Layout.fillWidth: true

        text: MZI18n.OnboardingDevicesSlideHeader
        horizontalAlignment: Text.AlignLeft
    }

    MZInterLabel {
        Layout.topMargin: 8
        Layout.leftMargin: MZTheme.theme.windowMargin * 1.5
        Layout.rightMargin: MZTheme.theme.windowMargin * 1.5
        Layout.fillWidth: true

        text: MZUiUtils.isMobile() ? MZI18n.OnboardingDevicesSlideBodyMobile : MZI18n.OnboardingDevicesSlideBody2
        horizontalAlignment: Text.AlignLeft
        color: MZTheme.colors.fontColor
    }

    RowLayout {
        Layout.topMargin: MZTheme.theme.vSpacingSmall
        Layout.leftMargin: MZTheme.theme.windowMargin * 1.5
        Layout.rightMargin: MZTheme.theme.windowMargin * 1.5

        MZBoldInterLabel {
            Layout.alignment: Qt.AlignVCenter
            Layout.fillWidth: true

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
            objectName: "deviceTypeToggle"

            Layout.preferredWidth: 108

            model: ListModel {
                ListElement {
                    segmentIconPath: "PlatformAndroidInactive"
                    //Android icon is too detailed for how small we are presenting it here (24x24) causing it to appear poorly/grainy when behind a ColorOverlay
                    //so we use an alternate image with the color built in
                    selectedSegmentIconPath: "PlatformAndroidActive"
                    segmentLabelStringId: "OnboardingDevicesSlideDeviceTypeAndroid"
                    segmentButtonId: "tabAndroid"
                }
                ListElement {
                    segmentIconPath: "PlatformAppleInactive"
                    //When ColorOverlay is applied to the apple icon, it "thickens" a little bit which is a somewhat jarring when the transition from unselected -> selected happens
                    //so we use an alternate image with the color built in
                    selectedSegmentIconPath: "PlatformAppleActive"
                    segmentLabelStringId: "OnboardingDevicesSlideDeviceTypeApple"
                    segmentButtonId: "tabApple"
                }
            }

            handleSegmentClick: () => {
                switch (selectedIndex) {
                    case 0:
                        Glean.interaction.showAndroidQrSelected.record({
                            screen: root.telemetryScreenId,
                        });
                        break
                    case 1:
                        Glean.interaction.showIosQrSelected.record({
                            screen: root.telemetryScreenId,
                        });
                        break
                    default:
                        break
                }
            }

            Component.onCompleted: {
                switch(Qt.platform.os) {
                    case "osx":
                    case "ios":
                        selectedIndex = 1
                        break
                    case "windows":
                    case "linux":
                    case "android":
                    default:
                        selectedIndex = 0
                        break
                }
            }
        }
    }

    Item {
        Layout.fillHeight: true
        Layout.minimumHeight: MZTheme.theme.onboardingMinimumVerticalSpacing
    }

    //Views in the stack layout must be in the same order as items in the above list model
    StackLayout {
        id: qrcodeStack
        readonly property int qrcodeSize: MZUiUtils.isLargePhone() ? 208 : 152

        Layout.maximumHeight: qrcodeSize
        Layout.maximumWidth: qrcodeSize
        Layout.minimumHeight: qrcodeSize
        Layout.minimumWidth: qrcodeSize
        Layout.alignment: Qt.AlignHCenter

        currentIndex: deviceTypeToggle.selectedIndex

        //Google play store QR code
        Rectangle {
            color: MZTheme.colors.bgColorStronger

            Layout.preferredHeight: qrcodeStack.qrcodeSize
            Layout.preferredWidth: qrcodeStack.qrcodeSize
            Layout.maximumHeight: qrcodeStack.qrcodeSize
            Layout.maximumWidth: qrcodeStack.qrcodeSize

            MZDropShadow {
                anchors.fill: parent
                source: parent

                verticalOffset: 1
                horizontalOffset: 1
                color: MZTheme.colors.dropShadow
                opacity: .1
                transparentBorder: true
                cached: true
            }

            Image {
                objectName: "playStoreQrCode"

                height: qrcodeStack.qrcodeSize
                width: qrcodeStack.qrcodeSize

                //QR coded generated via Adobe Express
                source: MZAssetLookup.getImageSource("PlayStoreQRCode")
                fillMode: Image.PreserveAspectFit

                Accessible.role: Accessible.Graphic
                Accessible.name: MZI18n.OnboardingDevicesSlideQRCodeAndroid
                Accessible.ignored: !visible
            }
        }

        //Apple app store QR code
        Rectangle {
            color: MZTheme.colors.bgColorStronger

            Layout.preferredHeight: qrcodeStack.qrcodeSize
            Layout.preferredWidth: qrcodeStack.qrcodeSize
            Layout.maximumHeight: qrcodeStack.qrcodeSize
            Layout.maximumWidth: qrcodeStack.qrcodeSize

            MZDropShadow {
                anchors.fill: parent
                source: parent

                verticalOffset: 1
                horizontalOffset: 1
                color: MZTheme.colors.dropShadow
                opacity: .1
                transparentBorder: true
                cached: true
            }

            Image {
                objectName: "appStoreQrCode"

                height: qrcodeStack.qrcodeSize
                width: qrcodeStack.qrcodeSize

                //QR coded generated via Adobe Express
                source: MZAssetLookup.getImageSource("AppStoreQRCode")
                fillMode: Image.PreserveAspectFit

                Accessible.role: Accessible.Graphic
                Accessible.name: MZI18n.OnboardingDevicesSlideQRCodeApple
                Accessible.ignored: !visible
            }
        }
    }

    Item {
        Layout.fillHeight: true
        Layout.minimumHeight: MZTheme.theme.onboardingMinimumVerticalSpacing
    }

    MZButton {
        objectName: "devicesNextButton"

        Layout.leftMargin: MZTheme.theme.windowMargin * 2
        Layout.rightMargin: MZTheme.theme.windowMargin * 2
        Layout.fillWidth: true

        width: undefined
        text: MZI18n.GlobalNext

        onClicked: {
            Glean.interaction.continueSelected.record({
                screen: root.telemetryScreenId,
            });

            root.nextClicked()
        }
    }

    MZLinkButton {
        objectName: "devicesBackButton"

        Layout.topMargin: 16
        Layout.leftMargin: MZTheme.theme.windowMargin * 2
        Layout.rightMargin: MZTheme.theme.windowMargin * 2
        Layout.bottomMargin: MZUiUtils.isMobile() ? MZTheme.theme.windowMargin * 2 : MZTheme.theme.windowMargin
        Layout.fillWidth: true

        implicitHeight: MZTheme.theme.rowHeight

        labelText: MZI18n.GlobalGoBack

        onClicked: {
            Glean.interaction.goBackSelected.record({
                screen: root.telemetryScreenId,
            });

            root.backClicked()
        }
    }
}
