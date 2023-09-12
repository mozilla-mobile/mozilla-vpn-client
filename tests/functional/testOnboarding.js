/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const queries = require('./queries.js');
const vpn = require('./helper.js');

describe('Onboarding', function() {
  beforeEach(async () => {
    await vpn.hardReset();
     await vpn.flipFeatureOn("newOnboarding")
     await vpn.authenticateInApp();
  });

  async function clickNavigationButton(button) {
    await vpn.wait()
    await vpn.waitForQueryAndClick(button.visible());
    await vpn.wait()
  }

  it('Complete onboarding', async () => {
    await vpn.waitForQuery(queries.screenOnboarding.SCREEN.visible());
    await vpn.waitForQuery(queries.screenOnboarding.ONBOARDING_VIEW.visible());

    //DATA SLIDE
    //Do some checks on the initial state of the data slide
    await vpn.waitForQuery(queries.screenOnboarding.DATA_SLIDE.visible());
    assert.equal(await vpn.getSetting('onboardingStep'), 0);
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.DATA_CHECKBOX, 'isChecked'), 'false');
    assert.equal(await vpn.getSetting('gleanEnabled'), false);

    //Check data collection checkbox
    await vpn.waitForQueryAndClick(queries.screenOnboarding.DATA_CHECKBOX.visible());
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.DATA_CHECKBOX, 'isChecked'), 'true');
    assert.equal(await vpn.getSetting('gleanEnabled'), true);

    //Check privacy link
    await vpn.waitForQueryAndClick(queries.screenOnboarding.DATA_PRIVACY_LINK.visible());
    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
        return url.endsWith('/r/vpn/privacy');
    });

    //Proceed to privacy slide
    await clickNavigationButton(queries.screenOnboarding.DATA_NEXT_BUTTON)


    //PRIVACY SLIDE
    //Do some checks on the initial state of the privacy slide
    await vpn.waitForQuery(queries.screenOnboarding.PRIVACY_SLIDE.visible());
    assert.equal(await vpn.getSetting('onboardingStep'), 1);
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.PRIVACY_BLOCK_ADS_CHECKBOX, 'checked'), 'false');
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.PRIVACY_BLOCK_TRACKERS_CHECKBOX, 'checked'), 'false');
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.PRIVACY_BLOCK_MALWARE_CHECKBOX, 'checked'), 'false');
    assert.equal(await vpn.getSetting('dnsProviderFlags'), 0);

    //Clicks block ads checkbox
    await vpn.waitForQueryAndClick(queries.screenOnboarding.PRIVACY_BLOCK_ADS_CHECKBOX.visible());
    // await vpn.wait(2000)
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.PRIVACY_BLOCK_ADS_CHECKBOX, 'checked'), 'true');
    assert.equal(await vpn.getSetting('dnsProviderFlags'), 2);

    //Clicks block trackers checkbox
    await vpn.waitForQueryAndClick(queries.screenOnboarding.PRIVACY_BLOCK_TRACKERS_CHECKBOX.visible());
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.PRIVACY_BLOCK_TRACKERS_CHECKBOX, 'checked'), 'true');
    assert.equal(await vpn.getSetting('dnsProviderFlags'), 6);

    //Clicks block trackers checkbox
    await vpn.waitForQueryAndClick(queries.screenOnboarding.PRIVACY_BLOCK_MALWARE_CHECKBOX.visible());
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.PRIVACY_BLOCK_MALWARE_CHECKBOX, 'checked'), 'true');
    assert.equal(await vpn.getSetting('dnsProviderFlags'), 14);

    //Test back button
    await clickNavigationButton(queries.screenOnboarding.PRIVACY_BACK_BUTTON)
    await vpn.waitForQuery(queries.screenOnboarding.DATA_SLIDE.visible());

    //Ensure all selections on previous slide are saved
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.DATA_CHECKBOX, 'isChecked'), 'true');

    //Proceed to devices slide
    await clickNavigationButton(queries.screenOnboarding.DATA_NEXT_BUTTON)
    await clickNavigationButton(queries.screenOnboarding.PRIVACY_NEXT_BUTTON)


    //DEVICES SLIDE
    //Do some checks on the initial state of the devices slide
    await vpn.waitForQuery(queries.screenOnboarding.DEVICES_SLIDE.visible());
    assert.equal(await vpn.getSetting('onboardingStep'), 2);

    //Switch between toggle buttons
    if (await vpn.getQueryProperty(queries.screenOnboarding.DEVICES_DEVICE_TYPE_TOGGLE.visible(), 'selectedIndex') === 0) {
      //Starting with Android - switch to iOS and back to Android
      await vpn.waitForQueryAndClick(queries.screenOnboarding.DEVICES_TOGGLE_BTN_IOS.visible());
      await vpn.waitForQuery(queries.screenOnboarding.DEVICES_APP_STORE_QRCODE.visible());
      await vpn.waitForQueryAndClick(queries.screenOnboarding.DEVICES_TOGGLE_BTN_ANDROID.visible());
      await vpn.waitForQuery(queries.screenOnboarding.DEVICES_PLAY_STORE_QRCODE.visible());
    }
    else {
      //Starting with iOS - switch to Android and back to iOS
      await vpn.waitForQueryAndClick(queries.screenOnboarding.DEVICES_TOGGLE_BTN_ANDROID.visible());
      await vpn.waitForQuery(queries.screenOnboarding.DEVICES_PLAY_STORE_QRCODE.visible());
      await vpn.waitForQueryAndClick(queries.screenOnboarding.DEVICES_TOGGLE_BTN_IOS.visible());
      await vpn.waitForQuery(queries.screenOnboarding.DEVICES_APP_STORE_QRCODE.visible());
    }

    //Test back button
    await clickNavigationButton(queries.screenOnboarding.DEVICES_BACK_BUTTON)
    await vpn.waitForQuery(queries.screenOnboarding.PRIVACY_SLIDE.visible());

    //Ensure all selections on previous slide are saved
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.PRIVACY_BLOCK_ADS_CHECKBOX, 'checked'), 'true');
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.PRIVACY_BLOCK_TRACKERS_CHECKBOX, 'checked'), 'true');
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.PRIVACY_BLOCK_MALWARE_CHECKBOX, 'checked'), 'true');

    //Proceed to start slide
    await clickNavigationButton(queries.screenOnboarding.PRIVACY_NEXT_BUTTON)
    await clickNavigationButton(queries.screenOnboarding.DEVICES_NEXT_BUTTON)


    //START SLIDE
    //Do some checks on the initial state of the start slide
    await vpn.waitForQuery(queries.screenOnboarding.START_SLIDE.visible());
    assert.equal(await vpn.getSetting('onboardingStep'), 3);
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.START_START_AT_BOOT_CHECKBOX, 'isChecked'), 'false');
    assert.equal(await vpn.getSetting('startAtBoot'), false);

    //Check start at boot checkbox
    await vpn.waitForQueryAndClick(queries.screenOnboarding.START_START_AT_BOOT_CHECKBOX.visible());
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.START_START_AT_BOOT_CHECKBOX, 'isChecked'), 'true');
    assert.equal(await vpn.getSetting('startAtBoot'), true);

    //Test back button
    await clickNavigationButton(queries.screenOnboarding.START_BACK_BUTTON)
    await vpn.waitForQuery(queries.screenOnboarding.DEVICES_SLIDE.visible());

    //Proceed to completing onboarding
    await clickNavigationButton(queries.screenOnboarding.DEVICES_NEXT_BUTTON)
    await clickNavigationButton(queries.screenOnboarding.START_NEXT_BUTTON)
  });
});