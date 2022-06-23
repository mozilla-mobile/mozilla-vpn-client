/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQuick.Window 2.1

import Mozilla.VPN 1.0
import components 0.1

Item {
    id: viewAboutUs
    property alias releaseVersionText: releaseVersion.text
    //% "About us"
    property string _menuTitle: qsTrId("vpn.settings.aboutUs")
    property bool listenForUpdateEvents:false
    property string licenseURL: ""

    ListModel {
        id: aboutUsListModel

        ListElement {
            linkId: "tos"

            //% "Terms of service"
            linkTitle: qsTrId("vpn.aboutUs.tos2")
            openUrl: VPN.LinkTermsOfService
            openView: ""
        }

        ListElement {
            linkId: "privacy"

            //% "Privacy notice"
            linkTitle: qsTrId("vpn.aboutUs.privacyNotice2")
            openUrl: VPN.LinkPrivacyNotice
            openView: ""
        }
    }

    Component.onCompleted: {
       aboutUsListModel.append({
            linkId: "license",
            linkTitle: VPNl18n.AboutUsLicenses,
            openView: viewAboutUs.licenseURL
       });
    }

    Rectangle {
        id: aboutUsCopy

        anchors.top: parent.top
        anchors.topMargin: VPNTheme.theme.windowMargin
        anchors.left: viewAboutUs.left
        anchors.leftMargin: VPNTheme.theme.windowMargin
        anchors.rightMargin: VPNTheme.theme.windowMargin
        height: childrenRect.height
        width: viewAboutUs.width - (VPNTheme.theme.windowMargin * 2)
        color: VPNTheme.theme.transparent

        VPNBoldLabel {
            id: mozillaLabel

            width: aboutUsCopy.width
            text: qsTrId("vpn.main.productName")
        }

        VPNTextBlock {
            id: mozillaText

            text: qsTrId("vpn.main.productDescription")
            anchors.top: mozillaLabel.bottom
            anchors.topMargin: 8
            width: aboutUsCopy.width
        }

        VPNBoldLabel {
            id: releaseLabel

            //% "Release version"
            //: Refers to the installed version. For example: "Release Version: 1.23"
            text: qsTrId("vpn.aboutUs.releaseVersion2")
            anchors.top: mozillaText.bottom
            anchors.topMargin: 16
        }

        TextEdit {
            id: releaseVersion
            anchors.top: releaseLabel.bottom
            anchors.topMargin: 8
            text: VPN.buildNumber === "" ? VPN.versionString : (VPN.versionString + " (" + VPN.buildNumber + ")")
            readOnly: true
            wrapMode: Text.WordWrap
            selectByMouse: true

            color: VPNTheme.theme.fontColor
            font.family: VPNTheme.theme.fontInterFamily
            font.pixelSize: VPNTheme.theme.fontSizeSmall
            width: VPNTheme.theme.maxTextWidth

            Accessible.role: Accessible.StaticText
            Accessible.name: text
        }
    }

    Rectangle {
        id: divider

        height: 1
        width: viewAboutUs.width
        anchors.top: aboutUsCopy.bottom
        anchors.left: viewAboutUs.left
        anchors.right: viewAboutUs.right
        anchors.topMargin: 12
        anchors.leftMargin: VPNTheme.theme.windowMargin
        anchors.rightMargin: VPNTheme.theme.windowMargin
        color: "#0C0C0D0A"
    }

    VPNList {
        id: settingList
        objectName: "aboutUsList"

        anchors.top: divider.bottom
        anchors.topMargin: 16
        anchors.bottomMargin: VPNTheme.theme.vSpacing
        width: viewAboutUs.width
        spacing: VPNTheme.theme.listSpacing
        model: aboutUsListModel
        listName: _menuTitle

        delegate: VPNExternalLinkListItem {
            objectName: "aboutUsList-" + linkId
            title: linkTitle
            accessibleName: title
            onClicked: {
                if (openUrl) {
                    VPN.openLink(openUrl)
                }
                if (openView) {
                    settingsStackView.push(openView)
                }
            }
            iconSource: openUrl ? "qrc:/nebula/resources/externalLink.svg" : "qrc:/nebula/resources/chevron.svg"
        }

        ScrollBar.vertical: ScrollBar {
            Accessible.ignored: true
        }

    }

    VPNButton{
        id:updateButton
        anchors.top: settingList.bottom
        anchors.topMargin: 16
        anchors.bottomMargin: VPNTheme.theme.vSpacing
        anchors.horizontalCenter : viewAboutUs.horizontalCenter

        onClicked: {
            listenForUpdateEvents=true;
            updateButtonImageAnimation.start();
            VPNReleaseMonitor.runSoon();
        }
        text: VPNl18n.UpdateButtonCheckForUpdateButtonText
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
    Connections {
        target: VPNReleaseMonitor
        function onUpdateRequiredOrRecommended() {
            if(!listenForUpdateEvents){
                return;
            }
            updateButtonImageAnimation.stop();
            updateAvailablePopup.open()
            listenForUpdateEvents=false;
        }
        function onUpdateNotAvailable() {
            if(!listenForUpdateEvents){return;}
            updateButtonImageAnimation.stop();
            noUpdateAvailablePopup.open()
            listenForUpdateEvents=false;
        }
    }

    VPNSimplePopup {
        id: updateAvailablePopup

        anchors.centerIn: Overlay.overlay
        imageSrc: "qrc:/nebula/resources/updateStatusUpdateAvailable.svg"
        imageSize: Qt.size(80, 80)
        title: VPNl18n.UpdateButtonTitleOnUpdate
        description: VPNl18n.UpdateButtonDescriptionOnUpdate
        buttons: [
            VPNButton {
                text: VPNl18n.UpdateButtonActionOnUpdate
                onClicked: {
                    updateAvailablePopup.close()
                    mainStackView.push("qrc:/ui/views/ViewUpdate.qml");
                }
            }
        ]
    }

    VPNSimplePopup {
        id: noUpdateAvailablePopup

        anchors.centerIn: Overlay.overlay
        imageSrc: "qrc:/nebula/resources/updateStatusUpToDate.svg"
        imageSize: Qt.size(80, 80)
        title: VPNl18n.UpdateButtonTitleNoUpdate
        description: VPNl18n.UpdateButtonDescriptionNoUpdate2
        buttons: [
            VPNButton {
                text: VPNl18n.UpdateButtonActionNoUpdate
                onClicked: {
                    noUpdateAvailablePopup.close();
                }
            }
        ]
    }
}
