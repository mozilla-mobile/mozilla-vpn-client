/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/**
 * Part 3: Derived theme colors
 * Color objects that dervive from colors set in part 3.
 *
 * The named colors in this section and section 3 SHOULD be used in code -
 * and should be the ONLY colors used in code. (One exception: color.transparent
 * can be used in code.)
 */

color.bgColorTransparent = addTransparency(color.bgColor, 0.0);

color.vpnToggleDisconnectedMainSwitch = {
  defaultColor:
      (color.vpnToggleDisconnectedBorder === color.transparent ?
           color.vpnToggleDisconnected.defaultColor :
           color.bgColorStronger),
  buttonHovered: color.vpnToggleDisconnected.buttonHovered,
  buttonPressed: color.vpnToggleDisconnected.buttonPressed,
  buttonDisabled: color.vpnToggleDisconnected.buttonDisabled,
  focusOutline: color.vpnToggleDisconnected.focusOutline,
  focusBorder: color.vpnToggleDisconnected.focusBorder,
};

// pressed/disabled goes darker on light theme, goes lighter on dark theme
color.normalButton = {
  defaultColor: color.normalButtonDefault,
  buttonHovered: mixColors(color.normalButtonDefault, color.bgColor, 0.35),
  buttonPressed: mixColors(color.normalButtonDefault, color.bgColor, 0.5),
  buttonDisabled: mixColors(
      color.normalButtonDefault, color.black,
      0.3),  // use black here in all cases
  focusOutline: addTransparency(color.normalButtonDefault, 0.5),
  focusBorder: color.normalButtonDefault,
  fontColor: color.normalButtonFont,
};

color.infoAlert = {
  defaultColor: color.normalLevelAccent,
  buttonHovered: mixColors(color.normalLevelAccent, color.black, 0.35),
  buttonPressed: mixColors(color.normalLevelAccent, color.black, 0.5),
  buttonDisabled: mixColors(
      color.normalLevelAccent, color.black,
      0.3),  // use black here in all cases
  focusOutline: addTransparency(color.normalLevelAccent, 0.5),
  focusBorder: color.normalLevelAccent,
};

color.errorAlert = {
  defaultColor: color.errorAccent,
  buttonHovered: mixColors(color.errorAccent, color.black, 0.35),
  buttonPressed: mixColors(color.errorAccent, color.black, 0.5),
  buttonDisabled: mixColors(
      color.errorAccent, color.black,
      0.3),  // use black here in all cases
  focusOutline: addTransparency(color.errorAccent, 0.5),
  focusBorder: color.errorAccent,
};

color.clickableRow = {
  defaultColor: color.bgColor,
  buttonHovered: mixColors(color.normalButtonDefault, color.bgColor, 0.85),
  buttonPressed: mixColors(color.normalButtonDefault, color.bgColor, 0.7),
  focusOutline: addTransparency(color.bgColor, 0.0),
  focusBorder: mixColors(color.normalButtonDefault, color.bgColor, 0.25),
};

color.stepProgressBarDelegate = {
  defaultColor: color.stepProgressBarComplete,
  buttonHovered: color.stepProgressBarComplete,
  buttonPressed: color.primaryPressed,
  buttonDisabled: color.stepProgressBarIncomplete,
  focusOutline: color.bgColorTransparent,
  focusBorder: color.bgColorTransparent,
};

color.invertedButton = {
  defaultColor: color.fontColorInverted,
  buttonHovered: color.disabledButtonHovered,
  buttonPressed: color.disabledButtonPressed,
  focusOutline: color.disabledButtonHovered,
  focusBorder: color.disabledButtonPressed,
};

color.linkButton = {
  defaultColor: color.bgColorTransparent,
  buttonHovered: color.bgColorTransparent,
  buttonPressed: color.bgColorTransparent,
  buttonDisabled: color.bgColorTransparent,
  focusOutline: color.bgColorTransparent,
  focusBorder: color.bgColorTransparent,
};

color.iconButtonLightBackground = {
  defaultColor: color.bgColorTransparent,
  buttonHovered: color.disabledButtonHovered,
  buttonPressed: color.disabledButtonPressed,
  buttonDisabled: color.transparent,
  focusOutline: color.bgColorTransparent,
  focusBorder: color.focusBorder,
};

color.iconButtonDarkBackground = {
  defaultColor: addTransparency(color.primary, 0.0),
  buttonHovered: color.primaryHovered,
  buttonPressed: color.primaryPressed,
  buttonDisabled: addTransparency(color.primary, 0.0),
  focusOutline: addTransparency(color.primaryHovered, 0.0),
  focusBorder: color.focusBorder,
};

color.card = {
  defaultColor: color.bgColorStronger,
  buttonHovered: color.disabledButtonHovered,
  buttonPressed: color.disabledButtonPressed,
  buttonDisabled: color.bgColorTransparent,
  focusOutline: color.bgColorTransparent,
  focusBorder: color.focusBorder,
};

color.inputState = {
  default: {
    border: color.focusBorder,
    placeholder: color.fontColor,
    text: color.fontColorDark,
  },
  hover: {
    border: color.focusBorder,
    placeholder: color.fontColorDark,
    text: color.fontColorDark,
  },
  focus: {
    border: color.normalButton.defaultColor,
    highlight: addTransparency(color.normalButton.defaultColor, 0.4),
    placeholder: color.fontColor,
    text: color.fontColorDark,
  },
  error: {
    border: color.errorAccentLight,
    highlight: color.errorBackground,
    placeholder: color.fontColor,
    text: color.fontColorDark,
  },
  disabled: {
    border: color.disabledButtonColor,
    placeholder: color.disabledButtonColor,
    text: color.disabledButtonColor,
  },
};

// 3 files get combined into one JS color object - As this is the last file,
// this next line is needed to create the object.
color;
