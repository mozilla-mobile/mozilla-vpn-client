/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const {navBar, settingsScreen, homeScreen} = require('./elements.js');
const vpn = require('./helper.js');

describe('Addons', function() {
  this.ctx.authenticationNeeded = true;

  it('Empty addon index', async () => {
    await vpn.resetAddons('01_empty_manifest');
    await vpn.waitForCondition(async () => {
      return parseInt(
                 await vpn.getElementProperty('VPNAddonManager', 'count'),
                 10) === 0;
    });
  });

  it('Broken addon index', async () => {
    await vpn.resetAddons('03_single_addon');
    await vpn.waitForCondition(async () => {
      return parseInt(
                 await vpn.getElementProperty('VPNAddonManager', 'count'),
                 10) === 1;
    });

    await vpn.fetchAddons('02_broken_manifest');
    await vpn.waitForCondition(async () => {
      return parseInt(
                 await vpn.getElementProperty('VPNAddonManager', 'count'),
                 10) === 1;
    });
  });

  it('Addons are loaded', async () => {
    await vpn.resetAddons('03_single_addon');
    await vpn.waitForCondition(async () => {
      return parseInt(
                 await vpn.getElementProperty('VPNAddonManager', 'count'),
                 10) === 1;
    });

    await vpn.fetchAddons('01_empty_manifest');
    await vpn.waitForCondition(async () => {
      return parseInt(
                 await vpn.getElementProperty('VPNAddonManager', 'count'),
                 10) === 0;
    });

    await vpn.fetchAddons('03_single_addon');
    await vpn.waitForCondition(async () => {
      return parseInt(
                 await vpn.getElementProperty('VPNAddonManager', 'count'),
                 10) === 1;
    });
  });

  it('Settings rollback - location', async () => {
    // Loading the custom tutorial
    await vpn.resetAddons('05_settings_rollback');

    await vpn.waitForCondition(async () => {
      return parseInt(
                 await vpn.getElementProperty('VPNAddonManager', 'count'),
                 10) === 1;
    });

    const exitCityName =
        await vpn.getElementProperty('VPNCurrentServer', 'exitCityName');
    const exitCountryCode =
        await vpn.getElementProperty('VPNCurrentServer', 'exitCountryCode');

    // Let's start the tutorial
    await vpn.waitForElementAndClick(navBar.SETTINGS);
    await vpn.waitForElementAndClick(settingsScreen.TIPS_AND_TRICKS);
    await vpn.waitForElementAndClick(homeScreen.TUTORIAL_LIST_HIGHLIGHT);

    // Confirmation dialog for settings-rollback
    await vpn.waitForElementProperty(
        homeScreen.TUTORIAL_POPUP_PRIMARY_BUTTON, 'visible', 'true');
    assert(
        (await vpn.getElementProperty(
            homeScreen.TUTORIAL_POPUP_PRIMARY_BUTTON, 'text')) === 'Continue');
    await vpn.waitForElementAndClick(homeScreen.TUTORIAL_POPUP_PRIMARY_BUTTON);

    await vpn.waitForCondition(async () => {
      return await vpn.getElementProperty(
                 'VPNCurrentServer', 'exitCityName') === 'Vienna';
    });

    assert(
        await vpn.getElementProperty('VPNCurrentServer', 'exitCityName') ===
        'Vienna');
    assert(
        await vpn.getElementProperty('VPNCurrentServer', 'exitCountryCode') ===
        'at');

    await vpn.waitForElementAndClick('serverListButton-btn');

    // Final dialog
    await vpn.waitForElementProperty(
        homeScreen.TUTORIAL_POPUP_PRIMARY_BUTTON, 'visible', 'true');
    assert(
        (await vpn.getElementProperty(
            homeScreen.TUTORIAL_POPUP_PRIMARY_BUTTON, 'text')) === 'Let’s go!');
    await vpn.waitForElementAndClick(
        homeScreen.TUTORIAL_POPUP_SECONDARY_BUTTON);

    await vpn.waitForCondition(async () => {
      return await vpn.getElementProperty(
                 'VPNCurrentServer', 'exitCityName') === exitCityName;
    });

    assert(
        await vpn.getElementProperty('VPNCurrentServer', 'exitCityName') ===
        exitCityName);
    assert(
        await vpn.getElementProperty('VPNCurrentServer', 'exitCountryCode') ===
        exitCountryCode);
  });
});
