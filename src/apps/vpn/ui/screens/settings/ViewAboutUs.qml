/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQuick.Window 2.1

import Mozilla.VPN 1.0
import components 0.1

VPNViewBase {
    objectName: "viewAboutUs"
    property bool listenForUpdateEvents:false
    property string licenseURL: ""

    //% "About us"
    _menuTitle: qsTrId("vpn.settings.aboutUs")

    _viewContentData: ColumnLayout {
        spacing: VPNTheme.theme.windowMargin
        Layout.fillWidth: true
        objectName: "aboutUsList"

        ColumnLayout {
            Layout.leftMargin: VPNTheme.theme.windowMargin
            Layout.rightMargin: VPNTheme.theme.windowMargin
            Layout.fillWidth: true
            spacing: VPNTheme.theme.windowMargin

            ColumnLayout {
                spacing: VPNTheme.theme.windowMargin / 2

                VPNBoldLabel {
                    text: qsTrId("vpn.main.productName")
                    Layout.fillWidth: true
                }
                VPNTextBlock {
                    Layout.fillWidth: true
                    width: undefined
                    text: qsTrId("vpn.main.productDescription")
                }
            }
            VPNBoldLabel {
                //% "Release version"
                //: Refers to the installed version. For example: "Release Version: 1.23"
                text: qsTrId("vpn.aboutUs.releaseVersion2")
                Accessible.role: Accessible.StaticText
            }
            VPNClickableRow {
                property int marginOffset: VPNTheme.theme.windowMargin / 2

                id: copyVersionNumber

                Accessible.name: VPNI18n.AboutUsCopyVersionNumber.arg(releaseVersion.text)
                Layout.leftMargin: -marginOffset
                Layout.rightMargin: -marginOffset
                Layout.topMargin: -VPNTheme.theme.windowMargin
                anchors {
                    left: undefined
                    right: undefined
                    rightMargin: undefined
                    leftMargin: undefined
                }

                Layout.fillWidth: true
                Layout.preferredHeight: VPNTheme.theme.rowHeight
                onClicked: {
                    VPN.storeInClipboard(releaseVersion.text)
                    VPNErrorHandler.requestAlert(VPNErrorHandler.CopiedToClipboardConfirmationAlert);
                }

                RowLayout {
                    id: versionRow
                    anchors.left: parent.left
                    anchors.leftMargin: parent.marginOffset
                    anchors.right: parent.right
                    anchors.verticalCenter: copyVersionNumber.verticalCenter
                    spacing: parent.marginOffset

                    VPNTextBlock {
                        id: releaseVersion
                        text: VPN.env.buildNumber === "" ? VPN.env.versionString : (VPN.env.versionString + " (" + VPN.env.buildNumber + ")")
                        width: undefined
                        Layout.alignment: Qt.AlignVCenter
                    }
                    Image {
                        objectName: "copyVersionNumberIcon"
                        source: "qrc:/nebula/resources/copy.svg"
                        fillMode: Image.PreserveAspectFit
                        Layout.rightMargin: copyVersionNumber.marginOffset
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                    }
                }
            }
            Rectangle {
                id: divider

                Layout.preferredHeight: 1
                Layout.fillWidth: true
                color: "#0C0C0D0A"
            }
        }

        Repeater {
           model: ListModel {
               id: aboutUsListModel
               ListElement {
                   linkId: "tos"

                   //% "Terms of service"
                   linkTitle: qsTrId("vpn.aboutUs.tos2")
                   openUrl: "terms-of-service"
                   openView: ""
               }

               ListElement {
                   linkId: "privacy"

                   //% "Privacy notice"
                   linkTitle: qsTrId("vpn.aboutUs.privacyNotice2")
                   openUrl: "link-privacy-notice"
                   openView: ""
               }
           }
           delegate: VPNExternalLinkListItem {
               objectName: "aboutUsList-" + linkId
               title: linkTitle
               accessibleName: title
               Layout.fillWidth: true
               Layout.preferredHeight: VPNTheme.theme.rowHeight
               Layout.leftMargin: VPNTheme.theme.windowMargin / 2
               Layout.rightMargin: VPNTheme.theme.windowMargin / 2

               onClicked: {
                   if (openUrl === "terms-of-service") {
                       VPNUrlOpener.openUrlLabel("termsOfService");
                   }

                   if (openUrl === "link-privacy-notice") {
                       VPNUrlOpener.openUrlLabel("privacyNotice");
                   }

                   if (openView) {
                       stackview.push(openView)
                   }
               }
               iconSource: openUrl ? "qrc:/nebula/resources/externalLink.svg" : "qrc:/nebula/resources/chevron.svg"
               iconMirror: !openUrl && VPNLocalizer.isRightToLeft
               anchors.left: undefined
               anchors.right: undefined
           }
           Component.onCompleted: {
              aboutUsListModel.append({
                   linkId: "license",
                   linkTitle: VPNI18n.AboutUsLicenses,
                   openView: "qrc:/ui/screens/settings/ViewLicenses.qml"
              });
           }
        }

        VPNButton {
            id:updateButton
            Layout.fillWidth: true
            Layout.leftMargin: VPNTheme.theme.windowMargin
            Layout.rightMargin: VPNTheme.theme.windowMargin

            onClicked: {
                listenForUpdateEvents=true;
                updateButtonImageAnimation.start();
                VPNReleaseMonitor.runSoon();
            }
            text: VPNI18n.UpdateButtonCheckForUpdateButtonText
            Image {
                id:updateButtonImage
                anchors {
                    // TODO: The content item spans the whole Button
                    // If we wish to align to the text, maybe we can get
                    // the texts bounding box with "TextMetrics"?
                    left: updateButton.contentItem.left
                    leftMargin: VPNTheme.theme.windowMargin
                    verticalCenter: parent.verticalCenter
                }
                fillMode: Image.PreserveAspectFit
                source: "qrc:/nebula/resources/refresh.svg"
                sourceSize.height: VPNTheme.theme.iconSize * 1.5
                sourceSize.width: VPNTheme.theme.iconSize * 1.5
                visible: true
                z:6

                SequentialAnimation {
                    id: updateButtonImageAnimation
                    running: false
                    PropertyAnimation {
                        target: updateButtonImage
                        property: "rotation"
                        from: 0
                        to: 360
                        duration: 2000
                        loops: Animation.Infinite
                    }
                }
            }
        }
    }
    Connections {
        target: VPNReleaseMonitor
        function onUpdateRequiredOrRecommended() {
            openUpdatePopup(true)
        }
        function onUpdateNotAvailable() {
            openUpdatePopup(false);
        }

        function openUpdatePopup(updateAvailable) {
            if(!listenForUpdateEvents){ return }

            updatePopup.updateAvailable = updateAvailable;

            if (updateAvailable) {
                updateButtonImageAnimation.stop();
                updatePopup.imageSrc = "qrc:/nebula/resources/updateStatusUpdateAvailable.svg";
                updatePopup.imageSize = Qt.size(80, 80)
                updatePopup.title = VPNI18n.UpdateButtonTitleOnUpdate
                updatePopup.description = VPNI18n.UpdateButtonDescriptionOnUpdate
                updatePopup.buttonText = VPNI18n.UpdateButtonActionOnUpdate
            } else {
                updateButtonImageAnimation.stop();
                updatePopup.imageSrc = "qrc:/nebula/resources/updateStatusUpToDate.svg";
                updatePopup.imageSize = Qt.size(80, 80)
                updatePopup.title = VPNI18n.UpdateButtonTitleNoUpdate
                updatePopup.description = VPNI18n.UpdateButtonDescriptionNoUpdate2
                updatePopup.buttonText = VPNI18n.UpdateButtonActionNoUpdate
            }
            updatePopup.open()
            listenForUpdateEvents=false;
        }
    }

    VPNSimplePopup {
        id: updatePopup
        property alias buttonText: popupBtn.text
        property bool updateAvailable

        anchors.centerIn: Overlay.overlay
        imageSrc: "qrc:/nebula/resources/updateStatusUpdateAvailable.svg"
        imageSize: Qt.size(80, 80)
        title: VPNI18n.UpdateButtonTitleOnUpdate
        description: VPNI18n.UpdateButtonDescriptionOnUpdate
        buttons: [
            VPNButton {
                id: popupBtn
                text: VPNI18n.UpdateButtonActionOnUpdate
                Layout.fillWidth: true
                onClicked: {
                   if (updatePopup.updateAvailable) {
                       VPN.update()
                   }
                    updatePopup.close()
                }
            }
        ]
    }
}
