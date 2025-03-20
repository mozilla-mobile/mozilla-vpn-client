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
color.fontColorInvertedMuted = addTransparency(
    color.grey3,
    0.8);  // how does this work w/ transparency - make sure it works okay
color.fontTitleColor = color.grey3;
color.normalButtonFont = color.grey55;
color.destructiveButtonFont = color.grey55;

// Dividers and borders
color.divider = addTransparency(
    color.grey3,
    0.2);  // how does this work w/ transparency - make sure it works okay
color.dropShadow = color.grey60;
color.scrollBar = color.grey40;
color.focusBorder = color.grey40;  // used for keyboard navigation highlighting
                                   // for settings toggles and swipe actions
color.inputHighlight = color.brightSlate;

// Buttons are built off these base colors
color.normalButtonDefault = color.blue20;
color.destructiveButtonDefault = color.red50;

// Success/Failure/Warning/Normal levels
color.normalLevelMain = color.blue30;
color.normalLevelAccent = color.blue50;
color.normalLevelBackground = color.blue90;
color.successMain = color.green40;
color.successAccent = color.green80;
color.successBackground = color.green90;
color.errorMain = color.red80;
color.errorAccent = color.red70;
color.errorAccentLight = color.red30;
color.errorBackground = color.red50;
color.warningMain = color.yellow70;
color.warningAccent = color.yellow50;
color.warningBackground = color.orange60;

// Disabled button colors
color.disabledButtonColor = addTransparency(
    color.blue20,
    0.6);  // how does this work w/ transparency - make sure it works okay
color.disabledButtonHovered = addTransparency(color.white, 0.1);
color.disabledButtonPressed = addTransparency(color.white, 0.2);

// Step progress bar (several other colors for bar based on primary color)
color.stepProgressBarHighlight = color.purple20;
color.stepProgressBarIncomplete = color.brightSlate;

color.vpnToggleConnected = {
  defaultColor: color.blue20,
  buttonHovered: color.green60,                       // NEED THIS
  buttonPressed: color.green70,                       // NEED THIS
  focusOutline: addTransparency(color.bgColor, 0.3),  // NEED THIS
  focusBorder: color.focusBorder,                     // NEED THIS
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
};

color.successAlert = {
  defaultColor: color.green80,
  buttonHovered: color.green60,                       // NEED THIS
  buttonPressed: color.green70,                       // NEED THIS
  focusOutline: addTransparency(color.green50, 0.2),  // NEED THIS
  focusBorder: color.green70,                         // NEED THIS
};
color.warningAlert = {
  buttonHovered: color.yellow50,
  buttonPressed: color.yellow70,                       // NEED THIS
  focusOutline: addTransparency(color.yellow60, 0.3),  // NEED THIS
  focusBorder: color.yellow70,                         // NEED THIS
};
color.textLink = {
  defaultColor: color.blue20,
  buttonHovered: addTransparency(color.grey60, 0.8),  //  SET THIS TO 50% white
  buttonPressed: color.grey60,                        // SET THIS TO 65% white
  // buttonDisabled: 30% black
};

color.onBoardingGradient = {
  start: color.lighterOnboardingPurple,
  middle: color.mediumOnboardingPurple,
  end: color.darkerOnboardingPurple,
};

color.vpnToggleDisconnected = {
  defaultColor: color.grey30,
  buttonHovered: color.fontColor,             // NEED THIS
  buttonPressed: color.fontColorDark,         // NEED THIS
  buttonDisabled: color.disabledButtonColor,  // NEED THIS
  focusOutline: color.transparent,            // NEED THIS
  focusBorder: color.focusBorder,             // NEED THIS
};
