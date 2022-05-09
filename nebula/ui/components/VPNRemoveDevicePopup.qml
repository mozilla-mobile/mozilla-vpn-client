/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import compat 0.1

Popup {
    id: popup

    property var deviceName
    property var devicePublicKey
    property var wasmView

    anchors.centerIn: parent
    closePolicy: Popup.CloseOnEscape
    focus: true
    leftInset: VPNTheme.theme.windowMargin
    rightInset: VPNTheme.theme.windowMargin
    modal: true
    verticalPadding: VPNTheme.theme.popupMargin

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
        color: VPNTheme.theme.bgColor
        radius: 8

        VPNDropShadow {
            id: popupShadow

            anchors.fill: popupBackground
            cached: true
            color: "black"
            opacity: 0.2
            radius: 16
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
            width: contentRoot.width - VPNTheme.theme.windowMargin / 2

            Image {
                fillMode: Image.PreserveAspectFit
                source: "qrc:/nebula/resources/devicesRemove.svg"
                sourceSize: Qt.size(116, 80)

                Layout.alignment: Qt.AlignHCenter
                Layout.preferredHeight: 80
            }

            VPNMetropolisLabel {
                id: popupTitle

                color: VPNTheme.theme.fontColorDark
                font.pixelSize: VPNTheme.theme.fontSizeLarge
                horizontalAlignment: Text.AlignHCenter

                Layout.alignment: Qt.AlignHCenter
                Layout.bottomMargin: VPNTheme.theme.vSpacingSmall
                Layout.fillWidth: true
                Layout.leftMargin: VPNTheme.theme.popupMargin
                Layout.minimumHeight: 36
                Layout.rightMargin: VPNTheme.theme.popupMargin
                Layout.topMargin: 4

                //% "Remove device?"
                text: qsTrId("vpn.devices.removeDeviceQuestion")
                verticalAlignment: Text.AlignBottom
            }

            VPNTextBlock {
                id: popupText

                property string textString

                color: VPNTheme.theme.fontColor
                font.pixelSize: 15
                horizontalAlignment: Text.AlignHCenter
                Layout.alignment: Qt.AlignHCenter
                Layout.bottomMargin: VPNTheme.theme.vSpacing
                Layout.leftMargin: VPNTheme.theme.popupMargin
                Layout.rightMargin: VPNTheme.theme.popupMargin
                Layout.fillWidth: true
                lineHeight: 22;
                text: textString.arg(popup.deviceName)
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere

                Component.onCompleted: {
                    //: %1 is the name of the device being removed. The name is displayed on purpose on a new line.
                    //% "Please confirm you would like to remove\n%1."
                    textString = qsTrId("vpn.devices.deviceRemovalConfirm").replace("\n", " ")
                }
            }

            GridLayout {
                id: buttonsContainer

                property int gridSpacing: 16

                columnSpacing: gridSpacing
                rowSpacing: gridSpacing
                columns: 1

                Layout.fillWidth: true
                Layout.minimumHeight: 40
                Layout.leftMargin: VPNTheme.theme.popupMargin
                Layout.rightMargin: VPNTheme.theme.popupMargin
                Layout.topMargin: gridSpacing / 2

                VPNPopupButton {
                    id: removeBtn

                    //: This is the “remove” device button.
                    //% "Remove"
                    buttonText: qsTrId("vpn.devices.removeDeviceButton")
                    buttonTextColor: VPNTheme.theme.white
                    colorScheme: VPNTheme.theme.redButton
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
                    isCancelBtn: false
                }

                VPNCancelButton {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignCenter

                    onClicked: {
                        popup.close();
                    }
                }
            }
        }

        Overlay.modal: Rectangle {
            id: overlayBackground
            color: VPNTheme.theme.overlayBackground

            Behavior on opacity {
                NumberAnimation {
                    duration: 175
                }
            }
        }

    }
}
