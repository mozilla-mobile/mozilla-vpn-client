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
    property alias isSettingsView: menu.isSettingsView
    property alias isMainView: menu.isMainView
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

    VPNMenu {
        id: menu
        objectName: "aboutUsBackButton"
        title: qsTrId("vpn.settings.aboutUs")
        visible: !isSettingsView
    }

    Rectangle {
        id: aboutUsCopy

        anchors.top: menu.visible ? menu.bottom : parent.top
        anchors.topMargin: menu.visible ? 0 : VPNTheme.theme.menuHeight + VPNTheme.theme.windowMargin
        anchors.left: viewAboutUs.left
        anchors.leftMargin: VPNTheme.theme.windowMargin
        anchors.rightMargin: VPNTheme.theme.windowMargin
        height: childrenRect.height
        width: viewAboutUs.width - (VPNTheme.theme.windowMargin * 2)
        color: "transparent"

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
                    if (isSettingsView) {
                        settingsStackView.push(openView, { isSettingsView, isMainView })
                    } else if (isMainView) {
                        mainStackView.push(openView, { isSettingsView, isMainView })
                    } else {
                        stackview.push(openView, { isSettingsView, isMainView })
                    }
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
    VPNPopup {
        id: updateAvailablePopup
        anchors.centerIn: parent
        maxWidth: VPNTheme.theme.desktopAppWidth
        contentItem: ColumnLayout {

            Item {
                // Main Image
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredHeight: 90
                Layout.preferredWidth: 90
                Layout.bottomMargin: VPNTheme.theme.listSpacing
                Layout.topMargin: VPNTheme.theme.vSpacing*1.5
                Image {
                    anchors.fill: parent
                    source:  "qrc:/nebula/resources/updateStatusUpdateAvailable.svg"
                    sourceSize.height: parent.height * Screen.devicePixelRatio
                    sourceSize.width: parent.width * Screen.devicePixelRatio
                    fillMode: Image.PreserveAspectFit

                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: VPNTheme.theme.listSpacing * 0.5
                }
            }

            VPNMetropolisLabel {
                color: VPNTheme.theme.fontColorDark
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: VPNTheme.theme.fontSizeLarge
                text: VPNl18n.UpdateButtonTitleOnUpdate
                Layout.bottomMargin: VPNTheme.theme.listSpacing
                Layout.fillWidth: true
            }

            VPNTextBlock {
                horizontalAlignment: Text.AlignHCenter
                text: VPNl18n.UpdateButtonDescriptionOnUpdate
                Layout.fillWidth: true
                Layout.preferredWidth: parent.width
            }

            VPNButton {
                radius: VPNTheme.theme.cornerRadius
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBottom
                Layout.topMargin: VPNTheme.theme.vSpacing
                Layout.bottomMargin: VPNTheme.theme.vSpacing * 0.5
                text: VPNl18n.UpdateButtonActionOnUpdate
                onClicked: {
                    updateAvailablePopup.close()
                    stackview.push("qrc:/ui/views/ViewUpdate.qml");
                }

                Image {
                    anchors {
                        right: parent.contentItem.right
                        rightMargin: VPNTheme.theme.windowMargin
                        verticalCenter: parent.verticalCenter
                    }
                    fillMode: Image.PreserveAspectFit
                    source: "qrc:/nebula/resources/arrow-forward-white.svg"
                    sourceSize.height: VPNTheme.theme.iconSize * 1.5
                    sourceSize.width: VPNTheme.theme.iconSize * 1.5
                    visible: false
                }
            }

        }
    }


    VPNPopup {
        id: noUpdateAvailablePopup
        anchors.centerIn: parent
        maxWidth: VPNTheme.theme.desktopAppWidth
        contentItem: ColumnLayout {

            Item {
                // Main Image
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredHeight: 90
                Layout.preferredWidth: 90
                Layout.bottomMargin: VPNTheme.theme.listSpacing
                Layout.topMargin: VPNTheme.theme.vSpacing*1.5
                Image {
                    anchors.fill: parent
                    source: "qrc:/nebula/resources/updateStatusUpToDate.svg"
                    sourceSize.height: parent.height * Screen.devicePixelRatio
                    sourceSize.width: parent.width * Screen.devicePixelRatio
                    fillMode: Image.PreserveAspectFit

                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: VPNTheme.theme.listSpacing * 0.5
                }
            }

            VPNMetropolisLabel {
                color: VPNTheme.theme.fontColorDark
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: VPNTheme.theme.fontSizeLarge
                text: VPNl18n.UpdateButtonTitleNoUpdate

                Layout.bottomMargin: VPNTheme.theme.listSpacing
                Layout.fillWidth: true
            }

            VPNTextBlock {
                horizontalAlignment: Text.AlignHCenter
                text: VPNl18n.UpdateButtonDescriptionNoUpdate2
                Layout.fillWidth: true
                Layout.preferredWidth: parent.width
            }

            VPNButton {
                radius: VPNTheme.theme.cornerRadius
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBottom
                Layout.topMargin: VPNTheme.theme.vSpacing
                Layout.bottomMargin: VPNTheme.theme.vSpacing * 0.5
                text: VPNl18n.UpdateButtonActionNoUpdate
                onClicked: {
                    noUpdateAvailablePopup.close();
                }

                Image {
                    anchors {
                        right: parent.contentItem.right
                        rightMargin: VPNTheme.theme.windowMargin
                        verticalCenter: parent.verticalCenter
                    }
                    fillMode: Image.PreserveAspectFit
                    source: "qrc:/nebula/resources/arrow-forward-white.svg"
                    sourceSize.height: VPNTheme.theme.iconSize * 1.5
                    sourceSize.width: VPNTheme.theme.iconSize * 1.5
                    visible: false
                }
            }

        }
    }


}
