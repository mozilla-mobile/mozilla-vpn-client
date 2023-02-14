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

    _menuTitle: VPNI18n.SettingsAppExclusionSettings
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
                                               // Pin y height to be below the alert bar as we can't render above it
                                               setY: vpnFlickable.y+VPNTheme.theme.windowMargin,
                                               onActionPressed: ()=>{VPNAppPermissions.openFilePicker();},
                                           });

                    alert.show();

                }
            }
        }

        AppPermissionsList {
            id: enabledList
            Layout.fillWidth: true
            Layout.fillHeight: false
            Layout.leftMargin: VPNTheme.theme.vSpacing
            Layout.rightMargin: VPNTheme.theme.vSpacing
            searchBarPlaceholder: searchApps
        }
    }
    Component.onCompleted: {
        console.log("Component ready");
        VPNAppPermissions.requestApplist();
        MZGleanDeprecated.recordGleanEvent("appPermissionsViewOpened");
        Glean.sample.appPermissionsViewOpened.record();
    }

}
