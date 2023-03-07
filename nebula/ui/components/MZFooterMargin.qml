/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0

ColumnLayout {
    height: {
        let footerHeight = 0;
        if (navbar.visible) footerHeight += MZTheme.theme.navBarHeight + (MZTheme.theme.navBarBottomMargin * 4)
        return footerHeight
    }
}
