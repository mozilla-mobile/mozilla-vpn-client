/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const theme = {};

theme.transparent = '#00000000';

theme.bgColor = '#F9F9FA';
theme.bgColor30 = '#4DF9F9FA';
theme.bgColor80 = '#CCF9F9FA';
theme.bgColorTransparent = '#00F9F9FA';

theme.blue = '#0060DF';
theme.blueHovered = '#0250BB';
theme.bluePressed = '#054096';
theme.blueDisabled = '#a3c0f3';
theme.blueFocusOutline = '#4d0a84ff';
theme.blueFocusBorder = '#0a84ff';

theme.divider = '#0C0C0D0A'
theme.green = '#3FE1B0';
theme.grey = '#CACACA';
theme.greyHovered = '#E6E6E6';
theme.greyPressed = '#C2C2C2';
theme.greyDisabled = '#D8D8D8';
theme.ink = '#321C64';
theme.orange = '#FFA436';
theme.orangeHovered = '#E27F2E';
theme.orangeFocus = '#4DE27F2E';
theme.orangePressed = '#C45A27';
theme.purple60 = '#7542E5';
theme.red = '#E22850';
theme.redHovered = '#C50042';
theme.redPressed = '#810220';
theme.redDisabled = '#FFBDC5';
theme.redfocusOutline = '#66C50042';
theme.redBadgeText = "#810220";
theme.greenBadgeText = '#00736C';
theme.greenBadgeBackground = '#E3FFF3';
theme.orangeBadgeText = '#960E18';
theme.orangeBadgeBackground = '#FFD5B2';
theme.blueBadgeText  = "#09204D"
theme.blueBadgeBackground = "#660060DF"

theme.white = '#FFFFFF'

theme.overlayBackground = '#4D000000';

theme.checkBoxWarning = '#C45A27';

theme.guideCardImageBgColor = "#50134B"
theme.tutorialCardImageBgColor = "#321C64"

theme.fontColor = '#6D6D6E';
theme.fontColorDark = '#3D3D3D';
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
theme.desktopAppHeight = 640;
theme.desktopAppWidth = 360;
theme.tabletMinimumWidth = 600;
theme.menuHeight = 56;

theme.tutorialCardHeight = 144
theme.guideCardHeight = 172

theme.tutorialCardHeight = 144;
theme.navBarHeight = 64;
theme.navBarMaxWidth = 608;
theme.navBarTopMargin = 48;
theme.navBarBottomMargin = Qt.platform.os !== 'ios' ? 16 : 34;
theme.navBarHeightWithMargins =
theme.navBarHeight + theme.navBarTopMargin + theme.navBarBottomMargin;
theme.navBarIconSize = 48
theme.navBarMaxPaddingTablet = 120
theme.navBarMaxPadding = 48

theme.swipeDelegateActionWidth = 56;
theme.badgeHorizontalPadding = 8
theme.badgeVerticalPadding = 3

theme.checkBoxHeightWidth = 20;
theme.checkmarkHeightWidth = 12;

theme.progressBarDelegateHeightWidth = 32

theme.darkFocusBorder = theme.fontColor;
theme.lightFocusBorder = '#d5d3e0';

theme.maxZLevel = 10


theme.onBoardingGradient = {
  'start': '#472C87',
  'middle': '#301962',
  'end': '#1D0942',
};

theme.blueButton = {
  'defaultColor': theme.blue,
  'buttonHovered': theme.blueHovered,
  'buttonPressed': theme.bluePressed,
  'buttonDisabled': theme.blueDisabled,
  'focusBgColor': theme.blue,
  'focusOutline': theme.blueFocusOutline,
  'focusBorder': theme.blueFocusBorder,
};

theme.wasmOptionBtn = {
  'defaultColor': '#00eeeeee',
  'buttonHovered': '#330a84ff',
  'buttonPressed': '#4d0a84ff',
  'buttonDisabled': theme.blueDisabled,
  'focusBgColor': theme.blue,
  'focusOutline': theme.blueFocusOutline,
  'focusBorder': theme.blueFocusBorder,
};

theme.warningAlertFocusBorders = {
  'focusOutline': theme.orangeFocus,
  'focusBorder': theme.orangePressed,
};

theme.clickableRowBlue = {
  'defaultColor': theme.bgColor,
  'buttonHovered': '#D4E2F6',
  'buttonPressed': '#AECBF2',
  'focusOutline': theme.bgColorTransparent,
  'focusBorder': theme.blueFocusBorder,
};

theme.clickableRowPurple = {
  'defaultColor': theme.bgColorTransparent,
  'buttonHovered': '#E7DFFF',
  'buttonPressed': '#D9BFFF',
  'focusOutline': theme.bgColorTransparent,
  'focusBorder': '#592ACB',
};

theme.iconButtonLightBackground = {
  'defaultColor': theme.bgColorTransparent,
  'buttonHovered': theme.greyHovered,
  'buttonPressed': theme.greyPressed,
  'buttonDisabled': theme.transparent,
  'focusOutline': theme.bgColorTransparent,
  'focusBorder': theme.darkFocusBorder,
};

theme.iconButtonDarkBackground = {
  'defaultColor': '#00321C64',
  'buttonHovered': '#5b4983',
  'buttonPressed': '#8477a2',
  'buttonDisabled': '#00321C64',
  'focusOutline': '#005b4983',
  'focusBorder': theme.lightFocusBorder,
};

theme.linkButton = {
  'defaultColor': theme.bgColorTransparent,
  'buttonHovered': theme.bgColorTransparent,
  'buttonPressed': theme.bgColorTransparent,
  'buttonDisabled': theme.bgColorTransparent,
  'focusOutline': theme.bgColorTransparent,
  'focusBorder': theme.bgColorTransparent,
};

theme.whiteButton = {
  'defaultColor': theme.white,
  'buttonHovered': theme.greyHovered,
  'buttonPressed': theme.greyPressed,
  'focusOutline': theme.greyHovered,
  'focusBorder': theme.greyPressed,
};

theme.greyButton = {
  'defaultColor': '#CECECF',
  'buttonHovered': theme.greyHovered,
  'buttonPressed': theme.greyPressed,
  'buttonDisabled': theme.greyDisabled,
  'focusOutline': theme.greyHovered,
  'focusBorder': theme.greyPressed,
};

theme.redButton = {
  'defaultColor': theme.red,
  'buttonHovered': theme.redHovered,
  'buttonPressed': theme.redPressed,
  'buttonDisabled': theme.redDisabled,
  'focusOutline': theme.redDisabled,
  'focusBorder': theme.redHovered,
};

theme.redLinkButton = {
  'defaultColor': theme.redHovered,
  'buttonHovered': theme.redPressed,
  'buttonPressed': theme.redPressed,
  'buttonDisabled': theme.redDisabled,
  'focusOutline': theme.redfocusOutline,
  'focusBorder': theme.redPressed,
};

theme.removeDeviceBtn = {
  'defaultColor': theme.bgColorTransparent,
  'buttonHovered': '#FFDFE7',
  'buttonPressed': '#FFBDC5',
  'buttonDisabled': theme.bgColorTransparent,
  'focusOutline': theme.bgColorTransparent,
  'focusBorder': theme.red,
};

theme.vpnToggleConnected = {
  'defaultColor': '#3FE1B0',
  'buttonHovered': '#3AD4B3',
  'buttonPressed': '#1CC5A0',
  'focusOutline': theme.bgColor30,
  'focusBorder': theme.lightFocusBorder,
};

theme.vpnToggleDisconnected = {
  'defaultColor': '#9E9E9E',
  'buttonHovered': theme.fontColor,
  'buttonPressed': theme.fontColorDark,
  'buttonDisabled': theme.grey,
  'focusOutline': 'transparent',
  'focusBorder': theme.darkFocusBorder,
};

theme.card = {
    'defaultColor': theme.white,
    'buttonHovered': theme.greyHovered,
    'buttonPressed': theme.greyPressed,
    'buttonDisabled': theme.bgColorTransparent,
    'focusOutline': theme.bgColorTransparent,
    'focusBorder': theme.darkFocusBorder,
};

theme.greenAlert = {
  defaultColor: '#3FE1B0',
  buttonHovered: '#3AD4B3',
  buttonPressed: '#1CC5A0',
  focusOutline: '#333FE1B0',
  focusBorder: '#1CC5A0',
};

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

theme.greyLink = {
  'defaultColor': '#B30C0C0D',
  'buttonHovered': '#CC0C0C0D',
  'buttonPressed': '#FF0C0C0D',
  'focusOutline': '#FF0C0C0D',
  'focusBorder': '#000000'
};

theme.input = {
  'backgroundColor': '#ffffff',
  'borderColor': '#9E9E9E',
  'highlight': '#E7E7E7',
  'defaultColor': '#ffffff',
  'buttonHovered': theme.blueHovered,
  'buttonPressed': theme.bluePressed,
  'buttonDisabled': theme.blueDisabled,
  'focusBgColor': theme.blue,
  'focusOutline': theme.blueFocusOutline,
  'focusBorder': theme.blueFocusBorder,
};

theme.greenBadge = {
    'textColor': theme.greenBadgeText,
    'backgroundColor': theme.greenBadgeBackground
};

theme.redBadge = {
    'textColor': theme.redBadgeText,
    'backgroundColor': theme.redDisabled
};

theme.orangeBadge = {
    'textColor': theme.orangeBadgeText,
    'backgroundColor': theme.orangeBadgeBackground
};

theme.blueBadge = {
    'textColor': theme.blueBadgeText,
    'backgroundColor': theme.blueBadgeBackground
};

theme.purpleStepProgressBarDelegate = {
    'defaultColor': '#592ACB',
    'buttonHovered': '#592ACB',
    'buttonPressed': '#45278D',
    'buttonDisabled': '#9E9E9E',
    'focusOutline': theme.bgColorTransparent,
    'focusBorder': theme.bgColorTransparent,
};

theme;
