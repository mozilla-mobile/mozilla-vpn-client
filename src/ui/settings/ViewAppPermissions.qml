/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1



Item {
    id: root

    //% "Search apps"
    //: Search bar placeholder text
    property string searchApps: qsTrId("vpn.protectSelectedApps.searchApps")

    //% "Add application"
    //: Button label
    property string addApplication: qsTrId("vpn.protectSelectedApps.addApplication")
    property string _menuTitle: qsTrId("vpn.settings.appPermissions2")
    objectName: "appPermissions"

    VPNFlickable {
        id: vpnFlickable
        property bool vpnIsOff: (VPNController.state === VPNController.StateOff)
        flickContentHeight: col.height + VPNTheme.theme.menuHeight
        anchors.fill: parent
        interactive: (VPNSettings.protectSelectedApps)
        Component.onCompleted: {
            console.log("Component ready");
            VPNAppPermissions.requestApplist();
            VPN.recordGleanEvent("appPermissionsViewOpened");

            if (!vpnIsOff) {
                VPN.recordGleanEvent("appPermissionsViewWarning");
            }
        }

        ColumnLayout {
            id: col
            spacing: 0
            width: root.width
            anchors.top: parent.top

            ColumnLayout{
                id: messageBox
                visible: true
                Layout.leftMargin: VPNTheme.theme.windowMargin
                Layout.rightMargin: VPNTheme.theme.windowMargin * 2
                Layout.fillHeight: false
                spacing: 0

                VPNContextualAlerts {
                    id: vpnOnAlert
                    Layout.preferredWidth: parent.width
                    Layout.fillHeight: false
                    Layout.topMargin: vpnFlickable.vpnIsOff ? 0 : VPNTheme.theme.windowMargin

                    messages: [
                        {
                            type: "warning",
                            //% "VPN must be off to edit App Permissions"
                            //: Associated to a group of settings that require the VPN to be disconnected to change
                            message: qsTrId("vpn.settings.protectSelectedApps.vpnMustBeOff"),
                            visible: !vpnFlickable.vpnIsOff
                        }
                    ]
                }

                Connections {
                    target: VPNAppPermissions
                    function onNotification(type,message,action) {
                        console.log("Got notification: "+type + "  message:"+message);
                        var component = Qt.createComponent("qrc:/nebula/components/VPNAlert.qml", { updateURL: "qrc:/ui/views/ViewUpdate.qml" });
                        if(component.status !== Component.Ready)
                            {
                                if( component.status == Component.Error )
                                    console.debug("Error:"+ component.errorString() );

                            }
                        var alert = component.createObject(root, {
                                                   isLayout:false,
                                                   visible:true,
                                                   alertText: message,
                                                   alertType: type,
                                                   alertActionText: action,
                                                   duration:type === "warning"? 0: 2000,
                                                   destructive:true,
                                                   // Pin y hight to be below the alert bar as we can't render above it
                                                   setY: vpnFlickable.y+VPNTheme.theme.windowMargin,
                                                   onActionPressed: ()=>{VPNAppPermissions.openFilePicker();},
                                               });

                        alert.show();

                    }
                }
            }

            VPNToggleCard {
                id: toggleCard

                toggleObjectName: "settingsAppPermissionsToggle"
                toggleEnabled: vpnFlickable.vpnIsOff
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: childrenRect.height + VPNTheme.theme.windowMargin

                //% "Protect all apps with VPN"
                labelText: qsTrId("vpn.settings.protectAllApps")

                //% "VPN protects all apps by default. Turn off to choose which apps Mozilla VPN should not protect."
                sublabelText: qsTrId("vpn.settings.protectAllApps.description")

                toolTipTitleText: qsTrId("vpn.settings.protectAllApps")

                toggleChecked: (!VPNSettings.protectSelectedApps)

                function handleClick() {
                    if (vpnFlickable.vpnIsOff) {
                        VPNSettings.protectSelectedApps = !VPNSettings.protectSelectedApps
                    }
                }
            }

            VPNExpandableAppList {
                id: enabledList
                Layout.fillWidth: true
                Layout.leftMargin: VPNTheme.theme.vSpacing
                Layout.rightMargin: VPNTheme.theme.vSpacing
                Layout.preferredWidth: parent.width - VPNTheme.theme.vSpacing * 2
                Layout.maximumWidth: parent.width - VPNTheme.theme.vSpacing * 2
                Layout.topMargin: VPNTheme.theme.vSpacing
                searchBarPlaceholder: searchApps

                //% "Exclude apps from VPN protection"
                //: Header for the list of apps protected by VPN
                header: qsTrId("vpn.settings.excludeTitle")
            }

            VPNTextBlock {
                id: helpInfoText
                width: undefined
                Layout.preferredWidth: parent.width - VPNTheme.theme.vSpacing * 2
                Layout.leftMargin: VPNTheme.theme.vSpacing
                Layout.rightMargin: VPNTheme.theme.vSpacing
                Layout.topMargin: VPNTheme.theme.vSpacing
                text: VPNl18n.SplittunnelInfoText
            }

            VPNLinkButton {
                id: helpLink
                labelText: VPNl18n.SplittunnelInfoLinkText
                Layout.preferredWidth: col.width
                Layout.alignment: Qt.AlignHCenter
                Layout.topMargin: VPNTheme.theme.windowMargin / 2
                onClicked: {
                   VPN.openLink(VPN.LinkSplitTunnelHelp)
                }
            }
        }
    }
}
