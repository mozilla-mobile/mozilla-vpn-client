/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1


VPNViewBase {
    //% "Search apps"
    //: Search bar placeholder text
    property string searchApps: qsTrId("vpn.protectSelectedApps.searchApps")

    //% "Add application"
    //: Button label
    property string addApplication: qsTrId("vpn.protectSelectedApps.addApplication")

    id: vpnFlickable
    objectName: "appPermissions"

    _menuTitle: VPNl18n.SettingsAppExclusionSettings
    _viewContentData: ColumnLayout {
        Layout.fillWidth: true

        ColumnLayout{
            id: messageBox
            visible: true
            Layout.leftMargin: VPNTheme.theme.windowMargin
            Layout.rightMargin: VPNTheme.theme.windowMargin * 2
            Layout.topMargin: -VPNTheme.theme.windowMargin
            Layout.fillWidth: true
            Layout.fillHeight: false
            spacing: 0

            Connections {
                target: VPNAppPermissions
                function onNotification(type,message,action) {
                    console.log("Got notification: "+type + "  message:"+message);
                    var component = Qt.createComponent("qrc:/nebula/components/VPNAlert.qml");
                    if(component.status !== Component.Ready)
                        {
                            if( component.status == Component.Error )
                                console.debug("Error:"+ component.errorString() );

                        }
                    var alert = component.createObject(vpnFlickable, {
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
            toggleEnabled: true
            Layout.fillWidth: true
            Layout.preferredHeight: childrenRect.height + VPNTheme.theme.windowMargin

            //% "Protect all apps with VPN"
            labelText: qsTrId("vpn.settings.protectAllApps")

            //% "VPN protects all apps by default. Turn off to choose which apps Mozilla VPN should not protect."
            sublabelText: qsTrId("vpn.settings.protectAllApps.description")

            toolTipTitleText: qsTrId("vpn.settings.protectAllApps")

            toggleChecked: (!VPNSettings.protectSelectedApps)

            function handleClick() {
                VPNSettings.protectSelectedApps = !VPNSettings.protectSelectedApps
            }
        }

        AppPermissionsList {
            id: enabledList
            Layout.fillWidth: true
            Layout.fillHeight: false
            Layout.leftMargin: VPNTheme.theme.vSpacing
            Layout.rightMargin: VPNTheme.theme.vSpacing
            Layout.topMargin: VPNTheme.theme.vSpacing
            searchBarPlaceholder: searchApps

            //% "Exclude apps from VPN protection"
            //: Header for the list of apps protected by VPN
            header: qsTrId("vpn.settings.excludeTitle")
        }

        VPNTextBlock {
            id: helpInfoText
            width: undefined
            Layout.fillWidth: true
            Layout.leftMargin: VPNTheme.theme.vSpacing
            Layout.rightMargin: VPNTheme.theme.vSpacing
            Layout.topMargin: VPNTheme.theme.vSpacing
            text: VPNl18n.SplittunnelInfoText
        }

        VPNLinkButton {
            id: helpLink
            labelText: VPNl18n.SplittunnelInfoLinkText
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: VPNTheme.theme.windowMargin / 2
            onClicked: {
               VPNUrlOpener.openUrlLabel("splitTunnelHelp")
            }
        }
    }
    Component.onCompleted: {
        console.log("Component ready");
        VPNAppPermissions.requestApplist();
        VPNGleanDeprecated.recordGleanEvent("appPermissionsViewOpened");
        Glean.sample.appPermissionsViewOpened.record();
    }

}
