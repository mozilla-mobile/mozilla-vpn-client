import QtQuick 2.0
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15
import QtQuick.Layouts 1.11
import Mozilla.VPN 1.0

import "../components"
import "../themes/themes.js" as Theme

Flickable {
    id: scrollingFrame

    ListModel {
        id: settingsMenuListModel
        ListElement {
            settingTitle: qsTr("Notifications")
            imageLeftSource: "../resources/settings/notifications.svg"
            imageRightSource: "../resources/chevron.svg"
            pushView: "../settings/ViewNotifications.qml"
        }
        ListElement {
            settingTitle: qsTr("Network settings")
            imageLeftSource: "../resources/settings/networkSettings.svg"
            imageRightSource: "../resources/chevron.svg"
            pushView: "../settings/ViewNetworkSettings.qml"
        }
        ListElement {
            settingTitle: qsTr("Language")
            imageLeftSource: "../resources/settings/language.svg"
            imageRightSource: "../resources/chevron.svg"
            pushView: "../settings/ViewLanguage.qml"
        }
        ListElement {
            settingTitle: qsTr("About us")
            imageLeftSource: "../resources/settings/aboutUs.svg"
            imageRightSource: "../resources/chevron.svg"
            pushView: "../settings/ViewAboutUs.qml"
        }
        ListElement {
            settingTitle: qsTr("Get help")
            imageLeftSource: "../resources/settings/getHelp.svg"
            imageRightSource: "../resources/chevron.svg"
            pushView: "../settings/ViewGetHelp.qml"
        }
        ListElement {
            settingTitle: qsTr("Give feedback")
            imageLeftSource: "../resources/settings/feedback.svg"
            imageRightSource: "../resources/externalLink.svg"
            openUrl: "/r/vpn/client/feedback"
        }
    }

    contentHeight: 660
    boundsBehavior: Flickable.StopAtBounds

    Image {
        id: backImage
        source: "../resources/back.svg"
        sourceSize.width: Theme.iconSize
        fillMode: Image.PreserveAspectFit
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: 20
        anchors.leftMargin: Theme.windowMargin

        MouseArea {
            anchors.fill: backImage
            onClicked: stackview.pop()
        }
    }

    Image {
        id: logo
        source: VPNUser.avatar
        anchors.horizontalCenter: parent.horizontalCenter
        height: 80
        smooth: true
        fillMode: Image.PreserveAspectFit
        layer.enabled: true
        anchors.top: parent.top
        anchors.topMargin: 32
        layer.effect: OpacityMask {
            maskSource: mask
        }
        Rectangle {
            id: mask
            anchors.fill: parent
            radius: 40
            visible: false
        }
    }

    VPNHeadline {
        id: logoTitle
        // TODO : Find out officially what should go here
        // if the user hasn't set a `displayName`...
        text: !VPNUser.displayName ? qsTr("Settings") : VPNUser.displayName
        anchors.top: logo.bottom
        anchors.topMargin: Theme.vSpacing
        height: 32
    }

    VPNSubtitle {
        id: logoSubtitle
        text: VPNUser.email
    }

    VPNButton {
        id: manageAccountButton
        text: qsTr("Manage account")
        anchors.top: logoSubtitle.bottom
        anchors.topMargin: Theme.vSpacing
        onClicked: VPN.openLink("/r/vpn/account")
    }

    ListView {
        id: settingsList
        interactive: false // disable scrolling on list since the entire window is scrollable
        height: parent.height - manageAccountButton.height - logoSubtitle.height - logoTitle.height
        width: parent.width
        anchors.top: manageAccountButton.bottom
        anchors.topMargin: Theme.vSpacing
        spacing: Theme.listSpacing

        model: settingsMenuListModel
        delegate: VPNClickableRow {
            onClicked: {
                if (pushView) {
                    return settingsStackView.push(pushView)
                }
                return VPN.openLink(openUrl)
            }

            VPNSettingsItem {
                setting: settingTitle
                imageLeftSrc: imageLeftSource
                imageRightSrc: imageRightSource
            }
        }
    }

    VPNFooterLink {
        text: qsTr("Sign out")
        Layout.alignment: Qt.AlignHCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 40
        color: Theme.red
        font.weight: Font.Bold
        font.family: vpnFont.name

        MouseArea {
            anchors.fill: parent
            onClicked: VPNController.logout()
        }
    }
}
