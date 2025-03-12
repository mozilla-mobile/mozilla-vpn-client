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
color.primary = color.purple90;  // set
color.primaryBrighter = color.blue20;       // set
color.primaryHovered = color.washedPurple;  // set
color.primaryPressed = color.dullPurple;    // set

// Backgrounds
color.bgColor = color.grey45;  // set
color.bgColorStronger = color.brightSlate;          // set
color.overlay = addTransparency(color.black, 0.3);  // done

// Fonts
color.fontColor = color.grey3;                                     // done
color.fontColorDark = color.grey3;                                 // done
color.fontColorWarning = color.yellow50;                           // set
color.fontColorInverted = color.grey3;                             // done
color.fontColorInvertedMuted = addTransparency(color.grey3, 0.8);  // done
color.fontTitleColor = color.grey3;                                // done

// Dividers and borders
color.divider = addTransparency(color.grey3, 0.2);           // done
color.dropShadow = color.grey60;                             // done
color.scrollBar = color.grey40;
color.focusBorder = addTransparency(
    color.grey3, 0.2);  // done  // used for keyboard navigation highlighting
                        // for settings toggles and swipe actions
color.inputHighlight = color.brightSlate;                    // set

// Success/Failure/Warning/Normal levels
color.successMain = color.green90;        // set
color.successAccent = color.green80;      // set
color.successBackground = color.green70;  // set
color.errorMain = color.red80;            // set
color.errorAccent = color.red70;          // set
color.errorAccentLight = color.grey30;  // set
color.errorBackground = color.red10;    // set
color.warningMain = color.yellow70;     // set
color.warningAccent = color.yellow50;   // set
color.warningBackground = color.orange10;                          // set
color.normalLevelMain = color.blue90;                              // set
color.normalLevelAccent = color.blue50;                            // set
color.normalLevelBackground = addTransparency(color.blue90, 0.4);  // set

// Disabled button colors
color.disabledButtonColor =
    addTransparency(color.blue20, 0.6);  // HOW DOES THIS LOOK WITH TRANSPARENCY
color.disabledButtonHovered = addTransparency(color.white, 0.1);  // SET
color.disabledButtonPressed = addTransparency(color.white, 0.2);  // SET

// Step progress bar (several other colors for bar based on primary color)
color.stepProgressBarHighlight = color.purple20;      // SET
color.stepProgressBarIncomplete = color.brightSlate;  // set

// Toggle
color.connectingToggle = color.dullPurple;        // SET
color.connectingToggleBackground = color.grey30;  // SET

// Objects (these aren't colors, these are objects of colors)
color.destructiveButton = {
  defaultColor: color.red80,  // SET
  buttonHovered: color.red75,
  buttonPressed: color.red80,
  buttonDisabled: color.red10,
  focusOutline: addTransparency(color.red70, 0.4),
  focusBorder: color.red70,
};
color.normalButton = {
  defaultColor: color.blue20,  // SET
  buttonHovered: color.blue60,
  buttonPressed: color.blue70,
  buttonDisabled: color.washedLightBlue,
  focusOutline: addTransparency(color.strongBlue, 0.3),
  focusBorder: color.strongBlue,
};
color.successAlert = {
  defaultColor: color.green80,  // SET
  buttonHovered: color.green60,
  buttonPressed: color.green70,
  focusOutline: addTransparency(color.green50, 0.2),
  focusBorder: color.green70,
};
color.warningAlert = {
  buttonHovered: color.yellow70,  // SET
  buttonPressed: color.yellow70,
  focusOutline: addTransparency(color.yellow60, 0.3),
  focusBorder: color.yellow70,
};
color.clickableRow = {
  defaultColor: addTransparency(color.white, 0.1),  // SET
  buttonHovered: color.washedBlue,
  buttonPressed: color.washedLightBlue,
  focusOutline: addTransparency(color.grey5, 0.0),
  focusBorder: color.strongBlue,
};
color.textLink = {
  defaultColor: color.blue20,  // SET
  buttonHovered: addTransparency(color.grey60, 0.8),
  buttonPressed: color.grey60,
};

color.onBoardingGradient = {
  // set
  start: color.lighterOnboardingPurple,
  middle: color.mediumOnboardingPurple,
  end: color.darkerOnboardingPurple,
};

color.vpnToggleConnected = {
  defaultColor: color.blue20,  // set
  buttonHovered: color.green60,
  buttonPressed: color.green70,
  focusOutline: addTransparency(color.bgColor, 0.3),
  focusBorder: color.focusBorder,
};

color.vpnToggleDisconnected = {
  defaultColor: color.grey30,  // set
  buttonHovered: color.fontColor,
  buttonPressed: color.fontColorDark,
  buttonDisabled: color.disabledButtonColor,
  focusOutline: color.transparent,
  focusBorder: color.focusBorder,
};
