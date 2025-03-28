/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// This is to determine whether to show the dark mode version of images or not.
color.useDarkAssets = false;

/**
 * Part 2: Theme colors
 * Named colors in this section should only be set to colors in part 1 or 2.
 * This file should be duplicated for additional themes, and is the only file
 * that needs to be duplicated.
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
color.bgColorStronger = color.white;
color.overlay = addTransparency(color.black, 0.3);

// Fonts
color.fontColor = color.grey40;
color.fontColorDark = color.grey50;
color.fontColorWarningForConnectionBox = color.yellow50;
color.fontColorWarningForBackground = color.yellow70;
color.fontColorInverted = color.white;
color.fontColorInvertedMuted = color.grey20;
color.fontTitleColor = color.black;
color.normalButtonFont = color.white;
color.destructiveButtonFont = color.white;

// Dividers and borders
color.divider = color.grey10;
color.dropShadow = color.grey60;
color.scrollBar = color.grey40;
color.focusBorder = color.grey40;  // used for keyboard navigation highlighting
                                   // for settings toggles and swipe actions
color.inputHighlight = color.grey10;

// Buttons are built off these base colors
color.normalButtonDefault = color.blue50;
color.destructiveButtonDefault = color.red70;

// Success/Failure/Warning/Normal levels
color.normalLevelAccent = color.blue50;
color.successAccent = color.green50;
color.errorAccent = color.red60;
color.errorAccentLight = color.red50;
color.warningAccent = color.yellow50;

// Badges
color.normalLevelText = color.blue90;
color.normalLevelBackground = addTransparency(color.blue50, 0.4);
color.successText = color.green90;
color.successBackground = color.green5;
color.errorText = color.red80;
color.errorBackground = color.red10;
color.warningText = color.yellow90;
color.warningBackground = color.orange10;

// Disabled button colors
color.disabledButtonColor = color.grey20;
color.disabledButtonHovered = color.grey10;
color.disabledButtonPressed = color.grey20;

// Step progress bar (several other colors for bar based on primary color)
color.stepProgressBarHighlight = color.purple10;
color.stepProgressBarIncomplete = color.grey30;

// Toggle
color.connectingToggle = color.dullPurple;
color.connectingToggleBackground = color.dullGreen;

// Objects (these aren't colors, these are objects of colors)
color.destructiveButton = {
  defaultColor: color.destructiveButtonDefault,
  buttonHovered: color.red75,
  buttonPressed: color.red80,
  buttonDisabled: color.red10,
  focusOutline: addTransparency(color.destructiveButtonDefault, 0.4),
  focusBorder: color.destructiveButtonDefault,
  fontColor: color.destructiveButtonFont,
};

color.successAlert = {
  defaultColor: color.green50,
  buttonHovered: color.green60,
  buttonPressed: color.green70,
  focusOutline: addTransparency(color.green50, 0.2),
  focusBorder: color.green70,
};

color.warningAlert = {
  buttonHovered: color.yellow60,
  buttonPressed: color.yellow70,
  focusOutline: addTransparency(color.yellow60, 0.3),
  focusBorder: color.yellow70,
};

color.textLink = {
  defaultColor: addTransparency(color.grey60, 0.7),
  buttonHovered: addTransparency(color.grey60, 0.8),
  buttonPressed: color.grey60,
};

color.onBoardingGradient = {
  start: color.lighterOnboardingPurple,
  middle: color.mediumOnboardingPurple,
  end: color.darkerOnboardingPurple,
};

color.vpnToggleConnected = {
  defaultColor: color.green50,
  buttonHovered: color.green60,
  buttonPressed: color.green70,
  focusOutline: addTransparency(color.bgColor, 0.3),
  focusBorder: color.focusBorder,
};

color.vpnToggleDisconnected = {
  defaultColor: color.grey30,
  buttonHovered: color.fontColor,
  buttonPressed: color.fontColorDark,
  buttonDisabled: color.disabledButtonColor,
  focusOutline: color.transparent,
  focusBorder: color.focusBorder,
};
