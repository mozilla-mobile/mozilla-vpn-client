/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0


ColumnLayout {
    height: {
        let footerHeight = 0;
        if (navbar.visible) footerHeight += VPNTheme.theme.navBarHeight + (VPNTheme.theme.navBarBottomMargin * 2)
        return footerHeight
    }
}
