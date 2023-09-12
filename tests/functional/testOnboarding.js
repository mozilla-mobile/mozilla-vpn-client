/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const queries = require('./queries.js');
const vpn = require('./helper.js');
const setup = require('./setupVpn.js');

describe('Onboarding', function() {
  beforeEach(async () => {
    await vpn.hardReset();
    await vpn.flipFeatureOn("newOnboarding")
    assert.equal(await vpn.getSetting('onboardingStarted'), false);
    assert.equal(await vpn.getSetting('onboardingCompleted'), false);
    await vpn.authenticateInApp();
  });

  //Tests pretty much all of the new onboarding, including all slides, controls, navigation, and ensuring settings persist into StateMain
  it('Complete onboarding', async () => {
    await vpn.waitForQuery(queries.screenOnboarding.SCREEN.visible());
    await vpn.waitForQuery(queries.screenOnboarding.ONBOARDING_VIEW.visible());
    assert.equal(await vpn.getSetting('onboardingStarted'), true);

    //DATA SLIDE
    //Do some checks on the initial state of the data slide
    await vpn.waitForQuery(queries.screenOnboarding.DATA_SLIDE.visible());
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.ONBOARDING_VIEW, 'currentIndex'), 0);
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
    await vpn.waitForQueryAndClick(queries.screenOnboarding.DATA_NEXT_BUTTON.visible());
    await vpn.waitForQuery(queries.screenOnboarding.STEP_NAV_STACK_VIEW.ready());


    //PRIVACY SLIDE
    //Do some checks on the initial state of the privacy slide
    await vpn.waitForQuery(queries.screenOnboarding.PRIVACY_SLIDE.visible());
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.ONBOARDING_VIEW, 'currentIndex'), 1);
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
    await vpn.waitForQueryAndClick(queries.screenOnboarding.PRIVACY_BACK_BUTTON.visible());
    await vpn.waitForQuery(queries.screenOnboarding.STEP_NAV_STACK_VIEW.ready());
    await vpn.waitForQuery(queries.screenOnboarding.DATA_SLIDE.visible());
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.ONBOARDING_VIEW, 'currentIndex'), 0);

    //Ensure all selections on previous slide are saved
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.DATA_CHECKBOX, 'isChecked'), 'true');

    //Return to privacy slide
    await vpn.waitForQueryAndClick(queries.screenOnboarding.DATA_NEXT_BUTTON.visible());
    await vpn.waitForQuery(queries.screenOnboarding.STEP_NAV_STACK_VIEW.ready());

    //Test going back one slide via progress bar
    await vpn.waitForQueryAndClick(queries.screenOnboarding.STEP_PROG_BAR_DATA_BUTTON.visible());
    await vpn.waitForQuery(queries.screenOnboarding.STEP_NAV_STACK_VIEW.ready());

    //Proceed to devices slide
    await vpn.waitForQueryAndClick(queries.screenOnboarding.DATA_NEXT_BUTTON.visible());
    await vpn.waitForQuery(queries.screenOnboarding.STEP_NAV_STACK_VIEW.ready());
    await vpn.waitForQueryAndClick(queries.screenOnboarding.PRIVACY_NEXT_BUTTON.visible());
    await vpn.waitForQuery(queries.screenOnboarding.STEP_NAV_STACK_VIEW.ready());


    //DEVICES SLIDE
    //Do some checks on the initial state of the devices slide
    await vpn.waitForQuery(queries.screenOnboarding.DEVICES_SLIDE.visible());
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.ONBOARDING_VIEW, 'currentIndex'), 2);
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
    await vpn.waitForQueryAndClick(queries.screenOnboarding.DEVICES_BACK_BUTTON.visible());
    await vpn.waitForQuery(queries.screenOnboarding.STEP_NAV_STACK_VIEW.ready());
    await vpn.waitForQuery(queries.screenOnboarding.PRIVACY_SLIDE.visible());
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.ONBOARDING_VIEW, 'currentIndex'), 1);

    //Ensure all selections on previous slide are saved
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.PRIVACY_BLOCK_ADS_CHECKBOX, 'checked'), 'true');
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.PRIVACY_BLOCK_TRACKERS_CHECKBOX, 'checked'), 'true');
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.PRIVACY_BLOCK_MALWARE_CHECKBOX, 'checked'), 'true');

    //Return to devices slide
    await vpn.waitForQueryAndClick(queries.screenOnboarding.PRIVACY_NEXT_BUTTON.visible());
    await vpn.waitForQuery(queries.screenOnboarding.STEP_NAV_STACK_VIEW.ready());

    //Test going back one slide via progress bar
    await vpn.waitForQueryAndClick(queries.screenOnboarding.STEP_PROG_BAR_PRIVACY_BUTTON.visible());
    await vpn.waitForQuery(queries.screenOnboarding.STEP_NAV_STACK_VIEW.ready());

    //Proceed to start slide
    await vpn.waitForQueryAndClick(queries.screenOnboarding.PRIVACY_NEXT_BUTTON.visible());
    await vpn.waitForQuery(queries.screenOnboarding.STEP_NAV_STACK_VIEW.ready());
    await vpn.waitForQueryAndClick(queries.screenOnboarding.DEVICES_NEXT_BUTTON.visible());
    await vpn.waitForQuery(queries.screenOnboarding.STEP_NAV_STACK_VIEW.ready());


    //START SLIDE
    //Do some checks on the initial state of the start slide
    await vpn.waitForQuery(queries.screenOnboarding.START_SLIDE.visible());
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.ONBOARDING_VIEW, 'currentIndex'), 3);
    assert.equal(await vpn.getSetting('onboardingStep'), 3);
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.START_START_AT_BOOT_CHECKBOX, 'isChecked'), 'false');
    assert.equal(await vpn.getSetting('startAtBoot'), false);

    //Check start at boot checkbox
    await vpn.waitForQueryAndClick(queries.screenOnboarding.START_START_AT_BOOT_CHECKBOX.visible());
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.START_START_AT_BOOT_CHECKBOX, 'isChecked'), 'true');
    assert.equal(await vpn.getSetting('startAtBoot'), true);

    //Test back button
    await vpn.waitForQueryAndClick(queries.screenOnboarding.START_BACK_BUTTON.visible());
    await vpn.waitForQuery(queries.screenOnboarding.STEP_NAV_STACK_VIEW.ready());
    await vpn.waitForQuery(queries.screenOnboarding.DEVICES_SLIDE.visible());
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.ONBOARDING_VIEW, 'currentIndex'), 2);

    //Return to start slide
    await vpn.waitForQueryAndClick(queries.screenOnboarding.DEVICES_NEXT_BUTTON.visible());
    await vpn.waitForQuery(queries.screenOnboarding.STEP_NAV_STACK_VIEW.ready());

    //Ensure all selections on start slide are saved
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.START_START_AT_BOOT_CHECKBOX, 'isChecked'), 'true');
    assert.equal(await vpn.getSetting('startAtBoot'), true);

    //Test going back one slide via progress bar
    await vpn.waitForQueryAndClick(queries.screenOnboarding.STEP_PROG_BAR_DEVICES_BUTTON.visible());
    await vpn.waitForQuery(queries.screenOnboarding.STEP_NAV_STACK_VIEW.ready());

    //Return to start slide
    await vpn.waitForQueryAndClick(queries.screenOnboarding.DEVICES_NEXT_BUTTON.visible());
    await vpn.waitForQuery(queries.screenOnboarding.STEP_NAV_STACK_VIEW.ready());

    //Test going all the way back to the first slide
    await vpn.waitForQueryAndClick(queries.screenOnboarding.STEP_PROG_BAR_DATA_BUTTON.visible());
    await vpn.waitForQuery(queries.screenOnboarding.STEP_NAV_STACK_VIEW.ready());

    //Proceed by completing onboarding
    await vpn.waitForQueryAndClick(queries.screenOnboarding.DATA_NEXT_BUTTON.visible());
    await vpn.waitForQuery(queries.screenOnboarding.STEP_NAV_STACK_VIEW.ready());
    await vpn.waitForQueryAndClick(queries.screenOnboarding.PRIVACY_NEXT_BUTTON.visible());
    await vpn.waitForQuery(queries.screenOnboarding.STEP_NAV_STACK_VIEW.ready());
    await vpn.waitForQueryAndClick(queries.screenOnboarding.DEVICES_NEXT_BUTTON.visible());
    await vpn.waitForQuery(queries.screenOnboarding.STEP_NAV_STACK_VIEW.ready());
    await vpn.waitForQueryAndClick(queries.screenOnboarding.START_NEXT_BUTTON.visible());
    await vpn.waitForQuery(queries.screenHome.SCREEN.visible());
    assert.equal(await vpn.getSetting('onboardingCompleted'), true);


    //Test that changes made during onboarding persist
    await vpn.waitForQueryAndClick(queries.navBar.SETTINGS);

    //Check privacy settings
    await vpn.waitForQueryAndClick(queries.screenSettings.PRIVACY.visible());
    assert.equal(await vpn.getQueryProperty(queries.screenSettings.privacyView.BLOCK_ADS_CHECKBOX, 'checked'), 'true');
    assert.equal(await vpn.getQueryProperty(queries.screenSettings.privacyView.BLOCK_TRACKERS_CHECKBOX, 'checked'), 'true');
    assert.equal(await vpn.getQueryProperty(queries.screenSettings.privacyView.BLOCK_MALWARE_CHECKBOX, 'checked'), 'true');
    await vpn.waitForQueryAndClick(queries.screenSettings.BACK.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    //Check data collection and start at boot
    await vpn.waitForQueryAndClick(queries.screenSettings.APP_PREFERENCES.visible());
    assert.equal(await vpn.getQueryProperty(queries.screenSettings.appPreferencesView.DATA_COLLECTION, 'isChecked'), 'true');
    assert.equal(await vpn.getQueryProperty(queries.screenSettings.appPreferencesView.START_AT_BOOT, 'isChecked'), 'true');
  });
  
   //Tests restoring onboarding to current step after quitting 
  it('Quitting app during onboarding', async () => {
    await vpn.waitForQuery(queries.screenOnboarding.SCREEN.visible());
    await vpn.waitForQuery(queries.screenOnboarding.ONBOARDING_VIEW.visible());

    //Ensure we are on the first onboarding step
    await vpn.waitForQuery(queries.screenOnboarding.DATA_SLIDE.visible());
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.ONBOARDING_VIEW, 'currentIndex'), 0);
    assert.equal(await vpn.getSetting('onboardingStep'), 0);

    //Quit and relaunch the app
    await vpn.quit()
    await setup.startAndConnect()

    //Ensure we start where we left off
    await vpn.waitForQuery(queries.screenOnboarding.DATA_SLIDE.visible());
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.ONBOARDING_VIEW, 'currentIndex'), 0);
    assert.equal(await vpn.getSetting('onboardingStep'), 0);

    //Proceed to second onboarding step
    await vpn.waitForQueryAndClick(queries.screenOnboarding.DATA_NEXT_BUTTON.visible());
    await vpn.waitForQuery(queries.screenOnboarding.STEP_NAV_STACK_VIEW.ready());

    //Quit and relaunch the app
    await vpn.quit()
    await setup.startAndConnect()

    //Ensure we start where we left off
    await vpn.waitForQuery(queries.screenOnboarding.PRIVACY_SLIDE.visible());
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.ONBOARDING_VIEW, 'currentIndex'), 1);
    assert.equal(await vpn.getSetting('onboardingStep'), 1);

    //Proceed to third onboarding step
    await vpn.waitForQueryAndClick(queries.screenOnboarding.PRIVACY_NEXT_BUTTON.visible());
    await vpn.waitForQuery(queries.screenOnboarding.STEP_NAV_STACK_VIEW.ready());

    //Quit and relaunch the app
    await vpn.quit()
    await setup.startAndConnect()

    //Ensure we start where we left off
    await vpn.waitForQuery(queries.screenOnboarding.DEVICES_SLIDE.visible());
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.ONBOARDING_VIEW, 'currentIndex'), 2);
    assert.equal(await vpn.getSetting('onboardingStep'), 2);

    //Proceed to fourth/final onboarding step
    await vpn.waitForQueryAndClick(queries.screenOnboarding.DEVICES_NEXT_BUTTON.visible());
    await vpn.waitForQuery(queries.screenOnboarding.STEP_NAV_STACK_VIEW.ready());

    //Quit and relaunch the app
    await vpn.quit()
    await setup.startAndConnect()

    //Ensure we start where we left off
    await vpn.waitForQuery(queries.screenOnboarding.START_SLIDE.visible());
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.ONBOARDING_VIEW, 'currentIndex'), 3);
    assert.equal(await vpn.getSetting('onboardingStep'), 3);
  });
});