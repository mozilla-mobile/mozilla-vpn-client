/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const color = {};

/**
 * Part 1: Basic colors
 * Should NOT be used directly in code.
 * (One exception: color.transparent can be used in code.)
 *
 * Primary palette
 */
// Purple
color.purple90 = '#321C64';
color.purple80 = '#45278D';
color.purple70 = '#592ACB';
color.purple60 = '#7542E5';
color.purple50 = '#9059FF';
color.purple40 = '#AB71FF';
color.purple30 = '#C689FF';
color.purple20 = '#CB9EFF';
color.purple10 = '#D9BFFF';
color.purple5 = '#E7DFFF';

// Green
color.green90 = '#00736C';
color.green80 = '#00A49A';
color.green70 = '#1CC4A0';
color.green60 = '#3AD4B3';
color.green50 = '#3FE1B0';
color.green40 = '#54FFBD';
color.green30 = '#88FFD1';
color.green20 = '#B3FFE3';
color.green10 = '#D1FFEE';
color.green5 = '#E3FFF3';

// Grey
color.black = '#000000';
color.grey60 = '#0C0C0D';
color.grey50 = '#3D3D3D';
color.grey40 = '#6D6D6E';
color.grey30 = '#9E9E9E';
color.grey20 = '#CECECF';
color.grey15 = '#D8D8D8';
color.grey10 = '#E7E7E7';
color.grey5 = '#F9F9FA';
color.white = '#FFFFFF';
color.white80 = '#CCFFFFFF';

/**
 * Secondary palette
 */
// Violet
color.violet90 = '#2B1141';
color.violet80 = '#4E1A69';
color.violet70 = '#722291';
color.violet60 = '#952BB9';
color.violet50 = '#B833E1';
color.violet40 = '#D74CF0';
color.violet30 = '#F770FF';
color.violet20 = '#F68FFF';
color.violet10 = '#F6B8FF';
color.violet5 = '#F7E2FF';

// Blue
color.blue90 = '#09204D';
color.blue80 = '#073072';
color.blue70 = '#054096';
color.blue60 = '#0250BB';
color.blue50 = '#0060DF';
color.blue40 = '#0090ED';
color.blue30 = '#00B3F4';
color.blue20 = '#00DDFF';
color.blue10 = '#80EBFF';
color.blue5 = '#AAF2FF';

// Orange
color.orange90 = '#7C1504';
color.orange80 = '#9E280B';
color.orange70 = '#CC3D00';
color.orange60 = '#E25920';
color.orange50 = '#FF7139';
color.orange40 = '#FF8A50';
color.orange30 = '#FFA266';
color.orange20 = '#FFB587';
color.orange10 = '#FFD5B2';
color.orange5 = '#FFF4DE';

// Yellow
color.yellow90 = '#960E18';
color.yellow80 = '#A7341F';
color.yellow70 = '#C45A27';
color.yellow60 = '#E27F2E';
color.yellow50 = '#FFA436';
color.yellow40 = '#FFBD4F';
color.yellow30 = '#FFD567';
color.yellow20 = '#FFEA80';
color.yellow10 = '#FFFF98';
color.yellow5 = '#FFFFCC';

// Red
color.red90 = '#440306';
color.red80 = '#810220';
color.red70 = '#C50042';
color.red60 = '#E22850';
color.red50 = '#FF4F5E';
color.red40 = '#FF6A75';
color.red30 = '#FF848B';
color.red20 = '#FF9AA2';
color.red10 = '#FFBDC5';
color.red5 = '#FFDFE7';

// Pink
color.pink90 = '#50134B';
color.pink80 = '#7F145B';
color.pink70 = '#C60084';
color.pink60 = '#E31587';
color.pink50 = '#FF298A';
color.pink40 = '#FF4AA2';
color.pink30 = '#FF6BBA';
color.pink20 = '#FF8AC5';
color.pink10 = '#FFB4DB';
color.pink5 = '#FFDEF0';

// additional colors
color.dullGreen = '#387E8A';
color.dullPurple = '#998DB2';
color.washedPurple = '#5B4983';
color.washedBlue = '#D4E2F6';
color.washedLightBlue = '#AECBF2';
color.strongBlue = '#0A84FF';

color.onboardingGradientStart = '#472C87';
color.onboardingGradientMiddle = '#301962';
color.onboardingGradientEnd = '#1D0942';

color.transparent = '#00000000';

/**
 * Helper functions
 */
const percentToHex = percent => {
  const int8Bit = Math.round(percent * 255);
  const hexAlphaValue = int8Bit.toString(16).padStart(2, '0');

  return hexAlphaValue.toUpperCase();
};

// Not all #00xxxxx colors are identical. While they look identical
// on screen, in transitions or with opacity changes they may look different.
// Changing all `addTransparency(color.[anything], 0.0)` to color.transparency
// will cause UI bugs.
const addTransparency = (hexColor, percent) => {
  const hexValue = hexColor.substring(1);
  const hexAlphaValue = percentToHex(percent);

  // Despite typical web usage being #{colorHex}{alphaHex}, Qt uses
  // #{alphaHex}{colorHex}: https://doc.qt.io/qt-6/qcolor.html#fromString
  return `#${hexAlphaValue}${hexValue}`;
};

/**
 * Part 2: Functional colors
 * Should NOT be used directly in code
 */
color.blue = color.blue50;
color.blueFocus = addTransparency(color.blue, 0.4);
color.blueHovered = color.blue60;
color.bluePressed = color.blue70;
color.blueFocusBorder = color.strongBlue;
color.blueBadgeText = color.blue90;
color.blueBadgeBackground = addTransparency(color.blue50, 0.4);

color.green = color.green50;
color.greenActive = color.green70;
color.greenHover = color.green60;
color.greenFocus = color.green20;

color.red = color.red60;
color.redHovered = color.red70;
color.redPressed = color.red80;
color.redDisabled = color.red10;
color.redFocusOutline = addTransparency(color.red70, 0.4);
color.redFocus = color.red10;

color.yellow = color.yellow50;
color.yellowActive = color.yellow70;
color.yellowHover = color.yellow60;
color.yellowFocus = color.orange10;

color.orange = color.yellow50;
color.orangeHovered = color.yellow60;
color.orangeFocus = addTransparency(color.yellow60, 0.3);
color.orangePressed = color.yellow70;

color.greyHovered = color.grey10;
color.greyPressed = color.grey20;
color.greyDisabled = color.grey15;

color.blueButton = {
  defaultColor: color.blue,
  buttonHovered: color.blueHovered,
  buttonPressed: color.bluePressed,
  buttonDisabled: color.washedLightBlue,
  focusBgColor: color.blue,
  focusOutline: addTransparency(color.strongBlue, 0.3),
  focusBorder: color.blueFocusBorder,
};

color.clickableRowBlue = {
  defaultColor: color.grey5,
  buttonHovered: color.washedBlue,
  buttonPressed: color.washedLightBlue,
  focusOutline: addTransparency(color.grey5, 0.0),
  focusBorder: color.blueFocusBorder,
};

color.whiteButton = {
  defaultColor: color.white,
  buttonHovered: color.greyHovered,
  buttonPressed: color.greyPressed,
  focusOutline: color.greyHovered,
  focusBorder: color.greyPressed,
};

color.greyButton = {
  defaultColor: color.grey20,
};

color.redButton = {
  defaultColor: color.red,
  buttonHovered: color.redHovered,
  buttonPressed: color.redPressed,
  buttonDisabled: color.redDisabled,
  focusOutline: color.redDisabled,
  focusBorder: color.redHovered,
};

color.redLinkButton = {
  defaultColor: color.redHovered,
  buttonHovered: color.redPressed,
  buttonPressed: color.redPressed,
  buttonDisabled: color.redDisabled,
  focusOutline: color.redFocusOutline,
  focusBorder: color.redPressed,
};

color.greenAlert = {
  defaultColor: color.green50,
  buttonHovered: color.green60,
  buttonPressed: color.green70,
  focusOutline: addTransparency(color.green50, 0.2),
  focusBorder: color.green70,
};

color.greyLink = {
  defaultColor: addTransparency(color.grey60, 0.7),
  buttonHovered: addTransparency(color.grey60, 0.8),
  buttonPressed: color.grey60,
  focusOutline: color.grey60,
  focusBorder: color.black
};

color.greenBadge = {
  textColor: color.green90,
  backgroundColor: color.green5
};

color.redBadge = {
  textColor: color.red80,
  backgroundColor: color.redDisabled
};

color.orangeBadge = {
  textColor: color.yellow90,
  backgroundColor: color.orange10
};

color.blueBadge = {
  textColor: color.blueBadgeText,
  backgroundColor: color.blueBadgeBackground
};

/**
 * Part 3: Theme colors
 * Named colors in this section should only be set to colors in part 1 or 2.
 *
 * These named colors in this section SHOULD be used in code - and should be
 * the ONLY colors used in code.
 *
 * One exception: color.transparent can be used in code.
 */
color.primary = color.purple90;
color.bgColor = color.grey5;
color.bgColorTransparent = addTransparency(color.bgColor, 0.0);
color.divider = addTransparency(color.grey60, 0.04);
color.overlayBackground = addTransparency(color.black, 0.0);
color.fontColor = color.grey40;
color.fontColorDark = color.grey50;

color.darkFocusBorder = color.grey40;
color.lightFocusBorder = color.grey15;

color.informational = {
  default: color.blue,
  focus: color.blueFocus,
};

color.success = {
  default: color.green
};

color.error = {
  default: color.red50,
  focus: color.redFocus,
};

color.warning = {
  default: color.yellow,
  active: color.yellowActive,
};

color.inputState = {
  default: {
    border: color.grey30,
    placeholder: color.grey40,
    text: color.grey50,
  },
  hover: {
    border: color.grey40,
    placeholder: color.grey50,
    text: color.grey50,
  },
  focus: {
    border: color.informational.default,
    highlight: color.informational.focus,
    placeholder: color.grey40,
    text: color.grey50,
  },
  error: {
    border: color.error.default,
    highlight: color.error.focus,
    placeholder: color.grey40,
    text: color.grey50,
  },
  disabled: {
    border: color.grey20,
    placeholder: color.grey20,
    text: color.grey20,
  },
};

color.onBoardingGradient = {
  start: color.onboardingGradientStart,
  middle: color.onboardingGradientMiddle,
  end: color.onboardingGradientEnd,
};

color.iconButtonLightBackground = {
  defaultColor: color.bgColorTransparent,
  buttonHovered: color.greyHovered,
  buttonPressed: color.greyPressed,
  buttonDisabled: color.transparent,
  focusOutline: color.bgColorTransparent,
  focusBorder: color.darkFocusBorder,
};

color.iconButtonDarkBackground = {
  defaultColor: addTransparency(color.purple90, 0.0),
  buttonHovered: color.washedPurple,
  buttonPressed: color.dullPurple,
  buttonDisabled: addTransparency(color.purple90, 0.0),
  focusOutline: color.transparent,
  focusBorder: color.lightFocusBorder,
};

color.linkButton = {
  defaultColor: color.bgColorTransparent,
  buttonHovered: color.bgColorTransparent,
  buttonPressed: color.bgColorTransparent,
  buttonDisabled: color.bgColorTransparent,
  focusOutline: color.bgColorTransparent,
  focusBorder: color.bgColorTransparent,
};

color.vpnToggleConnected = {
  defaultColor: color.green,
  buttonHovered: color.green60,
  buttonPressed: color.green70,
  focusOutline: addTransparency(color.bgColor, 0.3),
  focusBorder: color.lightFocusBorder,
};

color.vpnToggleDisconnected = {
  defaultColor: color.grey30,
  buttonHovered: color.fontColor,
  buttonPressed: color.fontColorDark,
  buttonDisabled: color.grey,
  focusOutline: color.transparent,
  focusBorder: color.darkFocusBorder,
};

color.input = {
  backgroundColor: color.white,
  highlight: color.grey10,
};

color.card = {
  defaultColor: color.white,
  buttonHovered: color.greyHovered,
  buttonPressed: color.greyPressed,
  buttonDisabled: color.bgColorTransparent,
  focusOutline: color.bgColorTransparent,
  focusBorder: color.darkFocusBorder,
};

color.stepProgressBarDelegate = {
  defaultColor: color.purple70,
  buttonHovered: color.purple70,
  buttonPressed: color.purple80,
  buttonDisabled: color.grey30,
  focusOutline: color.bgColorTransparent,
  focusBorder: color.bgColorTransparent,
};

color;
