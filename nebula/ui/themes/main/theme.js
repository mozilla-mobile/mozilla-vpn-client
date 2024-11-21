/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/**
 * Part 3: Theme colors
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
color.bgColorTransparent = addTransparency(color.bgColor, 0.0);
color.bgColorStronger = color.white;
color.overlayBackground = addTransparency(color.black, 0.3);

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
