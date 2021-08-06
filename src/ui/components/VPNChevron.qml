/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14
import "../themes/themes.js" as Theme

// VPNChevron
Image {
    property bool isDark: false

    source: isDark ? "../resources/chevron-right-dark.svg" : "../resources/chevron.svg"
    sourceSize.height: 24
    sourceSize.width: 24
    fillMode: Image.PreserveAspectFit
}
