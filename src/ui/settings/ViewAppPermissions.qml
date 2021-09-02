/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.14
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme

import org.mozilla.Glean 0.15
import telemetry 0.15


Item {
    id: root

    //% "Search Apps"
    //: Search bar placeholder text
    property string searchApps: qsTrId("vpn.protectSelectedApps.searchApps")

    //% "Add Application"
    //: Button label
    property string addApplication: qsTrId("vpn.protectSelectedApps.addApplication")
    property string _menuTitle: qsTrId("vpn.settings.appPermissions2")
    objectName: "appPermissions"

    VPNFlickable {
        id: vpnFlickable
        property bool vpnIsOff: (VPNController.state === VPNController.StateOff)
        flickContentHeight:  (VPNSettings.protectSelectedApps ? enabledList.y + enabledList.implicitHeight + 100 : vpnFlickable.y + toggleCard.height )+ helpInfoText.height + helpLink.height
        anchors.top: parent.top
        anchors.topMargin: Theme.menuHeight
        height: root.height - menu.height
        anchors.left: parent.left
        anchors.right: parent.right
        interactive: (VPNSettings.protectSelectedApps)
        Component.onCompleted: {
            console.log("Component ready");
            VPNAppPermissions.requestApplist();
            Sample.appPermissionsViewOpened.record();
            if (!vpnIsOff) {
                Sample.appPermissionsViewWarning.record();
            }
        }

        ColumnLayout{
            id:messageBox
            visible: true
            anchors.top: parent.top
            anchors.topMargin: Theme.windowMargin
            anchors.left: parent.left
            anchors.leftMargin: 8
            width: toggleCard.width-16

            height:(vpnOnAlert.visible? vpnOnAlert.height:0)+(toast.visible? toast.height:0)

            VPNCheckBoxAlert {
                anchors.fill: parent
                anchors.leftMargin: Theme.windowMargin
                anchors.left: parent.left
                id: vpnOnAlert
                visible: !vpnFlickable.vpnIsOff
                //% "VPN must be off to edit App Permissions"
                //: Associated to a group of settings that require the VPN to be disconnected to change
                errorMessage: qsTrId("vpn.settings.protectSelectedApps.vpnMustBeOff")
            }

            Connections {
                target: VPNAppPermissions
                function onNotification(type,message,action) {
                    console.log("Got notification: "+type + "  message:"+message);
                    var component = Qt.createComponent("../components/VPNAlert.qml");
                    component.createObject(root, {
                                               isLayout:false,
                                               visible:true,
                                               alertText: message,
                                               alertType: type,
                                               alertActionText: action,
                                               duration:type === "warning"? 0: 2000,
                                               destructive:true,
                                               onActionPressed: ()=>{VPNAppPermissions.openFilePicker();},
                                           });
                }
            }

        }


        VPNToggleCard {
            id: toggleCard

            toggleObjectName: "settingsAppPermissionsToggle"
            anchors.left: parent.left
            anchors.right: parent.right
            height: childrenRect.height
            anchors.top: messageBox.visible ? messageBox.bottom : parent.top

            //% "Protect all apps with VPN"
            labelText: qsTrId("vpn.settings.protectAllApps")

            //% "VPN protects all apps by default. Turn off to choose which apps Mozilla VPN should not protect."
            sublabelText: qsTrId("vpn.settings.protectAllApps.description")

            //% "VPN protects all apps by default. Turn off to choose which apps Mozilla VPN should not protect."
            toolTipTitleText: qsTrId("vpn.settings.protectAllApps.description")

            toggleChecked: (!VPNSettings.protectSelectedApps)

            function handleClick() {
                if (vpnFlickable.vpnIsOff) {
                    VPNSettings.protectSelectedApps = !VPNSettings.protectSelectedApps
                }
            }
        }

        VPNExpandableAppList {
            id: enabledList
            anchors.topMargin: 30
            anchors.top: toggleCard.bottom
            searchBarPlaceholder: searchApps

            //% "Exclude apps from VPN protection"
            //: Header for the list of apps protected by VPN
            header: qsTrId("vpn.settings.excludeTitle")
        }

        VPNTextBlock {
            id: helpInfoText
            width: vpnFlickable.width - Theme.windowMargin*3
            anchors.topMargin: 30
            anchors.top: enabledList.visible? enabledList.bottom : toggleCard.bottom
            anchors.horizontalCenter:  enabledList.visible? enabledList.horizontalCenter : toggleCard.horizontalCenter
            text: VPNl18n.tr(VPNl18n.SplittunnelInfoText)
        }

        VPNHeaderLink{
            id: helpLink
            anchors.top:  helpInfoText.bottom
            anchors.horizontalCenter: helpInfoText.horizontalCenter
            labelText: VPNl18n.tr(VPNl18n.SplittunnelInfoLinkText)
            onClicked: {
               VPN.openLink(VPN.LinkSplitTunnelHelp)
            }
        }
    }
}
