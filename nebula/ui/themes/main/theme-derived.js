/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/**
 * Part 4: Derived theme colors
 * Color objects that dervive from colors set in part 3.
 *
 * The named colors in this section and section 3 SHOULD be used in code -
 * and should be the ONLY colors used in code. (One exception: color.transparent
 * can be used in code.)
 */

color.stepProgressBarDelegate = {
  defaultColor: color.primaryBrighter,
  buttonHovered: color.primaryBrighter,
  buttonPressed: color.primaryPressed,
  buttonDisabled: color.stepProgressBarIncomplete,
  focusOutline: color.bgColorTransparent,
  focusBorder: color.bgColorTransparent,
};

color.invertedButton = {
  defaultColor: color.fontColorInverted,
  buttonHovered: color.radioButtonDisabledHovered,
  buttonPressed: color.radioButtonDisabledPressed,
  focusOutline: color.radioButtonDisabledHovered,
  focusBorder: color.radioButtonDisabledPressed,
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
  buttonHovered: color.radioButtonDisabledHovered,
  buttonPressed: color.radioButtonDisabledPressed,
  buttonDisabled: color.transparent,
  focusOutline: color.bgColorTransparent,
  focusBorder: color.darkFocusBorder,
};

color.iconButtonDarkBackground = {
  defaultColor: addTransparency(color.primary, 0.0),
  buttonHovered: color.primaryHovered,
  buttonPressed: color.primaryPressed,
  buttonDisabled: addTransparency(color.primary, 0.0),
  focusOutline: color.transparent,
  focusBorder: color.lightFocusBorder,
};

color.inputState = {
  default: {
    border: color.darkFocusBorder,
    placeholder: color.fontColor,
    text: color.fontColorDark,
  },
  hover: {
    border: color.darkFocusBorder,
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
