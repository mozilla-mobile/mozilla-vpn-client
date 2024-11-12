/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const color = {};

/**
 * Part 1: Basic colors
 * Should NOT be used directly in code, and colors should not be modified.
 * (Can add to this section if really, truly needed.)
 *
 * One exception: color.transparent can be used in code.
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
color.red75 = '#A3012B';
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

color.lighterOnboardingPurple = '#472C87';
color.mediumOnboardingPurple = '#301962';
color.darkerOnboardingPurple = '#1D0942';

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
 * Should NOT be used directly in code, and colors should not be modified.
 * (Can add to this section if really, truly needed.)
 */
color.blue = color.blue50;
color.blueHovered = color.blue60;
color.bluePressed = color.blue70;
color.blueFocusBorder = color.strongBlue;

color.green = color.green50;
color.red = color.red60;
color.orange = color.yellow50;

color.greyHovered = color.grey10;
color.greyPressed = color.grey20;

color.blueButton = {
  defaultColor: color.blue,
  buttonHovered: color.blueHovered,
  buttonPressed: color.bluePressed,
  buttonDisabled: color.washedLightBlue,
  focusBgColor: color.blue,
  focusOutline: addTransparency(color.blueFocusBorder, 0.3),
  focusBorder: color.blueFocusBorder,
};

color.clickableRowBlue = {
  defaultColor: color.grey5,
  buttonHovered: color.washedBlue,
  buttonPressed: color.washedLightBlue,
  focusOutline: addTransparency(color.grey5, 0.0),
  focusBorder: color.blueFocusBorder,
};

color.redButton = {
  defaultColor: color.red70,
  buttonHovered: color.red75,
  buttonPressed: color.red80,
  buttonDisabled: color.red10,
  focusOutline: addTransparency(color.red70, 0.4),
  focusBorder: color.red70,
};

color.greenAlert = {
  defaultColor: color.green,
  buttonHovered: color.green60,
  buttonPressed: color.green70,
  focusOutline: addTransparency(color.green, 0.2),
  focusBorder: color.green70,
};

color.orangeAlert = {
  buttonHovered: color.yellow60,
  buttonPressed: color.yellow70,
  focusOutline: addTransparency(color.yellow60, 0.3),
  focusBorder: color.yellow70,
};

color.greyLink = {
  defaultColor: addTransparency(color.grey60, 0.7),
  buttonHovered: addTransparency(color.grey60, 0.8),
  buttonPressed: color.grey60,
  focusOutline: color.grey60,
  focusBorder: color.black
};

/**
 * Part 3: Theme colors
 * Named colors in this section should only be set to colors in part 1 or 2.
 *
 * The named colors in this section and section 4 SHOULD be used in code -
 * and should be the ONLY colors used in code. (One exception: color.transparent
 * can be used in code.)
 */
// Primary
color.primary = color.purple90;
color.primaryBrighter = color.purple70;
color.primaryHovered = color.washedPurple;
color.primaryPressed = color.dullPurple;

// Backgrounds
color.bgColor = color.grey5;
color.bgColorTransparent = addTransparency(color.bgColor, 0.0);
color.bgColorStronger = color.white;
color.overlayBackground = addTransparency(color.black, 0.0);

// Fonts
color.fontColor = color.grey40;
color.fontColorDark = color.grey50;
color.fontColorWarning = color.yellow70;
color.fontColorInverted = color.white;
color.fontColorInvertedMuted = color.grey20;
color.fontTitleColor = color.black;

// Dividers and borders
color.divider = color.grey10;
color.dropShadow = color.grey60;
color.darkFocusBorder = color.grey40;
color.lightFocusBorder = color.grey15;
color.inputHighlight = color.grey10;

// Success/Failure/Warning/Normal levels
color.successMain = color.green90;
color.successAccent = color.green;
color.successBackground = color.green5;
color.errorMain = color.red80;
color.errorAccent = color.red;
color.errorAccentLight = color.red50;
color.errorBackground = color.red10;
color.warningMain = color.yellow90;
color.warningAccent = color.orange;
color.warningBackground = color.orange10;
color.normalLevelMain = color.blue90;
color.normalLevelAccent = color.blue;
color.normalLevelBackground = addTransparency(color.blue, 0.4);

// Button colors
color.radioButtonDisabledHovered = color.greyHovered;
color.radioButtonDisabledPressed = color.greyPressed;
color.disabledButtonColor = color.grey20;

// Step progress bar (several other colors for bar based on primary color)
color.stepProgressBarHighlight = color.purple10;
color.stepProgressBarIncomplete = color.grey30;

// Toggle
color.activatingToggle = color.dullPurple;
color.activatingToggleBackground = color.dullGreen;

// Objects (these aren't colors, these are objects of colors)
color.destructiveButton = color.redButton;
color.normalButton = color.blueButton;
color.successAlert = color.greenAlert;
color.warningAlert = color.orangeAlert;
color.clickableRow = color.clickableRowBlue;
color.textLink = color.greyLink;

color.onBoardingGradient = {
  start: color.lighterOnboardingPurple,
  middle: color.mediumOnboardingPurple,
  end: color.darkerOnboardingPurple,
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
  buttonDisabled: color.disabledButtonColor,
  focusOutline: color.transparent,
  focusBorder: color.darkFocusBorder,
};

color.card = {
  defaultColor: color.white,
  buttonHovered: color.greyHovered,
  buttonPressed: color.greyPressed,
  buttonDisabled: color.bgColorTransparent,
  focusOutline: color.bgColorTransparent,
  focusBorder: color.darkFocusBorder,
};

/**
 * Part 4: Derived theme colors
 * Color objects that dervive from colors set in part 3.
 *
 * The named colors in this section and section 3 SHOULD be used in code -
 * and should be the ONLY colors used in code. (One exception: color.transparent
 * can be used in code.)
 */

color.stepProgressBarDelegate = {
  defaultColor: color.primaryBrighter,
  buttonHovered: color.primaryBrighter,
  buttonPressed: color.primaryPressed,
  buttonDisabled: color.stepProgressBarIncomplete,
  focusOutline: color.bgColorTransparent,
  focusBorder: color.bgColorTransparent,
};

color.invertedButton = {
  defaultColor: color.fontColorInverted,
  buttonHovered: color.radioButtonDisabledHovered,
  buttonPressed: color.radioButtonDisabledPressed,
  focusOutline: color.radioButtonDisabledHovered,
  focusBorder: color.radioButtonDisabledPressed,
};

color.linkButton = {
  defaultColor: color.bgColorTransparent,
  buttonHovered: color.bgColorTransparent,
  buttonPressed: color.bgColorTransparent,
  buttonDisabled: color.bgColorTransparent,
  focusOutline: color.bgColorTransparent,
  focusBorder: color.bgColorTransparent,
};

color.iconButtonLightBackground = {
  defaultColor: color.bgColorTransparent,
  buttonHovered: color.radioButtonDisabledHovered,
  buttonPressed: color.radioButtonDisabledPressed,
  buttonDisabled: color.transparent,
  focusOutline: color.bgColorTransparent,
  focusBorder: color.darkFocusBorder,
};

color.iconButtonDarkBackground = {
  defaultColor: addTransparency(color.primary, 0.0),
  buttonHovered: color.primaryHovered,
  buttonPressed: color.primaryPressed,
  buttonDisabled: addTransparency(color.primary, 0.0),
  focusOutline: color.transparent,
  focusBorder: color.lightFocusBorder,
};

color.inputState = {
  default: {
    border: color.darkFocusBorder,
    placeholder: color.fontColor,
    text: color.fontColorDark,
  },
  hover: {
    border: color.darkFocusBorder,
    placeholder: color.fontColorDark,
    text: color.fontColorDark,
  },
  focus: {
    border: color.normalButton.defaultColor,
    highlight: addTransparency(color.normalButton.defaultColor, 0.4),
    placeholder: color.fontColor,
    text: color.fontColorDark,
  },
  error: {
    border: color.errorAccentLight,
    highlight: color.errorBackground,
    placeholder: color.fontColor,
    text: color.fontColorDark,
  },
  disabled: {
    border: color.disabledButtonColor,
    placeholder: color.disabledButtonColor,
    text: color.disabledButtonColor,
  },
};

color;
