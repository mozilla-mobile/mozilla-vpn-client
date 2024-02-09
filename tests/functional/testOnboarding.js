/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const queries = require('./queries.js');
const vpn = require('./helper.js');
const setup = require('./setupVpn.js');

describe('Onboarding', function() {
  this.timeout(120000);

  beforeEach(async () => {
    await vpn.flipFeatureOn("newOnboarding");
    assert.equal(await vpn.getSetting('onboardingCompleted'), false);
    assert.equal(await vpn.getSetting('onboardingStep'), 0);
    await vpn.authenticateInApp(false);
  });

 async function completeOnboarding() {
   await vpn.waitForQuery(queries.screenOnboarding.STEP_NAV_STACK_VIEW.ready());
   switch(await vpn.getSetting('onboardingStep')) {
   case 0:
    await vpn.waitForQueryAndClick(queries.screenOnboarding.DATA_NEXT_BUTTON.visible());
    await vpn.waitForQuery(queries.screenOnboarding.STEP_NAV_STACK_VIEW.ready());
   case 1:
    await vpn.waitForQueryAndClick(queries.screenOnboarding.PRIVACY_NEXT_BUTTON.visible());
    await vpn.waitForQuery(queries.screenOnboarding.STEP_NAV_STACK_VIEW.ready());
   case 2:
    await vpn.waitForQueryAndClick(queries.screenOnboarding.DEVICES_NEXT_BUTTON.visible());
    await vpn.waitForQuery(queries.screenOnboarding.STEP_NAV_STACK_VIEW.ready());
   case 3:
    await vpn.waitForQueryAndClick(queries.screenOnboarding.START_NEXT_BUTTON.visible());
    await vpn.waitForQuery(queries.screenHome.SCREEN.visible());
    assert.equal(await vpn.getSetting('onboardingCompleted'), true);
  default:
    break;
   }
 }

  async function advanceToSlide(slide) {
   await vpn.waitForQuery(queries.screenOnboarding.STEP_NAV_STACK_VIEW.ready());
   const currentSlide = await vpn.getSetting('onboardingStep')
   switch(currentSlide) {
   case 0:
    await vpn.waitForQueryAndClick(queries.screenOnboarding.DATA_NEXT_BUTTON.visible());
    await vpn.waitForQuery(queries.screenOnboarding.STEP_NAV_STACK_VIEW.ready());
    if (await vpn.getSetting('onboardingStep') === slide) break;
   case 1:
    await vpn.waitForQueryAndClick(queries.screenOnboarding.PRIVACY_NEXT_BUTTON.visible());
    await vpn.waitForQuery(queries.screenOnboarding.STEP_NAV_STACK_VIEW.ready());
    if (await vpn.getSetting('onboardingStep') === slide) break;
   case 2:
    await vpn.waitForQueryAndClick(queries.screenOnboarding.DEVICES_NEXT_BUTTON.visible());
    await vpn.waitForQuery(queries.screenOnboarding.STEP_NAV_STACK_VIEW.ready());
    if (await vpn.getSetting('onboardingStep') === slide) break;
  default:
    break;
   }
 }

  //Tests pretty much all of the new onboarding, including all slides, controls, navigation, and ensuring settings persist into StateMain
  it('Complete onboarding', async () => {
    await vpn.waitForQuery(queries.screenOnboarding.SCREEN.visible());
    await vpn.waitForQuery(queries.screenOnboarding.ONBOARDING_VIEW.visible());

    //DATA SLIDE
    //Do some checks on the initial state of the data slide
    await vpn.waitForQuery(queries.screenOnboarding.DATA_SLIDE.visible());
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.ONBOARDING_VIEW, 'currentIndex'), 0);
    assert.equal(await vpn.getSetting('onboardingStep'), 0);
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.DATA_TOGGLE, 'checked'), 'false');
    assert.equal(await vpn.getSetting('gleanEnabled'), true);

    //Check data collection toggle
    await vpn.waitForQueryAndClick(queries.screenOnboarding.DATA_TOGGLE.visible());
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.DATA_TOGGLE, 'checked'), 'true');

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
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.PRIVACY_BLOCK_ADS_TOGGLE, 'checked'), 'false');
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.PRIVACY_BLOCK_TRACKERS_TOGGLE, 'checked'), 'false');
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.PRIVACY_BLOCK_MALWARE_TOGGLE, 'checked'), 'false');
    assert.equal(await vpn.getSetting('dnsProviderFlags'), 0);

    //Clicks block ads toggle
    await vpn.waitForQueryAndClick(queries.screenOnboarding.PRIVACY_BLOCK_ADS_TOGGLE.visible());
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.PRIVACY_BLOCK_ADS_TOGGLE, 'checked'), 'true');
    assert.equal(await vpn.getSetting('dnsProviderFlags'), 2);

    //Clicks block trackers toggle
    await vpn.waitForQueryAndClick(queries.screenOnboarding.PRIVACY_BLOCK_TRACKERS_TOGGLE.visible());
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.PRIVACY_BLOCK_TRACKERS_TOGGLE, 'checked'), 'true');
    assert.equal(await vpn.getSetting('dnsProviderFlags'), 6);

    //Clicks block trackers toggle
    await vpn.waitForQueryAndClick(queries.screenOnboarding.PRIVACY_BLOCK_MALWARE_TOGGLE.visible());
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.PRIVACY_BLOCK_MALWARE_TOGGLE, 'checked'), 'true');
    //dnsProviderFlags is a bitfield mapping of hex flags that can handle bitwise operations to combine multiple flags
    //here, we combine blockAds (0x02), blockTrackers (0x04), and blockMalware (0x08) which totals to 0xE or 14 in decimal
    assert.equal(await vpn.getSetting('dnsProviderFlags'), 14);

    //Test back button
    await vpn.waitForQueryAndClick(queries.screenOnboarding.PRIVACY_BACK_BUTTON.visible());
    await vpn.waitForQuery(queries.screenOnboarding.STEP_NAV_STACK_VIEW.ready());
    await vpn.waitForQuery(queries.screenOnboarding.DATA_SLIDE.visible());
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.ONBOARDING_VIEW, 'currentIndex'), 0);

    //Ensure all selections on previous slide are saved
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.DATA_TOGGLE, 'checked'), 'true');

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

    //Switch between toggle buttons based on users platform
    if (await vpn.getQueryProperty(queries.screenOnboarding.DEVICES_DEVICE_TYPE_TOGGLE.visible(), 'selectedIndex') === 0) {
      //[Android, Windows, Linux] Starting with Android - switch to iOS and back to Android
      await vpn.waitForQueryAndClick(queries.screenOnboarding.DEVICES_TOGGLE_BTN_IOS.visible());
      await vpn.waitForQuery(queries.screenOnboarding.DEVICES_APP_STORE_QRCODE.visible());
      await vpn.waitForQueryAndClick(queries.screenOnboarding.DEVICES_TOGGLE_BTN_ANDROID.visible());
      await vpn.waitForQuery(queries.screenOnboarding.DEVICES_PLAY_STORE_QRCODE.visible());
    }
    else {
      //[iOS, macOS] Starting with iOS - switch to Android and back to iOS
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
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.PRIVACY_BLOCK_ADS_TOGGLE, 'checked'), 'true');
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.PRIVACY_BLOCK_TRACKERS_TOGGLE, 'checked'), 'true');
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.PRIVACY_BLOCK_MALWARE_TOGGLE, 'checked'), 'true');

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
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.START_START_AT_BOOT_TOGGLE, 'checked'), 'false');
    assert.equal(await vpn.getSetting('startAtBoot'), false);

    //Check start at boot toggle
    await vpn.waitForQueryAndClick(queries.screenOnboarding.START_START_AT_BOOT_TOGGLE.visible());
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.START_START_AT_BOOT_TOGGLE, 'checked'), 'true');
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
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.START_START_AT_BOOT_TOGGLE, 'checked'), 'true');
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
    assert.equal(await vpn.getQueryProperty(queries.screenSettings.privacyView.BLOCK_ADS_TOGGLE, 'checked'), 'true');
    assert.equal(await vpn.getQueryProperty(queries.screenSettings.privacyView.BLOCK_TRACKERS_TOGGLE, 'checked'), 'true');
    assert.equal(await vpn.getQueryProperty(queries.screenSettings.privacyView.BLOCK_MALWARE_TOGGLE, 'checked'), 'true');
    assert.equal(await vpn.getSetting('dnsProviderFlags'), 14);
    await vpn.waitForQueryAndClick(queries.screenSettings.BACK.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    //Check data collection and start at boot
    await vpn.waitForQueryAndClick(queries.screenSettings.APP_PREFERENCES.visible());
    assert.equal(await vpn.getQueryProperty(queries.screenSettings.appPreferencesView.DATA_COLLECTION_TOGGLE, 'checked'), 'true');
    assert.equal(await vpn.getSetting('gleanEnabled'), true);
    assert.equal(await vpn.getQueryProperty(queries.screenSettings.appPreferencesView.START_AT_BOOT_TOGGLE, 'checked'), 'true');
    assert.equal(await vpn.getSetting('startAtBoot'), true);

  });

  // Tests restoring onboarding to current step after quitting
  it('Quitting app during onboarding', async () => {
    // Skip WASM because this test does a lot of quitting and re-launching
    if (this.ctx.wasm) {
      return;
    }

    await vpn.waitForQuery(queries.screenOnboarding.SCREEN.visible());
    await vpn.waitForQuery(queries.screenOnboarding.ONBOARDING_VIEW.visible());

    //Ensure we are on the first onboarding step
    await vpn.waitForQuery(queries.screenOnboarding.DATA_SLIDE.visible());
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.ONBOARDING_VIEW, 'currentIndex'), 0);
    assert.equal(await vpn.getSetting('onboardingStep'), 0);

    //Quit and relaunch the app
    await vpn.quit();
    await setup.startAndConnect();

    //Ensure we start where we left off
    await vpn.waitForQuery(queries.screenOnboarding.DATA_SLIDE.visible());
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.ONBOARDING_VIEW, 'currentIndex'), 0);
    assert.equal(await vpn.getSetting('onboardingStep'), 0);

    //Proceed to second onboarding step
    await vpn.waitForQuery(queries.screenOnboarding.STEP_NAV_STACK_VIEW.ready());
    await vpn.waitForQueryAndClick(queries.screenOnboarding.DATA_NEXT_BUTTON.visible());
    await vpn.waitForQuery(queries.screenOnboarding.ONBOARDING_VIEW.prop('currentIndex', 1));

    //Quit and relaunch the app
    await vpn.quit();
    await setup.startAndConnect();

    //Ensure we start where we left off
    await vpn.waitForQuery(queries.screenOnboarding.PRIVACY_SLIDE.visible());
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.ONBOARDING_VIEW, 'currentIndex'), 1);
    assert.equal(await vpn.getSetting('onboardingStep'), 1);

    //Proceed to third onboarding step
    await vpn.waitForQuery(queries.screenOnboarding.STEP_NAV_STACK_VIEW.ready());
    await vpn.waitForQueryAndClick(queries.screenOnboarding.PRIVACY_NEXT_BUTTON.visible());
    await vpn.waitForQuery(queries.screenOnboarding.ONBOARDING_VIEW.prop('currentIndex', 2));

    //Quit and relaunch the app
    await vpn.quit();
    await setup.startAndConnect();

    //Ensure we start where we left off
    await vpn.waitForQuery(queries.screenOnboarding.DEVICES_SLIDE.visible());
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.ONBOARDING_VIEW, 'currentIndex'), 2);
    assert.equal(await vpn.getSetting('onboardingStep'), 2);

    //Proceed to fourth/final onboarding step
    await vpn.waitForQuery(queries.screenOnboarding.STEP_NAV_STACK_VIEW.ready());
    await vpn.waitForQueryAndClick(queries.screenOnboarding.DEVICES_NEXT_BUTTON.visible());
    await vpn.waitForQuery(queries.screenOnboarding.ONBOARDING_VIEW.prop('currentIndex', 3));

    //Quit and relaunch the app
    await vpn.quit();
    await setup.startAndConnect();

    //Ensure we start where we left off
    await vpn.waitForQuery(queries.screenOnboarding.START_SLIDE.visible());
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.ONBOARDING_VIEW, 'currentIndex'), 3);
    assert.equal(await vpn.getSetting('onboardingStep'), 3);
  });

  it('Complete onboarding with data collection disabled', async () => {

    //Ensure we are in onboarding
    await vpn.waitForQuery(queries.screenOnboarding.DATA_SLIDE.visible());

    //Ensure that the data collection toggle state is bound to onboardingDataCollectionEnabled, which defaults to false
    //and that this toggles state does not affect gleanEnabled
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.DATA_TOGGLE, 'checked'), 'false');
    assert.equal(await vpn.getSetting('onboardingDataCollectionEnabled'), false);
    assert.equal(await vpn.getSetting('gleanEnabled'), true);

    //Complete onboarding and get to the home screen
    await completeOnboarding();

    //Ensure glean is disabled
    assert.equal(await vpn.getSetting('gleanEnabled'), false);
  });

  it('Complete onboarding with data collection enabled', async () => {

    //Ensure we are in onboarding
    await vpn.waitForQuery(queries.screenOnboarding.DATA_SLIDE.visible());

    //Ensure that the data collection toggle state remains bound to onboardingDataCollectionEnabled when clicked
    //and that this toggles state does not affect gleanEnabled
    await vpn.waitForQueryAndClick(queries.screenOnboarding.DATA_TOGGLE.visible());
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.DATA_TOGGLE, 'checked'), 'true');
    assert.equal(await vpn.getSetting('onboardingDataCollectionEnabled'), true);
    assert.equal(await vpn.getSetting('gleanEnabled'), true);

    //Complete onboarding and get to the home screen
    await completeOnboarding();

    //Ensure glean is enabled
    assert.equal(await vpn.getSetting('gleanEnabled'), true);
  });

  it('Data collection toggle state persists across app sessions', async () => {
    // Skip WASM because this test involves quitting and relaunching
    if (this.ctx.wasm) {
      return;
    }
    //Ensure we are in onboarding
    await vpn.waitForQuery(queries.screenOnboarding.DATA_SLIDE.visible());

    //Click the data collection toggle to set onboardingDataCollectionEnabled to true
    await vpn.waitForQueryAndClick(queries.screenOnboarding.DATA_TOGGLE.visible());

    //Quit and relaunch the app
    await vpn.quit();
    await setup.startAndConnect();

    //Ensure we are still in onboarding
    await vpn.waitForQuery(queries.screenOnboarding.DATA_SLIDE.visible());

    //Ensure that the data collection toggle state remains bound to onboardingDataCollectionEnabled upon relaunch
    //and that this toggles state does not affect gleanEnabled
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.DATA_TOGGLE, 'checked'), 'true');
    assert.equal(await vpn.getSetting('onboardingDataCollectionEnabled'), true);
    assert.equal(await vpn.getSetting('gleanEnabled'), true);

    //Ensure the data collection toggle is checked and onboardingDataCollectionEnabled is set to true
    assert.equal(await vpn.getQueryProperty(queries.screenOnboarding.DATA_TOGGLE, 'checked'), 'true');
    assert.equal(await vpn.getSetting('onboardingDataCollectionEnabled'), true);
  });

  describe('initialize onboarding telemetry tests', () => {
    // No Glean on WASM.
    if(vpn.runningOnWasm()) {
      return;
    }

    const dataScreenTelemetryId = "data_collection";
    const privacyScreenTelemetryId = "get_more_privacy";
    const devicesScreenTelemetryId = "install_on_5_devices";
    const startScreenTelemetryId = "connect_on_startup";

    it('Onboarding started event is recorded', async () => {
      await vpn.waitForQuery(queries.screenOnboarding.ONBOARDING_VIEW.visible());
      let onboardingStartedEvents = await vpn.gleanTestGetValue("outcome", "onboardingStarted", "main");
      assert.equal(onboardingStartedEvents.length, 1);

      //Quit and relaunch the app
      await vpn.quit();
      await setup.startAndConnect();

      //Ensure the event is only recorded the first time it is shown (glean store is reset on re-launch)
      onboardingStartedEvents = await vpn.gleanTestGetValue("outcome", "onboardingStarted", "main");
      assert.equal(onboardingStartedEvents.length, 0);
    });

    it('Onboarding completed event is recorded', async () => {
      //Make sure data collection is enabled so our metrics don't get deleted after we complete onboarding
      await vpn.setSetting('onboardingDataCollectionEnabled', true);

      await vpn.waitForQuery(queries.screenOnboarding.ONBOARDING_VIEW.visible());

      await completeOnboarding()

      let onboardingCompletedEvents = await vpn.gleanTestGetValue("outcome", "onboardingCompleted", "main");
      assert.equal(onboardingCompletedEvents.length, 1);
    });

    it('Onboarding slide impression events are recorded', async () => {
      //Verify that dataCollectionScreen event is recorded
      const dataCollectionScreenEvents = await vpn.gleanTestGetValue("impression", "dataCollectionScreen", "main");
      assert.equal(dataCollectionScreenEvents.length, 1);
      const dataCollectionScreenEventsExtras = dataCollectionScreenEvents[0].extra;
      assert.equal(dataScreenTelemetryId, dataCollectionScreenEventsExtras.screen);

      await advanceToSlide(1)

      //Verify that getMorePrivacyScreen event is recorded
      const getMorePrivacyScreenEvents = await vpn.gleanTestGetValue("impression", "getMorePrivacyScreen", "main");
      assert.equal(getMorePrivacyScreenEvents.length, 1);
      const getMorePrivacyScreenEventsExtras = getMorePrivacyScreenEvents[0].extra;
      assert.equal(privacyScreenTelemetryId, getMorePrivacyScreenEventsExtras.screen);

      await advanceToSlide(2)

      //Verify that installOn5DevicesScreen event is recorded
      const installOn5DevicesScreenEvents = await vpn.gleanTestGetValue("impression", "installOn5DevicesScreen", "main");
      assert.equal(installOn5DevicesScreenEvents.length, 1);
      const installOn5DevicesScreenEventsExtras = installOn5DevicesScreenEvents[0].extra;
      assert.equal(devicesScreenTelemetryId, installOn5DevicesScreenEventsExtras.screen);

      await advanceToSlide(3)

      //Verify that connectOnStartupScreen event is recorded
      const connectOnStartupScreenEvents = await vpn.gleanTestGetValue("impression", "connectOnStartupScreen", "main");
      assert.equal(connectOnStartupScreenEvents.length, 1);
      const connectOnStartupScreenEventsExtras = connectOnStartupScreenEvents[0].extra;
      assert.equal(startScreenTelemetryId, connectOnStartupScreenEventsExtras.screen);
    });

    it('Progress bar button events are recorded', async () => {
      //We call these more than once throughout the test, lets keep track of the aggregate and avoid resetting glean
      let dataUseSelectedEventCounter = 0
      let morePrivacySelectedEventCounter = 0

      await vpn.waitForQuery(queries.screenOnboarding.DATA_SLIDE.visible());

      //Ensure telemetry is not generated by clicking on the current step
      await vpn.waitForQueryAndClick(queries.screenOnboarding.STEP_PROG_BAR_DATA_BUTTON.visible());
      let dataUseSelectedEvents = await vpn.gleanTestGetValue("interaction", "dataUseSelected", "main");
      assert.equal(dataUseSelectedEvents.length, 0);


      //Advance to privacy slide
      await advanceToSlide(1)

      //Ensure telemetry is not generated by clicking on the current step
      await vpn.waitForQueryAndClick(queries.screenOnboarding.STEP_PROG_BAR_PRIVACY_BUTTON.visible());
      let morePrivacySelectedEvents = await vpn.gleanTestGetValue("interaction", "morePrivacySelected", "main");
      assert.equal(morePrivacySelectedEvents.length, 0);

      //Ensure dataUseSelected event is recorded from the privacy screen
      await vpn.waitForQueryAndClick(queries.screenOnboarding.STEP_PROG_BAR_DATA_BUTTON.visible());
      dataUseSelectedEventCounter++
      dataUseSelectedEvents = await vpn.gleanTestGetValue("interaction", "dataUseSelected", "main");
      assert.equal(dataUseSelectedEvents.length, dataUseSelectedEventCounter);
      let dataUseSelectedEventsExtras = dataUseSelectedEvents[dataUseSelectedEventCounter - 1].extra;
      assert.equal(privacyScreenTelemetryId, dataUseSelectedEventsExtras.screen);


      //Advance to devices slide
      await advanceToSlide(2)

      //Ensure telemetry is not generated by clicking on the current step
      await vpn.waitForQueryAndClick(queries.screenOnboarding.STEP_PROG_BAR_DEVICES_BUTTON.visible());
      let addDevicesSelectedEvents = await vpn.gleanTestGetValue("interaction", "addDevicesSelected", "main");
      assert.equal(addDevicesSelectedEvents.length, 0);

      //Ensure morePrivacySelected event is recorded from the devices screen
      await vpn.waitForQueryAndClick(queries.screenOnboarding.STEP_PROG_BAR_PRIVACY_BUTTON.visible());
      morePrivacySelectedEventCounter++
      morePrivacySelectedEvents = await vpn.gleanTestGetValue("interaction", "morePrivacySelected", "main");
      assert.equal(morePrivacySelectedEvents.length, morePrivacySelectedEventCounter);
      let morePrivacySelectedEventsExtras = morePrivacySelectedEvents[morePrivacySelectedEventCounter - 1].extra;
      assert.equal(devicesScreenTelemetryId, morePrivacySelectedEventsExtras.screen);

      await advanceToSlide(2)

      //Ensure dataUseSelected event is recorded from the devices screen
      await vpn.waitForQueryAndClick(queries.screenOnboarding.STEP_PROG_BAR_DATA_BUTTON.visible());
      dataUseSelectedEventCounter++
      dataUseSelectedEvents = await vpn.gleanTestGetValue("interaction", "dataUseSelected", "main");
      assert.equal(dataUseSelectedEvents.length, dataUseSelectedEventCounter);
      dataUseSelectedEventsExtras = dataUseSelectedEvents[dataUseSelectedEventCounter - 1].extra;
      assert.equal(devicesScreenTelemetryId, dataUseSelectedEventsExtras.screen);


      //Advance to start slide
      await advanceToSlide(3)

      //Start slide progress bar button does not have telemetry (because it never does anything)
      //so we don't need to ensure it doesn't generate telemetry

      //Ensure addDevicesSelected event is recorded from the start screen
      await vpn.waitForQueryAndClick(queries.screenOnboarding.STEP_PROG_BAR_DEVICES_BUTTON.visible());
      addDevicesSelectedEvents = await vpn.gleanTestGetValue("interaction", "addDevicesSelected", "main");
      assert.equal(addDevicesSelectedEvents.length, 1);
      let addDevicesSelectedEventsExtras = addDevicesSelectedEvents[0].extra;
      assert.equal(startScreenTelemetryId, addDevicesSelectedEventsExtras.screen);

      await advanceToSlide(3)

      //Ensure morePrivacySelected event is recorded from the start screen
      await vpn.waitForQueryAndClick(queries.screenOnboarding.STEP_PROG_BAR_PRIVACY_BUTTON.visible());
      morePrivacySelectedEventCounter++
      morePrivacySelectedEvents = await vpn.gleanTestGetValue("interaction", "morePrivacySelected", "main");
      assert.equal(morePrivacySelectedEvents.length, morePrivacySelectedEventCounter);
      morePrivacySelectedEventsExtras = morePrivacySelectedEvents[morePrivacySelectedEventCounter - 1].extra;
      assert.equal(startScreenTelemetryId, morePrivacySelectedEventsExtras.screen);

      await advanceToSlide(3)

      //Ensure dataUseSelected event is recorded from the start screen
      await vpn.waitForQueryAndClick(queries.screenOnboarding.STEP_PROG_BAR_DATA_BUTTON.visible());
      dataUseSelectedEventCounter++
      dataUseSelectedEvents = await vpn.gleanTestGetValue("interaction", "dataUseSelected", "main");
      assert.equal(dataUseSelectedEvents.length, dataUseSelectedEventCounter);
      dataUseSelectedEventsExtras = dataUseSelectedEvents[dataUseSelectedEventCounter - 1].extra;
      assert.equal(startScreenTelemetryId, dataUseSelectedEventsExtras.screen);

    });

    it('Data slide events are recorded', async () => {
      await vpn.waitForQuery(queries.screenOnboarding.DATA_SLIDE.visible());

      //Verify that privacyNoticedSelected event is recorded
      await vpn.waitForQueryAndClick(queries.screenOnboarding.DATA_PRIVACY_LINK.visible());
      const privacyNoticeSelectedEvents = await vpn.gleanTestGetValue("interaction", "privacyNoticeSelected", "main");
      assert.equal(privacyNoticeSelectedEvents.length, 1);
      const privacyNoticeSelectedEventsExtras = privacyNoticeSelectedEvents[0].extra;
      assert.equal(dataScreenTelemetryId, privacyNoticeSelectedEventsExtras.screen);

      //Used to wipe previous continueSelected events that were recorded
      //during authentication before testing in onboarding
      await vpn.gleanTestReset();

      //Verify that continueSelected event is recorded
      await vpn.waitForQueryAndClick(queries.screenOnboarding.DATA_NEXT_BUTTON.visible());
      const continueSelectedEvents = await vpn.gleanTestGetValue("interaction", "continueSelected", "main");
      assert.equal(continueSelectedEvents.length, 1);
      const continueSelectedEventsExtras = continueSelectedEvents[0].extra;
      assert.equal(dataScreenTelemetryId, continueSelectedEventsExtras.screen);
    });

    it('Privacy slide events are recorded', async () => {
      await advanceToSlide(1);

      await vpn.waitForQuery(queries.screenOnboarding.PRIVACY_SLIDE.visible());

      //Verify that blockAdsEnabled event is recorded
      await vpn.waitForQueryAndClick(queries.screenOnboarding.PRIVACY_BLOCK_ADS_TOGGLE.visible());
      const blockAdsEnabledEvents = await vpn.gleanTestGetValue("interaction", "blockAdsEnabled", "main");
      assert.equal(blockAdsEnabledEvents.length, 1);
      const blockAdsEnabledEventsExtras = blockAdsEnabledEvents[0].extra;
      assert.equal(privacyScreenTelemetryId, blockAdsEnabledEventsExtras.screen);

      //Verify that blockAdsDisabled event is recorded
      await vpn.waitForQueryAndClick(queries.screenOnboarding.PRIVACY_BLOCK_ADS_TOGGLE.visible());
      const blockAdsDisabledEvents = await vpn.gleanTestGetValue("interaction", "blockAdsDisabled", "main");
      assert.equal(blockAdsDisabledEvents.length, 1);
      const blockAdsDisabledEventsExtras = blockAdsDisabledEvents[0].extra;
      assert.equal(privacyScreenTelemetryId, blockAdsDisabledEventsExtras.screen);

      //Verify that blockTrackersEnabled event is recorded
      await vpn.waitForQueryAndClick(queries.screenOnboarding.PRIVACY_BLOCK_TRACKERS_TOGGLE.visible());
      const blockTrackersEnabledEvents = await vpn.gleanTestGetValue("interaction", "blockTrackersEnabled", "main");
      assert.equal(blockTrackersEnabledEvents.length, 1);
      const blockTrackersEnabledEventsExtras = blockTrackersEnabledEvents[0].extra;
      assert.equal(privacyScreenTelemetryId, blockTrackersEnabledEventsExtras.screen);

      //Verify that blockTrackersDisabled event is recorded
      await vpn.waitForQueryAndClick(queries.screenOnboarding.PRIVACY_BLOCK_TRACKERS_TOGGLE.visible());
      const blockTrackersDisabledEvents = await vpn.gleanTestGetValue("interaction", "blockTrackersDisabled", "main");
      assert.equal(blockTrackersDisabledEvents.length, 1);
      const blockTrackersDisabledEventsExtras = blockTrackersDisabledEvents[0].extra;
      assert.equal(privacyScreenTelemetryId, blockTrackersDisabledEventsExtras.screen);

      //Verify that blockMalwareEnabled event is recorded
      await vpn.waitForQueryAndClick(queries.screenOnboarding.PRIVACY_BLOCK_MALWARE_TOGGLE.visible());
      const blockMalwareEnabledEvents = await vpn.gleanTestGetValue("interaction", "blockMalwareEnabled", "main");
      assert.equal(blockMalwareEnabledEvents.length, 1);
      const blockMalwareEnabledEventsExtras = blockMalwareEnabledEvents[0].extra;
      assert.equal(privacyScreenTelemetryId, blockMalwareEnabledEventsExtras.screen);

      //Verify that blockMalwareDisabled event is recorded
      await vpn.waitForQueryAndClick(queries.screenOnboarding.PRIVACY_BLOCK_MALWARE_TOGGLE.visible());
      const blockMalwareDisabledEvents = await vpn.gleanTestGetValue("interaction", "blockMalwareDisabled", "main");
      assert.equal(blockMalwareDisabledEvents.length, 1);
      const blockMalwareDisabledEventsExtras = blockMalwareDisabledEvents[0].extra;
      assert.equal(privacyScreenTelemetryId, blockMalwareDisabledEventsExtras.screen);

      //Verify that goBackSelected event is recorded
      await vpn.waitForQueryAndClick(queries.screenOnboarding.PRIVACY_BACK_BUTTON.visible());
      const goBackSelectedEvents = await vpn.gleanTestGetValue("interaction", "goBackSelected", "main");
      assert.equal(goBackSelectedEvents.length, 1);
      const goBackSelectedEventsExtras = goBackSelectedEvents[0].extra;
      assert.equal(privacyScreenTelemetryId, goBackSelectedEventsExtras.screen);

      await advanceToSlide(1)

      //Used to wipe previous continueSelected events that were recorded
      //during authentication before testing in onboarding
      await vpn.gleanTestReset();

      //Verify that continueSelected event is recorded
      await vpn.waitForQueryAndClick(queries.screenOnboarding.PRIVACY_NEXT_BUTTON.visible());
      const continueSelectedEvents = await vpn.gleanTestGetValue("interaction", "continueSelected", "main");
      assert.equal(continueSelectedEvents.length, 1);
      const continueSelectedEventsExtras = continueSelectedEvents[0].extra;
      assert.equal(privacyScreenTelemetryId, continueSelectedEventsExtras.screen);
    });

    it('Devices slide events are recorded', async () => {
      await advanceToSlide(2);

      await vpn.waitForQuery(queries.screenOnboarding.DEVICES_SLIDE.visible());

      if (await vpn.getQueryProperty(queries.screenOnboarding.DEVICES_DEVICE_TYPE_TOGGLE.visible(), 'selectedIndex') == 0) {
        //[Android, Windows, Linux] Starting with Android - switch to iOS and back to Android
        //Verify that showIosQrSelected event is recorded
        await vpn.waitForQueryAndClick(queries.screenOnboarding.DEVICES_TOGGLE_BTN_IOS.visible());
        const showIosQrSelectedEvents = await vpn.gleanTestGetValue("interaction", "showIosQrSelected", "main");
        assert.equal(showIosQrSelectedEvents.length, 1);
        const showIosQrSelectedEventsExtras = showIosQrSelectedEvents[0].extra;
        assert.equal(devicesScreenTelemetryId, showIosQrSelectedEventsExtras.screen);

        //Verify that showAndroidQrSelected event is recorded
        await vpn.waitForQueryAndClick(queries.screenOnboarding.DEVICES_TOGGLE_BTN_ANDROID.visible());
        const showAndroidQrSelectedEvents = await vpn.gleanTestGetValue("interaction", "showAndroidQrSelected", "main");
        assert.equal(showAndroidQrSelectedEvents.length, 1);
        const showAndroidQrSelectedEventsExtras = showAndroidQrSelectedEvents[0].extra;
        assert.equal(devicesScreenTelemetryId, showAndroidQrSelectedEventsExtras.screen);

      }
      else {
        //[iOS, macOS] Starting with iOS - switch to Android and back to iOS
        //Verify that showAndroidQrSelected event is recorded
        await vpn.waitForQueryAndClick(queries.screenOnboarding.DEVICES_TOGGLE_BTN_ANDROID.visible());
        const showAndroidQrSelectedEvents = await vpn.gleanTestGetValue("interaction", "showAndroidQrSelected", "main");
        assert.equal(showAndroidQrSelectedEvents.length, 1);
        const showAndroidQrSelectedEventsExtras = showAndroidQrSelectedEvents[0].extra;
        assert.equal(devicesScreenTelemetryId, showAndroidQrSelectedEventsExtras.screen);

        //Verify that showIosQrSelected event is recorded
        await vpn.waitForQueryAndClick(queries.screenOnboarding.DEVICES_TOGGLE_BTN_IOS.visible());
        const showIosQrSelectedEvents = await vpn.gleanTestGetValue("interaction", "showIosQrSelected", "main");
        assert.equal(showIosQrSelectedEvents.length, 1);
        const showIosQrSelectedEventsExtras = showIosQrSelectedEvents[0].extra;
        assert.equal(devicesScreenTelemetryId, showIosQrSelectedEventsExtras.screen);
      }

      //Verify that goBackSelected event is recorded
      await vpn.waitForQueryAndClick(queries.screenOnboarding.DEVICES_BACK_BUTTON.visible());
      const goBackSelectedEvents = await vpn.gleanTestGetValue("interaction", "goBackSelected", "main");
      assert.equal(goBackSelectedEvents.length, 1);
      const goBackSelectedEventsExtras = goBackSelectedEvents[0].extra;
      assert.equal(devicesScreenTelemetryId, goBackSelectedEventsExtras.screen);

      await advanceToSlide(2)

      //Used to wipe previous continueSelected events that were recorded
      //during authentication before testing in onboarding
      await vpn.gleanTestReset();

      //Verify that continueSelected event is recorded
      await vpn.waitForQueryAndClick(queries.screenOnboarding.DEVICES_NEXT_BUTTON.visible());
      const continueSelectedEvents = await vpn.gleanTestGetValue("interaction", "continueSelected", "main");
      assert.equal(continueSelectedEvents.length, 1);
      const continueSelectedEventsExtras = continueSelectedEvents[0].extra;
      assert.equal(devicesScreenTelemetryId, continueSelectedEventsExtras.screen);
    });

    it('Desktop start slide events are recorded', async () => {
      await advanceToSlide(3);

      await vpn.waitForQuery(queries.screenOnboarding.START_SLIDE.visible());

      //Verify that connectOnStartupEnabled event is recorded
      await vpn.waitForQueryAndClick(queries.screenOnboarding.START_START_AT_BOOT_TOGGLE.visible());
      const connectOnStartupEnabledEvents = await vpn.gleanTestGetValue("interaction", "connectOnStartupEnabled", "main");
      assert.equal(connectOnStartupEnabledEvents.length, 1);
      const connectOnStartupEnabledEventsExtras = connectOnStartupEnabledEvents[0].extra;
      assert.equal(startScreenTelemetryId, connectOnStartupEnabledEventsExtras.screen);

      //Verify that connectOnStartupDisabled event is recorded
      await vpn.waitForQueryAndClick(queries.screenOnboarding.START_START_AT_BOOT_TOGGLE.visible());
      const connectOnStartupDisabledEvents = await vpn.gleanTestGetValue("interaction", "connectOnStartupDisabled", "main");
      assert.equal(connectOnStartupDisabledEvents.length, 1);
      const connectOnStartupDisabledEventsExtras = connectOnStartupDisabledEvents[0].extra;
      assert.equal(startScreenTelemetryId, connectOnStartupDisabledEventsExtras.screen);

      //Verify that goBackSelected event is recorded
      await vpn.waitForQueryAndClick(queries.screenOnboarding.START_BACK_BUTTON.visible());
      const goBackSelectedEvents = await vpn.gleanTestGetValue("interaction", "goBackSelected", "main");
      assert.equal(goBackSelectedEvents.length, 1);
      const goBackSelectedEventsExtras = goBackSelectedEvents[0].extra;
      assert.equal(startScreenTelemetryId, goBackSelectedEventsExtras.screen);

      await advanceToSlide(3)

      //Used to wipe previous continueSelected events that were recorded
      //during authentication before testing in onboarding
      await vpn.gleanTestReset();

      //For the next test we need data collection enabled due to a race condition between recording
      //the getStartedSelected event and data collection being disabled at onboarding completion
      await vpn.setSetting('onboardingDataCollectionEnabled', true)

      //Verify that getStartedSelected event is recorded
      await vpn.waitForQueryAndClick(queries.screenOnboarding.START_NEXT_BUTTON.visible());
      const getStartedSelectedEvents = await vpn.gleanTestGetValue("interaction", "getStartedSelected", "main");
      assert.equal(getStartedSelectedEvents.length, 1);
      const getStartedSelectedEventsExtras = getStartedSelectedEvents[0].extra;
      assert.equal(startScreenTelemetryId, getStartedSelectedEventsExtras.screen);
    });

    //TODO (VPN-4784, VPN-4783): This cannot be tested until we are able to run
    //functional tests in mobile. This slide and its telemetry are mobile-only
    it.skip('Mobile start slide events are recorded')

    //TODO (VPN-4784, VPN-4783): This cannot be tested until we are able to run
    //functional tests in mobile. This slide and its telemetry are mobile-only
    it.skip('Mobile network permission granted event is recorded')

    //TODO (VPN-4784, VPN-4783): This cannot be tested until we are able to run
    //functional tests in mobile. This slide and its telemetry are mobile-only
    it.skip('Mobile network permission denied event is recorded')
    });
});