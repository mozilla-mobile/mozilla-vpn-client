/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const theme = {};

theme.fontFamily = 'Metropolis';
theme.fontBoldFamily = 'MetropolisSemiBold';
theme.fontInterFamily = 'InterUI';
theme.fontInterSemiBoldFamily = 'InterSemiBold';
theme.fontSize = 15;
theme.fontSizeLarge = 22;
theme.fontSizeSmall = 13;
theme.fontSizeSmallest = 11;
theme.fontWeightBold = 600;
theme.iconSize = 16;
theme.iconSizeFlag = 72;
theme.iconSizeSmall = 14;
theme.labelLineHeight = 22;
theme.cityListTopMargin = 18;
theme.controllerInterLineHeight = 18;
theme.hSpacing = 20;
theme.vSpacing = 24;
theme.vSpacingSmall = 16;
theme.listSpacing = 8;
theme.maxTextWidth = 296;
theme.windowMargin = 16;
theme.popupMargin = 24;
theme.sheetTopMargin = 32
theme.desktopAppHeight = 640;
theme.desktopAppWidth = 360;
theme.tabletMinimumWidth = 600;
theme.largePhoneHeight = 852
theme.menuHeight = 56;
theme.viewBaseTopMargin = 16;
theme.checkBoxRowSubLabelTopMargin = 2;
theme.dividerHeight = 1

theme.helpSheetTitleBodySpacing = 8
theme.helpSheetBodySpacing = 16
theme.helpSheetBodyButtonSpacing = 16
theme.helpSheetSecondaryButtonSpacing = 8

theme.navBarHeight = 64;
theme.navBarMaxWidth = 608;
theme.navBarTopMargin = 48;
theme.navBarBottomMargin = Qt.platform.os !== 'ios' ? 16 : 34;
theme.navBarHeightWithMargins =
theme.navBarHeight + theme.navBarTopMargin + theme.navBarBottomMargin;
theme.navBarIconSize = 48
theme.navBarMaxPaddingTablet = 120
theme.navBarMaxPadding = 48

theme.onboardingMinimumVerticalSpacing = 16;

theme.swipeDelegateActionWidth = 56;
theme.badgeHorizontalPadding = 8
theme.badgeVerticalPadding = 3

theme.checkBoxHeightWidth = 20;
theme.checkmarkHeightWidth = 12;

theme.progressBarDelegateHeightWidth = 32

theme.maxZLevel = 10

theme.toggleHeight = 24
theme.toggleWidth = 45
theme.toggleRowDividerSpacing = 16

theme.opacityDisabled = .5;

theme.cornerRadius = 4;
theme.focusBorderWidth = 2;

// In milliseconds, the animation of a single device removal
theme.removeDeviceAnimation = 300;
theme.rowHeight = 40;
theme.settingsMaxContentHeight = 740;
theme.maxHorizontalContentWidth = 460;
theme.contentTopMarginDesktop = 20;
theme.contentTopMarginMobile = 48;
theme.contentBottomMargin = 20;

theme.uiState = {
  'stateDefault': 'state-default',
  'stateHovered': 'state-hovered',
  'statePressed': 'state-pressed',
  'stateFocused': 'state-focused',
  'stateLoading': 'state-loading',
  'stateDisabled': 'state-disabled',
};

theme;
