/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const color = {};

/**
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

// Dull colors
color.dullPurple = '#387E8A';
color.dullGreen = '#998DB2';

color.transparent = '#00000000';

/**
 * Helper functions
 */
const percentToHex = percent => {
  const int8Bit = Math.round(percent * 255);
  const hexAlphaValue = int8Bit.toString(16).padStart(2, '0');

  return hexAlphaValue.toUpperCase();
};

const addTransparency = (hexColor, percent) => {
  const hexValue = hexColor.substring(1);
  const hexAlphaValue = percentToHex(percent);

  return `#${hexAlphaValue}${hexValue}`;
};

// TODO: BELOW HERE SHOULD NOT HAVE ANY HEX CODES
// Derived

color.bgColor = color.grey5;
color.bgColor30 = '#4DF9F9FA';
color.bgColor80 = '#CCF9F9FA';
color.bgColorTransparent = '#00F9F9FA';

color.blue = color.blue50;
color.blueHovered = color.blue60;
color.bluePressed = color.blue70;
color.blueDisabled = '#a3c0f3';
color.blueFocusOutline = '#4d0a84ff';
color.blueFocusBorder = '#0a84ff';

color.divider = '#0C0C0D0A'
color.green = color.green50;
color.grey = '#CACACA';
color.greyHovered = '#E6E6E6';
color.greyPressed = '#C2C2C2';
color.greyDisabled = '#D8D8D8';
color.ink = color.purple90;
color.orange = color.yellow50;
color.orangeHovered = color.yellow60;
color.orangeFocus = '#4DE27F2E';
color.orangePressed = color.yellow70;
color.red = color.red60;
color.redHovered = color.red70;
color.redPressed = color.red80;
color.redDisabled = color.red10;
color.redfocusOutline = '#66C50042';
color.redBadgeText = color.red80;
color.greenBadgeText = color.green90;
color.greenBadgeBackground = color.green5;
color.orangeBadgeText = color.yellow90;
color.orangeBadgeBackground = color.orange10;
color.blueBadgeText = color.blue90;
color.blueBadgeBackground = '#660060DF'

color.overlayBackground = '#4D000000';

color.checkBoxWarning = color.yellow70;

color.fontColor = color.grey40;
color.fontColorDark = color.grey50;

color.darkFocusBorder = color.fontColor;
color.lightFocusBorder = '#d5d3e0';

/**
 * Main palette
 */
color.primary = color.purple90;
color.secondary = color.green50;
color.dark = color.grey60;
color.light = color.grey5;

/**
 * Functional
 */
color.blueActive = color.blue70;
color.blueHover = color.blue60;
color.blueFocus = addTransparency(color.blue, 0.4);

color.greenActive = color.green70;
color.greenHover = color.green60;
color.greenFocus = color.green20;

color.redActive = color.red70;
color.redHover = color.red60;
color.redFocus = color.red10;

color.yellow = color.yellow50;
color.yellowActive = color.yellow70;
color.yellowHover = color.yellow60;
color.yellowFocus = color.orange10;

color.informational = {
  default: color.blue,
  active: color.blueActive,
  hover: color.blueHover,
  focus: color.blueFocus,
};

color.success = {
  default: color.green,
  active: color.greenActive,
  hover: color.greenHover,
  focus: color.greenFocus,
};

color.error = {
  default: color.red50,
  active: color.redActive,
  hover: color.redHover,
  focus: color.redFocus,
};

color.warning = {
  default: color.yellow,
  active: color.yellowActive,
  hover: color.yellowHover,
  focus: color.yellowFocus,
};

/**
 * Gradients
 */
color.customGradientColorPurple = '#9D62FC';
color.customGradientColorPink = '#FD3296';
color.customGradientColorBlue = '#5B6DF8';

// Pink gradient
color.gradientPink = {
  type: 'radial',
  stops: [
    {
      position: 0.0,
      color: color.customGradientColorPurple,
    },
    {
      position: 0.0,
      color: color.customGradientColorPink,
    },
  ],
};

// Blue gradient
color.gradientBlue = {
  type: 'radial',
  stops: [
    {
      position: 0.0,
      color: color.violet50,
    },
    {
      position: 0.371,
      color: color.purple50,
    },
    {
      position: 0.614,
      color: color.customGradientColorBlue,
    },
    {
      position: 1.0,
      color: color.blue40,
    },
  ],
};

// Orange gradient
color.gradientOrange = {
  type: 'radial',
  stops: [
    {
      position: 0.0,
      color: color.red60,
    },
    {
      position: 1.0,
      color: color.yellow50,
    },
  ],
};

// Green gradient
color.gradientGreen = {
  type: 'radial',
  stops: [
    {
      position: 0.0,
      color: color.blue50,
    },
    {
      position: 1.0,
      color: color.green50,
    },
  ],
};

/**
 * Inputs
 */
color.inputState = {
  default: {
    background: color.white,
    border: color.grey30,
    placeholder: color.grey40,
    text: color.grey50,
  },
  hover: {
    background: color.white,
    border: color.grey40,
    placeholder: color.grey50,
    text: color.grey50,
  },
  focus: {
    background: color.white,
    border: color.informational.default,
    highlight: color.informational.focus,
    placeholder: color.grey40,
    text: color.grey50,
  },
  error: {
    background: color.white,
    border: color.error.default,
    highlight: color.error.focus,
    placeholder: color.grey40,
    text: color.grey50,
  },
  disabled: {
    background: color.white,
    border: color.grey20,
    placeholder: color.grey20,
    text: color.grey20,
  },
};

color.onBoardingGradient = {
  start: '#472C87',
  middle: '#301962',
  end: '#1D0942',
};

color.blueButton = {
  defaultColor: color.blue,
  buttonHovered: color.blueHovered,
  buttonPressed: color.bluePressed,
  buttonDisabled: color.blueDisabled,
  focusBgColor: color.blue,
  focusOutline: color.blueFocusOutline,
  focusBorder: color.blueFocusBorder,
};

color.wasmOptionBtn = {
  defaultColor: '#00eeeeee',
  buttonHovered: '#330a84ff',
  buttonPressed: '#4d0a84ff',
  buttonDisabled: color.blueDisabled,
  focusBgColor: color.blue,
  focusOutline: color.blueFocusOutline,
  focusBorder: color.blueFocusBorder,
};

color.warningAlertFocusBorders = {
  focusOutline: color.orangeFocus,
  focusBorder: color.orangePressed,
};

color.clickableRowBlue = {
  defaultColor: color.bgColor,
  buttonHovered: '#D4E2F6',
  buttonPressed: '#AECBF2',
  focusOutline: color.bgColorTransparent,
  focusBorder: color.blueFocusBorder,
};

color.clickableRowPurple = {
  defaultColor: color.bgColorTransparent,
  buttonHovered: color.purple5,
  buttonPressed: color.purple10,
  focusOutline: color.bgColorTransparent,
  focusBorder: color.purple70,
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
  defaultColor: '#00321C64',
  buttonHovered: '#5b4983',
  buttonPressed: '#8477a2',
  buttonDisabled: '#00321C64',
  focusOutline: '#005b4983',
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

color.whiteButton = {
  defaultColor: color.white,
  buttonHovered: color.greyHovered,
  buttonPressed: color.greyPressed,
  focusOutline: color.greyHovered,
  focusBorder: color.greyPressed,
};

color.greyButton = {
  defaultColor: color.grey20,
  buttonHovered: color.greyHovered,
  buttonPressed: color.greyPressed,
  buttonDisabled: color.greyDisabled,
  focusOutline: color.greyHovered,
  focusBorder: color.greyPressed,
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
  focusOutline: color.redfocusOutline,
  focusBorder: color.redPressed,
};

color.removeDeviceBtn = {
  defaultColor: color.bgColorTransparent,
  buttonHovered: color.red5,
  buttonPressed: color.red10,
  buttonDisabled: color.bgColorTransparent,
  focusOutline: color.bgColorTransparent,
  focusBorder: color.red,
};

color.vpnToggleConnected = {
  defaultColor: color.green,
  buttonHovered: color.green60,
  buttonPressed: color.green70,
  focusOutline: color.bgColor30,
  focusBorder: color.lightFocusBorder,
};

color.vpnToggleDisconnected = {
  defaultColor: color.grey30,
  buttonHovered: color.fontColor,
  buttonPressed: color.fontColorDark,
  buttonDisabled: color.grey,
  focusOutline: 'transparent',
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

color.greenAlert = {
  defaultColor: color.green50,
  buttonHovered: color.green60,
  buttonPressed: color.green70,
  focusOutline: '#333FE1B0',
  focusBorder: color.green70,
};

color.greyLink = {
  defaultColor: '#B30C0C0D',
  buttonHovered: '#CC0C0C0D',
  buttonPressed: '#FF0C0C0D',
  focusOutline: '#FF0C0C0D',
  focusBorder: color.black
};

color.input = {
  backgroundColor: color.white,
  borderColor: color.grey30,
  highlight: color.grey10,
  defaultColor: color.white,
  buttonHovered: color.blueHovered,
  buttonPressed: color.bluePressed,
  buttonDisabled: color.blueDisabled,
  focusBgColor: color.blue,
  focusOutline: color.blueFocusOutline,
  focusBorder: color.blueFocusBorder,
};

color.greenBadge = {
  textColor: color.greenBadgeText,
  backgroundColor: color.greenBadgeBackground
};

color.redBadge = {
  textColor: color.redBadgeText,
  backgroundColor: color.redDisabled
};

color.orangeBadge = {
  textColor: color.orangeBadgeText,
  backgroundColor: color.orangeBadgeBackground
};

color.blueBadge = {
  textColor: color.blueBadgeText,
  backgroundColor: color.blueBadgeBackground
};

color.purpleStepProgressBarDelegate = {
  defaultColor: color.purple70,
  buttonHovered: color.purple70,
  buttonPressed: color.purple80,
  buttonDisabled: color.grey30,
  focusOutline: color.bgColorTransparent,
  focusBorder: color.bgColorTransparent,
};

color;
