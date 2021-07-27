/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/**
 * Primary palette
 */
// Purple
const purple90 = '#321C64';
const purple80 = '#45278D';
const purple70 = '#592ACB';
const purple60 = '#7542E5';
const purple50 = '#9059FF';
const purple40 = '#AB71FF';
const purple30 = '#C689FF';
const purple20 = '#CB9EFF';
const purple10 = '#D9BFFF';
const purple5 = '#E7DFFF';

// Green
const green90 = '#00736C';
const green80 = '#00A49A';
const green70 = '#1CC4A0';
const green60 = '#3AD4B3';
const green50 = '#3FE1B0';
const green40 = '#54FFBD';
const green30 = '#88FFD1';
const green20 = '#B3FFE3';
const green10 = '#D1FFEE';
const green5 = '#E3FFF3';

// Grey
const black = '#000000';
const grey60 = '#0C0C0D';
const grey50 = '#3D3D3D';
const grey40 = '#6D6D6E';
const grey30 = '#9E9E9E';
const grey20 = '#CECECF';
const grey10 = '#E7E7E7';
const grey5 = '#F9F9FA';
const white = '#FFFFFF';

/**
 * Secondary palette
 */
// Violet
const violet90 = '#2B1141';
const violet80 = '#4E1A69';
const violet70 = '#722291';
const violet60 = '#952BB9';
const violet50 = '#B833E1';
const violet40 = '#D74CF0';
const violet30 = '#F770FF';
const violet20 = '#F68FFF';
const violet10 = '#F6B8FF';
const violet5 = '#F7E2FF';

// Blue
const blue90 = '#09204D';
const blue80 = '#073072';
const blue70 = '#054096';
const blue60 = '#0250BB';
const blue50 = '#0060DF';
const blue40 = '#0090ED';
const blue30 = '#00B3F4';
const blue20 = '#00DDFF';
const blue10 = '#80EBFF';
const blue5 = '#AAF2FF';

// Orange
const orange90 = '#7C1504';
const orange80 = '#9E280B';
const orange70 = '#CC3D00';
const orange60 = '#E25920';
const orange50 = '#FF7139';
const orange40 = '#FF8A50';
const orange30 = '#FFA266';
const orange20 = '#FFB587';
const orange10 = '#FFD5B2';
const orange5 = '#FFF4DE';

// Yellow
const yellow90 = '#960E18';
const yellow80 = '#A7341F';
const yellow70 = '#C45A27';
const yellow60 = '#E27F2E';
const yellow50 = '#FFA436';
const yellow40 = '#FFBD4F';
const yellow30 = '#FFD567';
const yellow20 = '#FFEA80';
const yellow10 = '#FFFF98';
const yellow5 = '#FFFFCC';

// Red
const red90 = '#440306';
const red80 = '#810220';
const red70 = '#C50042';
const red60 = '#E22850';
const red50 = '#FF4F5E';
const red40 = '#FF6A75';
const red30 = '#FF848B';
const red20 = '#FF9AA2';
const red10 = '#FFBDC5';
const red5 = '#FFDFE7';

// Pink
const pink90 = '#50134B';
const pink80 = '#7F145B';
const pink70 = '#C60084';
const pink60 = '#E31587';
const pink50 = '#FF298A';
const pink40 = '#FF4AA2';
const pink30 = '#FF6BBA';
const pink20 = '#FF8AC5';
const pink10 = '#FFB4DB';
const pink5 = '#FFDEF0';

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
const primary = purple90;
const secondary = green50;
const dark = grey60;
const light = grey5;

/**
 * Functional
 */
const blue = blue50;
const blueActive = blue70;
const blueHover = blue60;
const blueFocus = addTransparency(blue, 0.4);

const green = green50;
const greenActive = green70;
const greenHover = green60;
const greenFocus = addTransparency(green, 0.2);

const red = red50;
const redActive = red70;
const redHover = red60;
const redFocus = addTransparency(red, 0.1);

const yellow = yellow50;
const yellowActive = yellow70;
const yellowHover = yellow60;
const yellowFocus = addTransparency(yellow, 0.1);

const informational = {
  default: blue,
  active: blueActive,
  hover: blueHover,
  focus: blueFocus,
};

const success = {
  default: green,
  active: greenActive,
  hover: greenHover,
  focus: greenFocus,
};

const error = {
  default: red,
  active: redActive,
  hover: redHover,
  focus: redFocus,
};

const warning = {
  default: yellow,
  active: yellowActive,
  hover: yellowHover,
  focus: yellowFocus,
};

/**
 * Gradients
 */
const customGradientColorPurple = '#9D62FC';
const customGradientColorPink = '#FD3296';
const customGradientColorBlue = '#5B6DF8';

// Pink gradient
const gradientPink = {
  type: 'radial',
  stops: [
    {
      position: 0.0,
      color: customGradientColorPurple,
    },
    {
      position: 0.0,
      color: customGradientColorPink,
    },
  ],
};

// Blue gradient
const gradientBlue = {
  type: 'radial',
  stops: [
    {
      position: 0.0,
      color: violet50,
    },
    {
      position: 0.371,
      color: purple50,
    },
    {
      position: 0.614,
      color: customGradientColorBlue,
    },
    {
      position: 1.0,
      color: blue40,
    },
  ],
};

// Orange gradient
const gradientOrange = {
  type: 'radial',
  stops: [
    {
      position: 0.0,
      color: red60,
    },
    {
      position: 1.0,
      color: yellow50,
    },
  ],
};

// Green gradient
const gradientGreen = {
  type: 'radial',
  stops: [
    {
      position: 0.0,
      color: blue50,
    },
    {
      position: 1.0,
      color: green50,
    },
  ],
};
