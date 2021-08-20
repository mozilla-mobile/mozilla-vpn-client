/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

.pragma library

const bgColor = "#F9F9FA";
const bgColor30 = "#4DF9F9FA";
const bgColor80 = "#CCF9F9FA";
const bgColorTransparent = "#00F9F9FA";

const blue = "#0060DF";
const blueHovered = "#0250BB";
const bluePressed = "#054096";
const blueDisabled = "#a3c0f3";
const blueFocusOutline = "#4d0a84ff";
const blueFocusBorder = "#0a84ff";

const divider = "#0C0C0D0A"
const green = "#3FE1B0";
const grey = "#CACACA";
const greyHovered = "#E6E6E6";
const greyPressed = "#C2C2C2";
const ink = "#321C64";
const orange = "#FFA436";
const purple60 = "#7542E5";
const red = "#FF4F5E";
const redHovered = "#E22850";
const redPressed = "#C50042";
const redfocusOutline = "#66C50042";
const white = "#FFFFFF"

const checkBoxWarning = "#C45A27";

const errorFocusOutline = "#FFBDC5";

const fontColor = "#6D6D6E";
const fontColorDark = "#3D3D3D";
const fontFamily = "Metropolis";
const fontBoldFamily = "MetropolisSemiBold";
const fontInterFamily = "InterUI";
const fontSize = 15;
const fontSizeLarge = 22;
const fontSizeSmall = 13;
const fontSizeSmallest = 11;
const fontWeightBold = 600;
const iconSize = 16;
const labelLineHeight = 22;
const cityListTopMargin = 18;
const controllerInterLineHeight = 18;
const hSpacing = 20;
const vSpacing = 24;
const vSpacingSmall = 16;
const listSpacing = 8;
const maxTextWidth = 296;
const windowMargin = 16;
const popupMargin = 24;
const desktopAppHeight = 520;
const desktopAppWidth = 360;

const darkFocusBorder = fontColor;
const lightFocusBorder = "#d5d3e0";

const blueButton = {
    "defaultColor" : blue,
    "buttonHovered": blueHovered,
    "buttonPressed": bluePressed,
    "buttonDisabled": blueDisabled,
    "focusBgColor": blue,
    "focusOutline": blueFocusOutline,
    "focusBorder": blueFocusBorder,
};

const wasmOptionBtn = {
    "defaultColor" : "#00eeeeee",
    "buttonHovered": "#330a84ff",
    "buttonPressed": "#4d0a84ff",
    "buttonDisabled": blueDisabled,
    "focusBgColor": blue,
    "focusOutline": blueFocusOutline,
    "focusBorder": blueFocusBorder,
};

const clickableRowBlue = {
    "defaultColor": bgColor,
    "buttonHovered": "#D4E2F6",
    "buttonPressed": "#AECBF2",
    "focusOutline": bgColorTransparent,
    "focusBorder": blueFocusBorder,
};

const clickableRowPurple = {
  'defaultColor': bgColorTransparent,
  'buttonHovered': '#E7DFFF',
  'buttonPressed': '#D9BFFF',
  'focusOutline': bgColorTransparent,
  'focusBorder': '#592ACB',
};

const iconButtonLightBackground = {
    "defaultColor": bgColorTransparent,
    "buttonHovered": greyHovered,
    "buttonPressed": greyPressed,
    "focusOutline": bgColorTransparent,
    "focusBorder": darkFocusBorder,
};

const iconButtonDarkBackground = {
    "defaultColor": "#00321C64",
    "buttonHovered": "#5b4983",
    "buttonPressed": "#8477a2",
    "focusOutline": "#005b4983",
    "focusBorder": lightFocusBorder,
};

const linkButton = {
    "defaultColor" : bgColorTransparent,
    "buttonHovered": bgColorTransparent,
    "buttonPressed": bgColorTransparent,
    "focusOutline": bgColorTransparent,
    "focusBorder": blueFocusBorder,
};

const greyButton = {
    "defaultColor": "#CECECF",
    "buttonHovered": greyHovered,
    "buttonPressed": greyPressed,
    "focusOutline": greyHovered,
    "focusBorder": greyPressed,
};

const redButton = {
    "defaultColor" : red,
    "buttonHovered": redHovered,
    "buttonPressed": redPressed,
    "focusOutline": redfocusOutline,
    "focusBorder": redPressed,
};

const removeDeviceBtn = {
    "defaultColor": bgColorTransparent,
    "buttonHovered": "#FFDFE7",
    "buttonPressed": "#FFBDC5",
    "focusOutline": bgColorTransparent,
    "focusBorder": red,
};

const vpnToggleConnected = {
    "defaultColor":  "#3FE1B0",
    "buttonHovered": "#3AD4B3",
    "buttonPressed": "#1CC5A0",
    "focusOutline": bgColor30,
    "focusBorder": lightFocusBorder,
};

const vpnToggleDisconnected = {
    "defaultColor": "#9E9E9E",
    "buttonHovered": fontColor,
    "buttonPressed": fontColorDark,
    "focusOutline": "transparent",
    "focusBorder": darkFocusBorder,
};

const greenAlert = {
    "defaultColor":  "#3FE1B0",
    "buttonHovered": "#3AD4B3",
    "buttonPressed": "#1CC5A0",
    "focusOutline": "#333FE1B0",
    "focusBorder": "#1CC5A0",
};

const cornerRadius = 4;
const focusBorderWidth = 2;

// In milliseconds, the animation of a single device removal
const removeDeviceAnimation = 300;
const rowHeight = 40;
const settingsMaxContentHeight = 740;
const maxHorizontalContentWidth = 460;
const contentTopMarginDesktop = 20;
const contentTopMarginMobile = 48;

const uiState = {
    "stateDefault": "state-default",
    "stateHovered": "state-hovered",
    "statePressed": "state-pressed",
    "stateFocused": "state-focused",
    "stateLoading": "state-loading",
};

const greyLink = {
    "defaultColor": "#B30C0C0D",
    "buttonHovered": "#CC0C0C0D",
    "buttonPressed": "#FF0C0C0D",
    "focusOutline": "#FF0C0C0D",
    "focusBorder": "#000000"
};

const input = {
    "backgroundColor": "#ffffff",
    "borderColor": "#9E9E9E",
    "highlight": "#E7E7E7",
    "defaultColor" : "#ffffff",
    "buttonHovered": blueHovered,
    "buttonPressed": bluePressed,
    "buttonDisabled": blueDisabled,
    "focusBgColor": blue,
    "focusOutline": blueFocusOutline,
    "focusBorder": blueFocusBorder,
};
