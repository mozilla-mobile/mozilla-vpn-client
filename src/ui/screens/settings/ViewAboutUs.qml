/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQuick.Window 2.1

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1

MZViewBase {
    objectName: "viewAboutUs"
    property bool listenForUpdateEvents:false
    property string licenseURL: ""

    _menuTitle: MZI18n.AboutUsTitle

    _viewContentData: ColumnLayout {
        spacing: MZTheme.theme.windowMargin
        Layout.preferredWidth: parent.width
        objectName: "aboutUsList"

        ColumnLayout {
            Layout.leftMargin: MZTheme.theme.windowMargin
            Layout.rightMargin: MZTheme.theme.windowMargin
            Layout.fillWidth: true
            spacing: MZTheme.theme.windowMargin

            ColumnLayout {
                spacing: MZTheme.theme.windowMargin / 2

                MZBoldLabel {
                    text: MZI18n.ProductName
                    Layout.fillWidth: true
                }
                MZTextBlock {
                    Layout.fillWidth: true
                    width: undefined
                    text: MZI18n.ProductDescription
                }
            }
            MZBoldLabel {
                text: MZI18n.AboutUsReleaseVersion
            }
            MZClickableRow {
                property int marginOffset: MZTheme.theme.windowMargin / 2

                id: copyVersionNumber

                Accessible.name: MZI18n.AboutUsCopyVersionNumber.arg(releaseVersion.text)
                Layout.leftMargin: -marginOffset
                Layout.rightMargin: -marginOffset
                Layout.topMargin: -MZTheme.theme.windowMargin
                anchors {
                    left: undefined
                    right: undefined
                    rightMargin: undefined
                    leftMargin: undefined
                }

                Layout.fillWidth: true
                Layout.preferredHeight: MZTheme.theme.rowHeight
                onClicked: {
                    MZUtils.storeInClipboard(releaseVersion.text)
                    MZErrorHandler.requestAlert(MZErrorHandler.CopiedToClipboardConfirmationAlert);
                }

                RowLayout {
                    id: versionRow
                    anchors.left: parent.left
                    anchors.leftMargin: parent.marginOffset
                    anchors.right: parent.right
                    anchors.verticalCenter: copyVersionNumber.verticalCenter
                    spacing: parent.marginOffset

                    MZTextBlock {
                        id: releaseVersion
                        text: MZEnv.buildNumber === "" ? MZEnv.versionString : (MZEnv.versionString + " (" + MZEnv.buildNumber + ")")
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

                   linkTitleId: "AboutUsTermsOfService"
                   openUrl: "terms-of-service"
                   openView: ""
               }

               ListElement {
                   linkId: "privacy"

                   linkTitleId: "AboutUsPrivacyNotice"
                   openUrl: "link-privacy-notice"
                   openView: ""
               }
           }
           delegate: MZExternalLinkListItem {
               objectName: "aboutUsList-" + linkId
               title: MZI18n[linkTitleId]
               accessibleName: title
               Layout.fillWidth: true
               Layout.preferredHeight: MZTheme.theme.rowHeight
               Layout.leftMargin: MZTheme.theme.windowMargin / 2
               Layout.rightMargin: MZTheme.theme.windowMargin / 2

               onClicked: {
                   if (openUrl === "terms-of-service") {
                       MZUrlOpener.openUrlLabel("termsOfService");
                   }

                   if (openUrl === "link-privacy-notice") {
                       MZUrlOpener.openUrlLabel("privacyNotice");
                   }

                   if (openView) {
                       stackview.push(openView)
                   }
               }
               iconSource: openUrl ? "qrc:/nebula/resources/externalLink.svg" : "qrc:/nebula/resources/chevron.svg"
               iconMirror: !openUrl && MZLocalizer.isRightToLeft
               anchors.left: undefined
               anchors.right: undefined
           }
           Component.onCompleted: {
              aboutUsListModel.append({
                   linkId: "license",
                   linkTitleId: "AboutUsLicenses",
                   openView: "qrc:/Mozilla/VPN/screens/settings/ViewLicenses.qml"
              });
           }
        }

        MZButton {
            id:updateButton
            Layout.fillWidth: true
            Layout.leftMargin: MZTheme.theme.windowMargin
            Layout.rightMargin: MZTheme.theme.windowMargin

            onClicked: {
                listenForUpdateEvents=true;
                updateButtonImageAnimation.start();
                VPNReleaseMonitor.runSoon();
            }
            text: MZI18n.UpdateButtonCheckForUpdateButtonText
            Image {
                id:updateButtonImage
                anchors {
                    // TODO: The content item spans the whole Button
                    // If we wish to align to the text, maybe we can get
                    // the texts bounding box with "TextMetrics"?
                    left: updateButton.contentItem.left
                    leftMargin: MZTheme.theme.windowMargin
                    verticalCenter: parent.verticalCenter
                }
                fillMode: Image.PreserveAspectFit
                source: "qrc:/nebula/resources/refresh.svg"
                sourceSize.height: MZTheme.theme.iconSize * 1.5
                sourceSize.width: MZTheme.theme.iconSize * 1.5
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
                updatePopup.title = MZI18n.UpdateButtonTitleOnUpdate
                updatePopup.description = MZI18n.UpdateButtonDescriptionOnUpdate
                updatePopup.buttonText = MZI18n.UpdateButtonActionOnUpdate
            } else {
                updateButtonImageAnimation.stop();
                updatePopup.imageSrc = "qrc:/nebula/resources/updateStatusUpToDate.svg";
                updatePopup.imageSize = Qt.size(80, 80)
                updatePopup.title = MZI18n.UpdateButtonTitleNoUpdate
                updatePopup.description = MZI18n.UpdateButtonDescriptionNoUpdate2
                updatePopup.buttonText = MZI18n.UpdateButtonActionNoUpdate
            }
            updatePopup.open()
            listenForUpdateEvents=false;
        }
    }

    MZSimplePopup {
        id: updatePopup
        property alias buttonText: popupBtn.text
        property bool updateAvailable

        anchors.centerIn: Overlay.overlay
        imageSrc: "qrc:/nebula/resources/updateStatusUpdateAvailable.svg"
        imageSize: Qt.size(80, 80)
        title: MZI18n.UpdateButtonTitleOnUpdate
        description: MZI18n.UpdateButtonDescriptionOnUpdate
        buttons: [
            MZButton {
                id: popupBtn
                text: MZI18n.UpdateButtonActionOnUpdate
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
