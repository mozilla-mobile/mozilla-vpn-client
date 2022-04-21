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
color.blue = color.blue50;
color.blueActive = color.blue70;
color.blueHover = color.blue60;
color.blueFocus = addTransparency(color.blue, 0.4);

color.green = color.green50;
color.greenActive = color.green70;
color.greenHover = color.green60;
color.greenFocus = color.green20;

color.red = color.red50;
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
  default: color.red,
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
color.input = {
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

color;
