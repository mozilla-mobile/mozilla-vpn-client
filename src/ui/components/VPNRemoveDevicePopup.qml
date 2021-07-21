/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.14
import Mozilla.VPN 1.0
import "../themes/themes.js" as Theme

Popup {
    id: popup

    property var deviceName
    property var devicePublicKey
    property var wasmView

    anchors.centerIn: parent
    closePolicy: Popup.CloseOnEscape
    focus: true
    leftInset: Theme.windowMargin
    rightInset: Theme.windowMargin
    modal: true
    verticalPadding: Theme.popupMargin

    // TODO: We can not use Accessible type on Popup because it does not inherit
    // from an Item. The code below generates the following warning:
    // "...Accessible must be attached to an Item..."
    // See https://github.com/mozilla-mobile/mozilla-vpn-client/issues/322 for
    // more details.
    // Accessible.role: Accessible.Dialog
    // Accessible.name: popupTitle.text

    enter: Transition {
        NumberAnimation {
            property: "opacity"
            duration: 120
            from: 0.0
            to: 1.0
            easing.type: Easing.InOutQuad
        }
    }

    exit: Transition {
        NumberAnimation {
            property: "opacity"
            duration: 120
            from: 1.0
            to: 0.0
            easing.type: Easing.InOutQuad
        }
    }

    onClosed: {
        console.log("state", popup.state);
        // When closing the dialog, put the focus back on the
        // remove button that originally triggered the dialog.
        if (wasmView) {
            return;
        }

        if (deviceList.focusedIconButton) {
            deviceList.focusedIconButton.forceActiveFocus();
        }
    }

    background: Rectangle {
        id: popupBackground

        anchors.margins: 0
        color: Theme.bgColor
        radius: 8

        DropShadow {
            id: popupShadow

            anchors.fill: popupBackground
            cached: true
            color: "black"
            opacity: 0.2
            radius: 16
            samples: 33
            source: popupBackground
            spread: 0.1
            transparentBorder: true
            verticalOffset: 4
            z: -1
        }
    }

    contentItem: Item {
        id: contentRoot

        ColumnLayout {
            id: col

            anchors.centerIn: contentRoot
            spacing: 0
            width: contentRoot.width - Theme.windowMargin / 2

            Image {
                fillMode: Image.PreserveAspectFit
                source: "../resources/devicesRemove.svg"
                sourceSize: Qt.size(116, 80)

                Layout.alignment: Qt.AlignHCenter
                Layout.preferredHeight: 72
            }

            VPNMetropolisLabel {
                id: popupTitle

                color: Theme.fontColorDark
                font.pixelSize: Theme.fontSizeLarge
                horizontalAlignment: Text.AlignHCenter

                Layout.alignment: Qt.AlignHCenter
                Layout.bottomMargin: Theme.vSpacingSmall
                Layout.fillWidth: true
                Layout.leftMargin: Theme.popupMargin
                Layout.minimumHeight: 36
                Layout.rightMargin: Theme.popupMargin
                Layout.topMargin: 4

                //% "Remove device?"
                text: qsTrId("vpn.devices.removeDeviceQuestion")
                verticalAlignment: Text.AlignBottom
            }

            VPNTextBlock {
                id: popupText

                color: Theme.fontColor
                font.pixelSize: 15
                horizontalAlignment: Text.AlignHCenter
                Layout.alignment: Qt.AlignHCenter
                Layout.bottomMargin: Theme.vSpacingSmall
                Layout.fillWidth: true
                lineHeight: 22

                //: %1 is the name of the device being removed. The name is displayed on purpose on a new line.
                //% "Please confirm you would like to remove\n%1."
                text: qsTrId("vpn.devices.deviceRemovalConfirm").arg(popup.deviceName)
            }

            Rectangle {
                id: buttonSeparator

                color: Theme.greyHovered
                Layout.fillWidth: true
                height: 1
            }

            GridLayout {
                id: buttonsContainer
                columnSpacing: 16
                rowSpacing: 16
                columns: {
                    const cancelButtonText = qsTrId("vpn.devices.cancelDeviceRemoval");
                    const removeButtonText = qsTrId("vpn.devices.removeDeviceButton");
                    const showStacked = cancelButtonText.length > 8 || removeButtonText.length > 8;

                    return showStackedButtons ? 1 : 2;
                }

                Layout.fillWidth: true
                Layout.minimumHeight: 40
                Layout.leftMargin: Theme.popupMargin
                Layout.rightMargin: Theme.popupMargin
                Layout.topMargin: Theme.popupMargin

                VPNPopupButton {
                    id: cancelBtn
                    Accessible.defaultButton: true

                    //% "Cancel"
                    buttonText: qsTrId("vpn.devices.cancelDeviceRemoval");
                    buttonTextColor: Theme.fontColorDark
                    colorScheme: Theme.greyButton
                    focus: true
                    onClicked: {
                        popup.close();
                    }
                }

                VPNPopupButton {
                    id: removeBtn

                    //: This is the “remove” device button.
                    //% "Remove"
                    buttonText: qsTrId("vpn.devices.removeDeviceButton")
                    buttonTextColor: Theme.white
                    colorScheme: Theme.redButton
                    onClicked: {
                        VPN.removeDeviceFromPublicKey(popup.devicePublicKey);
                        if (vpnFlickable.state === "deviceLimit") {
                            // there is no further action the user can take on the deviceList
                            // so leave the modal open until the user is redirected back to the main view
                            col.opacity = .5
                            return;
                        }

                        popup.close();
                    }
                }

            }

        }

        Overlay.modal: Rectangle {
            id: overlayBackground
            color: "#4D000000"

            Behavior on opacity {
                NumberAnimation {
                    duration: 175
                }
            }
        }

    }
}
