/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// This is to determine whether to show the dark mode version of images or not.
color.useDarkAssets = true;

/**
 * Part 2: Theme colors - High Contrast Dark
 * Meets WCAG 2.1 AAA contrast requirements with high-contrast dark palette.
 */
// Primary
color.primary = color.black;
color.primaryBrighter = color.white;
color.primaryHovered = color.grey55;
color.primaryPressed = color.black;

// Backgrounds
color.bgColor = color.black;
color.bgColorStronger = color.black;
color.overlay = addTransparency(color.black, 0.8);

// Fonts
color.fontColor = color.white;
color.fontColorMuted = color.grey15;
color.fontColorDark = color.white;
color.fontColorDarkMuted = color.grey20;
color.fontColorWarningForConnectionBox = color.yellow20;
color.fontColorWarningForBackground = color.yellow20;
color.fontColorInverted = color.white;
color.fontColorInvertedMuted = color.grey15;
color.fontTitleColor = color.white;
color.normalButtonFont = color.black;
color.destructiveButtonFont = color.white;
color.macosPermissionTitleFont = color.white;

// Dividers and borders
color.cardBorder = color.white;
color.divider = color.white;
color.dropShadow = color.black;
color.scrollBar = color.white;
color.focusBorder = color.white;
color.inputHighlight = color.black;

// Buttons are built off these base colors
color.normalButtonDefault = color.white;
color.destructiveButtonDefault = color.red60;

// Success/Failure/Warning/Normal levels
color.normalLevelAccent = color.white;
color.successAccent = color.green40;
color.errorAccent = color.red40;
color.errorAccentLight = color.red30;
color.warningAccent = color.yellow20;

// Badges
color.normalLevelText = color.black;
color.normalLevelBackground = color.white;
color.successText = color.black;
color.successBackground = color.green40;
color.errorText = color.white;
color.errorBackground = color.red70;
color.warningText = color.black;
color.warningBackground = color.yellow20;

// Disabled button colors
color.disabledButtonColor = color.grey45;
color.disabledButtonHovered = color.grey40;
color.disabledButtonPressed = color.grey50;

// Step progress bar
color.stepProgressBarHighlight = color.white;
color.stepProgressBarIncomplete = color.grey50;
color.stepProgressBarComplete = color.white;
color.stepProgressBarCompleteText = color.white;
color.stepProgressIconIncomplete = color.white;
color.stepProgressIconComplete = color.black;

// Toggle
color.vpnToggleConnected = {
  defaultColor: color.black,
  buttonHovered: color.grey55,
  buttonPressed: color.black,
  focusOutline: addTransparency(color.white, 0.4),
  focusBorder: color.white,
};

color.vpnToggleDisconnected = {
  defaultColor: color.black,
  buttonHovered: color.grey55,
  buttonPressed: color.black,
  buttonDisabled: color.disabledButtonColor,
  focusOutline: addTransparency(color.white, 0.4),
  focusBorder: color.white,
};

color.vpnToggleDisconnectedBorder = color.white;
color.vpnToggleConnectedBorder = color.white;

color.connectedToggle = color.white;
color.disconnectedToggle = color.white;
color.connectingToggle = color.yellow20;
color.connectingToggleBackground = color.purple80;

// Objects (these aren't colors, these are objects of colors)
color.destructiveButton = {
  defaultColor: color.destructiveButtonDefault,
  buttonHovered: color.red50,
  buttonPressed: color.red70,
  buttonDisabled: color.grey50,
  focusOutline: addTransparency(color.destructiveButtonDefault, 0.4),
  focusBorder: color.white,
  fontColor: color.destructiveButtonFont,
};

color.successAlert = {
  defaultColor: color.green40,
  buttonHovered: color.white,
  buttonPressed: color.green30,
  focusOutline: addTransparency(color.green40, 0.4),
  focusBorder: color.white,
};

color.warningAlert = {
  buttonHovered: color.white,
  buttonPressed: color.yellow10,
  focusOutline: addTransparency(color.yellow20, 0.4),
  focusBorder: color.white,
};

color.textLink = {
  defaultColor: color.blue20,
  buttonHovered: color.blue10,
  buttonPressed: color.white,
  buttonDisabled: color.grey40,
};

color.onBoardingGradient = {
  start: color.black,
  middle: color.purple90,
  end: color.black,
};
