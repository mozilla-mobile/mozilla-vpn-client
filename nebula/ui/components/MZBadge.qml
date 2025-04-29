/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0

import Mozilla.Shared 1.0

Rectangle {
    enum BadgeType {
        Success,
        Normal,
        Warning,
        Error
    }

    id: root

    // Private property, will be changed depending on badgeType
    QtObject {
        id: style
        property var backgroundColor: {
          switch (root.badgeType) {
            case MZBadge.BadgeType.Success:
              return MZTheme.colors.successBackground
            case MZBadge.BadgeType.Normal:
              return MZTheme.colors.normalLevelBackground
            case MZBadge.BadgeType.Warning:
              return MZTheme.colors.warningBackground
            case MZBadge.BadgeType.Error:
              return MZTheme.colors.errorBackground
          }
        }

        property var textColor: {
          switch (root.badgeType) {
            case MZBadge.BadgeType.Success:
              return MZTheme.colors.successText
            case MZBadge.BadgeType.Normal:
              return MZTheme.colors.normalLevelText
            case MZBadge.BadgeType.Warning:
              return MZTheme.colors.warningText
            case MZBadge.BadgeType.Error:
              return MZTheme.colors.errorText
          }
        }
    }

    property var badgeType: MZBadge.BadgeType.Normal
    property alias text: badgeLabel.text
    property alias badgeLabel: badgeLabel

    color: style.backgroundColor
    height: badgeLabel.height
    width: badgeLabel.width
    radius: 4
    clip: true

    MZBoldInterLabel {
        id: badgeLabel

        text: badgeType.badgeText
        color: style.textColor
        verticalAlignment: Text.AlignVCenter
        topPadding: MZTheme.theme.badgeVerticalPadding
        leftPadding: MZTheme.theme.badgeHorizontalPadding
        rightPadding: MZTheme.theme.badgeHorizontalPadding
        bottomPadding: MZTheme.theme.badgeVerticalPadding
        maximumLineCount: 1
        elide: Text.ElideRight
    }
}
