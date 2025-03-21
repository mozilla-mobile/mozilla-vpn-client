/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// This is to determine whether to show the dark mode version of images or not.
color.useDarkAssets = true;

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
color.primaryBrighter = color.blue20;
color.primaryHovered = color.washedPurple;
color.primaryPressed = color.dullPurple;

// Backgrounds
color.bgColor = color.grey45;
color.bgColorStronger = color.brightSlate;
color.overlay = addTransparency(color.black, 0.3);

// Fonts
color.fontColor = color.grey3;
color.fontColorDark = color.white;
color.fontColorWarning = color.yellow50;
color.fontColorInverted = color.white;
color.fontColorInvertedMuted = color.greyWithContrast;
color.fontTitleColor = color.grey3;
color.normalButtonFont = color.grey55;
color.destructiveButtonFont = color.grey55;

// Dividers and borders
color.divider = color.middlingGrey;
color.dropShadow = color.grey60;
color.scrollBar = color.grey40;
color.focusBorder = color.grey40;  // used for keyboard navigation highlighting
                                   // for settings toggles and swipe actions
color.inputHighlight = color.brightSlate;

// Buttons are built off these base colors
color.normalButtonDefault = color.blue20;
color.destructiveButtonDefault = color.red40;

// Success/Failure/Warning/Normal levels
color.normalLevelAccent = color.blue50;
color.successAccent = color.green90;
color.errorAccent = color.red80;
color.errorAccentLight = color.red30;
color.warningAccent = color.yellow70;

// Badges
color.normalLevelText = color.blue40;
color.normalLevelBackground = color.blue90;
color.successText = color.green30;
color.successBackground = color.green90;
color.errorText = color.red30;
color.errorBackground = color.red80;
color.warningText = color.orange10;
color.warningBackground = color.orange80;

// Disabled button colors
color.disabledButtonColor = color.grey30;
color.disabledButtonHovered = color.grey40;
color.disabledButtonPressed = color.grey30;

// Step progress bar (several other colors for bar based on primary color)
color.stepProgressBarHighlight = color.purple20;
color.stepProgressBarIncomplete = color.brightSlate;

color.vpnToggleConnected = {
  defaultColor: color.blue20,
  buttonHovered: mixColors(color.blue20, color.black, 0.2),
  buttonPressed: mixColors(color.blue20, color.black, 0.3),
  focusOutline: addTransparency(color.bgColor, 0.4),
  focusBorder: color.focusBorder,
};

// Toggle
color.connectingToggle = color.dullPurple;
color.connectingToggleBackground =
    mixColors(color.vpnToggleConnected.defaultColor, color.primary, 0.4);

// Objects (these aren't colors, these are objects of colors)
color.destructiveButton = {
  defaultColor: color.destructiveButtonDefault,
  buttonHovered: mixColors(color.destructiveButtonDefault, color.white, 0.5),
  buttonPressed: mixColors(color.destructiveButtonDefault, color.white, 0.65),
  buttonDisabled: mixColors(color.destructiveButtonDefault, color.black, 0.3),
  focusOutline: addTransparency(color.normalButtonDefault, 0.4),
  focusBorder: color.normalButtonDefault,
  fontColor: color.destructiveButtonFont,
};

color.successAlert = {
  defaultColor: color.green80,
  buttonHovered: mixColors(
      color.green80, color.black, 0.3),  // NEED confirmation - matt draft
  buttonPressed: mixColors(
      color.green80, color.black, 0.5),  // NEED confirmation - matt draft
  focusOutline:
      addTransparency(color.green80, 0.4),  // NEED confirmation - matt draft
  focusBorder:
      color.green80,  // or main button color? // NEED confirmation - matt draft
};
color.warningAlert = {
  buttonHovered: mixColors(
      color.yellow70, color.black, 0.3),  // NEED confirmation - matt draft
  buttonPressed: mixColors(
      color.yellow70, color.black, 0.5),  // NEED confirmation - matt draft
  focusOutline:
      addTransparency(color.yellow70, 0.4),  // NEED confirmation - matt draft
  focusBorder:
      color
          .yellow70,  // or main button color? // NEED confirmation - matt draft
};

color.textLink = {
  defaultColor: color.normalButtonDefault,
  buttonHovered: mixColors(color.normalButtonDefault, color.bgColor, 0.5),
  buttonPressed: mixColors(color.normalButtonDefault, color.bgColor, 0.65),
  buttonDisabled: mixColors(color.normalButtonDefault, color.black, 0.3),
};

color.onBoardingGradient = {
  start: color.lighterOnboardingPurple,
  middle: color.mediumOnboardingPurple,
  end: color.darkerOnboardingPurple,
};

color.vpnToggleDisconnected = {
  defaultColor: color.grey30,
  buttonHovered: mixColors(color.grey30, color.black, 0.2),
  buttonPressed: mixColors(color.grey30, color.black, 0.3),
  buttonDisabled: mixColors(color.grey30, color.black, 0.3),
  focusOutline: color.transparent,
  focusBorder: color.focusBorder,
};
