/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import components 0.1
import telemetry 0.30

VPNIconButton {
    id: btn

    property var _screen
    property alias _source: image.source
    property bool _hasNotification: false

    accessibleName: ""

    onClicked: {
        switch(_screen) {
            case VPNNavigator.ScreenMessaging:
                VPN.recordGleanEventWithExtraKeys("bottomNavigationBarClick", {"bar_button": "ScreenMessaging"});
                break;

            case VPNNavigator.ScreenHome:
                VPN.recordGleanEventWithExtraKeys("bottomNavigationBarClick", {"bar_button": "ScreenHome"});
                break;

            case VPNNavigator.ScreenSettings:
                VPN.recordGleanEventWithExtraKeys("bottomNavigationBarClick", {"bar_button": "ScreenSettings"});
                break;

            default:
                VPN.recordGleanEventWithExtraKeys("bottomNavigationBarClick", {"bar_button": "Unknown"});
                break;
        }
        VPNNavigator.requestScreen(_screen, VPNNavigator.screen === _screen ? VPNNavigator.ForceReload : VPNNavigator.NoFlags);
    }

    width: VPNTheme.theme.navBarIconSize
    height: VPNTheme.theme.navBarIconSize

    backgroundRadius: height / 2
    buttonColorScheme: VPNTheme.theme.iconButtonDarkBackground
    uiStatesVisible: !checked

    Rectangle {
        id: circleBackground
        anchors.fill: parent
        visible: checked
        radius: parent.height / 2
        opacity: .2
        color: VPNTheme.theme.white
    }

    Image {
        id: image
        anchors.centerIn: parent
        sourceSize.height: VPNTheme.theme.iconSize * 2
        sourceSize.width: VPNTheme.theme.iconSize * 2
    }
}
