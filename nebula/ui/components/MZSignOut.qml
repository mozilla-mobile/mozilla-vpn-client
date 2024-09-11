/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0

MZFooterLink {
    enum ColorStrength {
        Standard,
        Soft
    }

    labelText: MZI18n.GlobalSignOut
    fontName: MZTheme.theme.fontBoldFamily
    linkColor: getLinkColor()
    property int colorStrength: MZSignOut.ColorStrength.Standard

    property var preLogoutCallback: () => {}

    function getLinkColor() {
        switch (colorStrength) {
        case MZSignOut.ColorStrength.Standard:
            return MZTheme.theme.redLinkButton
        case MZSignOut.ColorStrength.Soft:
            return MZTheme.theme.blueButton
        default:
            return console.error("Unable to create view for footer link of type: " + colorStrength)
        }
    }

    onClicked: () => {
        preLogoutCallback();
        VPN.logout();
    }
}
