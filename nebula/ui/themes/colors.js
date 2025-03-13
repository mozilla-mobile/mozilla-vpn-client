/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// 3 files get combined into one JS color object - As this is the first file,
// this next line is needed to create the object.
const color = {};

/**
 * Part 0: Helper functions
 */
const percentToHex = percent => {
  const int8Bit = Math.round(percent * 255);
  const hexAlphaValue = int8Bit.toString(16).padStart(2, '0');

  return hexAlphaValue.toUpperCase();
};

const hexToPercent = hex => {
  const hexString = `0x${hex}`;
  const asNumber = Number(hexString);
  return asNumber / 255;
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

// Neither color can include transparency
const mixColors = (hexColor1, hexColor2, percentForSecond) => {
  var returnString = '#';
  for (var index = 1; index < 7; index = index + 2) {
    const hexColorSnippet1 = hexColor1.substring(index, index + 2);
    const hexColorSnippet2 = hexColor2.substring(index, index + 2);
    const percentColor1 = hexToPercent(hexColorSnippet1);
    const percentColor2 = hexToPercent(hexColorSnippet2);
    const smallerColor = Math.min(percentColor1, percentColor2);
    const largerColor = Math.max(percentColor1, percentColor2);

    // take difference between colors
    const colorDifference = largerColor - smallerColor;

    // get % for the larger
    const percentForLarger =
        (percentColor2 == largerColor ? percentForSecond :
                                        1 - percentForSecond);

    // multiply % by colorDifference, add it to the smaller
    const finalPercent = smallerColor + (colorDifference * percentForLarger);

    // add it to return string
    const finalAsString = percentToHex(finalPercent);

    returnString = returnString.concat(finalAsString);
  }
  return returnString;
};

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
color.grey45 = '#42414D';
color.grey40 = '#6D6D6E';
color.grey30 = '#9E9E9E';
color.grey20 = '#CECECF';
color.grey15 = '#D8D8D8';
color.grey10 = '#E7E7E7';
color.grey5 = '#F9F9FA';
color.grey3 = '#FBFBFE';
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
color.brightSlate = '#2B2A33';

color.lighterOnboardingPurple = '#472C87';
color.mediumOnboardingPurple = '#301962';
color.darkerOnboardingPurple = '#1D0942';

color.transparent = '#00000000';

/**
 * Part 2: Color objects
 * Uses the colors defined above.
 * Should NOT be used directly in code, and colors should not be modified.
 * (Can add to this section if really, truly needed.)
 */

color.clickableRowBlue = {
  defaultColor: color.grey5,
  buttonHovered: color.washedBlue,
  buttonPressed: color.washedLightBlue,
  focusOutline: addTransparency(color.grey5, 0.0),
  focusBorder: color.strongBlue,
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
  defaultColor: color.green50,
  buttonHovered: color.green60,
  buttonPressed: color.green70,
  focusOutline: addTransparency(color.green50, 0.2),
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
};
