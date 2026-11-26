/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// This is to determine whether to show the dark mode version of images or not.
color.useDarkAssets = false;

/**
 * Part 2: Theme colors - High Contrast Light
 * Meets WCAG 2.1 AAA contrast requirements with high-contrast light palette.
 */
// Primary
color.primary = color.black;
color.primaryBrighter = color.grey60;
color.primaryHovered = color.grey45;
color.primaryPressed = color.black;

// Backgrounds
color.bgColor = color.white;
color.bgColorStronger = color.grey3;
color.overlay = addTransparency(color.black, 0.6);

// Fonts
color.fontColor = color.black;
color.fontColorMuted = color.grey55;
color.fontColorDark = color.black;
color.fontColorDarkMuted = color.grey60;
color.fontColorWarningForConnectionBox = color.yellow90;
color.fontColorWarningForBackground = color.yellow90;
color.fontColorInverted = color.white;
color.fontColorInvertedMuted = color.grey15;
color.fontTitleColor = color.black;
color.normalButtonFont = color.white;
color.destructiveButtonFont = color.white;
color.macosPermissionTitleFont = color.black;

// Dividers and borders
color.cardBorder = color.black;
color.divider = color.black;
color.dropShadow = color.black;
color.scrollBar = color.black;
color.focusBorder = color.black;
color.inputHighlight = color.black;

// Buttons are built off these base colors
color.normalButtonDefault = color.black;
color.destructiveButtonDefault = color.red80;

// Success/Failure/Warning/Normal levels
color.normalLevelAccent = color.black;
color.successAccent = color.black;
color.errorAccent = color.red80;
color.errorAccentLight = color.red70;
color.warningAccent = color.yellow90;

// Badges
color.normalLevelText = color.white;
color.normalLevelBackground = color.black;
color.successText = color.white;
color.successBackground = color.black;
color.errorText = color.white;
color.errorBackground = color.red80;
color.warningText = color.black;
color.warningBackground = color.yellow20;

// Disabled button colors
color.disabledButtonColor = color.grey30;
color.disabledButtonHovered = color.grey20;
color.disabledButtonPressed = color.grey40;

// Step progress bar
color.stepProgressBarHighlight = color.black;
color.stepProgressBarIncomplete = color.grey40;
color.stepProgressBarComplete = color.black;
color.stepProgressBarCompleteText = color.black;
color.stepProgressIconIncomplete = color.white;
color.stepProgressIconComplete = color.white;

// Toggle
color.vpnToggleConnected = {
  defaultColor: color.black,
  buttonHovered: color.grey60,
  buttonPressed: color.black,
  focusOutline: addTransparency(color.black, 0.4),
  focusBorder: color.white,
};

color.vpnToggleDisconnected = {
  defaultColor: color.black,
  buttonHovered: color.grey60,
  buttonPressed: color.black,
  buttonDisabled: color.disabledButtonColor,
  focusOutline: addTransparency(color.black, 0.4),
  focusBorder: color.black,
};

color.vpnToggleDisconnectedBorder = color.transparent;
color.vpnToggleConnectedBorder = color.white;

color.connectedToggle = color.white;
color.disconnectedToggle = color.white;
color.connectingToggle = color.black;
color.connectingToggleBackground = color.grey20;

// Objects (these aren't colors, these are objects of colors)
color.destructiveButton = {
  defaultColor: color.destructiveButtonDefault,
  buttonHovered: color.red90,
  buttonPressed: color.black,
  buttonDisabled: color.grey20,
  focusOutline: addTransparency(color.destructiveButtonDefault, 0.4),
  focusBorder: color.black,
  fontColor: color.destructiveButtonFont,
};

color.successAlert = {
  defaultColor: color.black,
  buttonHovered: color.grey60,
  buttonPressed: color.black,
  focusOutline: addTransparency(color.black, 0.4),
  focusBorder: color.black,
};

color.warningAlert = {
  buttonHovered: color.black,
  buttonPressed: color.black,
  focusOutline: addTransparency(color.yellow90, 0.4),
  focusBorder: color.black,
};

color.textLink = {
  defaultColor: color.black,
  buttonHovered: color.grey60,
  buttonPressed: color.black,
  buttonDisabled: color.grey40,
};

color.onBoardingGradient = {
  start: color.black,
  middle: color.grey60,
  end: color.black,
};
