/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

 
import { equal } from 'assert';
import { screenOnboarding, screenHome, navBar, screenSettings } from './queries.js';
import { flipFeatureOn, getSetting, authenticateInApp, waitForQuery, waitForQueryAndClick, getQueryProperty, waitForCondition, getLastUrl, quit, runningOnWasm, gleanTestGetValue, setSetting, gleanTestReset } from './helper.js';
import { startAndConnect } from './setupVpn.js';

describe('Onboarding', function() {
  this.timeout(120000);

  beforeEach(async () => {
    await flipFeatureOn("newOnboarding");
    await setSetting('onboardingCompleted', false);
    await setSetting('onboardingStep',0);
    await authenticateInApp();
  });

 async function completeOnboarding() {
   await waitForQuery(screenOnboarding.STEP_NAV_STACK_VIEW.ready());
   switch(await getSetting('onboardingStep')) {
   case 0:
    await waitForQueryAndClick(screenOnboarding.DATA_NEXT_BUTTON.visible());
    await waitForQuery(screenOnboarding.STEP_NAV_STACK_VIEW.ready());
   case 1:
    await waitForQueryAndClick(screenOnboarding.PRIVACY_NEXT_BUTTON.visible());
    await waitForQuery(screenOnboarding.STEP_NAV_STACK_VIEW.ready());
   case 2:
    await waitForQueryAndClick(screenOnboarding.DEVICES_NEXT_BUTTON.visible());
    await waitForQuery(screenOnboarding.STEP_NAV_STACK_VIEW.ready());
   case 3:
    await waitForQueryAndClick(screenOnboarding.START_NEXT_BUTTON.visible());
    await waitForQuery(screenHome.SCREEN.visible());
    equal(await getSetting('onboardingCompleted'), true);
  default:
    break;
   }
 }

  async function advanceToSlide(slide) {
   await waitForQuery(screenOnboarding.STEP_NAV_STACK_VIEW.ready());
   const currentSlide = await getSetting('onboardingStep')
   switch(currentSlide) {
   case 0:
    await waitForQueryAndClick(screenOnboarding.DATA_NEXT_BUTTON.visible());
    await waitForQuery(screenOnboarding.STEP_NAV_STACK_VIEW.ready());
    if (await getSetting('onboardingStep') === slide) break;
   case 1:
    await waitForQueryAndClick(screenOnboarding.PRIVACY_NEXT_BUTTON.visible());
    await waitForQuery(screenOnboarding.STEP_NAV_STACK_VIEW.ready());
    if (await getSetting('onboardingStep') === slide) break;
   case 2:
    await waitForQueryAndClick(screenOnboarding.DEVICES_NEXT_BUTTON.visible());
    await waitForQuery(screenOnboarding.STEP_NAV_STACK_VIEW.ready());
    if (await getSetting('onboardingStep') === slide) break;
  default:
    break;
   }
 }

  //Tests pretty much all of the new onboarding, including all slides, controls, navigation, and ensuring settings persist into StateMain
  it('Complete onboarding', async () => {
    await waitForQuery(screenOnboarding.SCREEN.visible());
    await waitForQuery(screenOnboarding.ONBOARDING_VIEW.visible());

    //DATA SLIDE
    //Do some checks on the initial state of the data slide
    await waitForQuery(screenOnboarding.DATA_SLIDE.visible());
    equal(await getQueryProperty(screenOnboarding.ONBOARDING_VIEW, 'currentIndex'), 0);
    equal(await getSetting('onboardingStep'), 0);
    equal(await getQueryProperty(screenOnboarding.DATA_CHECKBOX_ROW, 'isChecked'), 'false');
    equal(await getSetting('gleanEnabled'), true);

    //Check data collection checkbox
    await waitForQueryAndClick(screenOnboarding.DATA_CHECKBOX.visible());
    equal(await getQueryProperty(screenOnboarding.DATA_CHECKBOX_ROW, 'isChecked'), 'true');

    //Check privacy link
    await waitForQueryAndClick(screenOnboarding.DATA_PRIVACY_LINK.visible());
    await waitForCondition(async () => {
      const url = await getLastUrl();
        return url.endsWith('/r/vpn/privacy');
    });

    //Proceed to privacy slide
    await waitForQueryAndClick(screenOnboarding.DATA_NEXT_BUTTON.visible());
    await waitForQuery(screenOnboarding.STEP_NAV_STACK_VIEW.ready());


    //PRIVACY SLIDE
    //Do some checks on the initial state of the privacy slide
    await waitForQuery(screenOnboarding.PRIVACY_SLIDE.visible());
    equal(await getQueryProperty(screenOnboarding.ONBOARDING_VIEW, 'currentIndex'), 1);
    equal(await getSetting('onboardingStep'), 1);
    equal(await getQueryProperty(screenOnboarding.PRIVACY_BLOCK_ADS_CHECKBOX, 'checked'), 'false');
    equal(await getQueryProperty(screenOnboarding.PRIVACY_BLOCK_TRACKERS_CHECKBOX, 'checked'), 'false');
    equal(await getQueryProperty(screenOnboarding.PRIVACY_BLOCK_MALWARE_CHECKBOX, 'checked'), 'false');
    equal(await getSetting('dnsProviderFlags'), 0);

    //Clicks block ads checkbox
    await waitForQueryAndClick(screenOnboarding.PRIVACY_BLOCK_ADS_CHECKBOX.visible());
    equal(await getQueryProperty(screenOnboarding.PRIVACY_BLOCK_ADS_CHECKBOX, 'checked'), 'true');
    equal(await getSetting('dnsProviderFlags'), 2);

    //Clicks block trackers checkbox
    await waitForQueryAndClick(screenOnboarding.PRIVACY_BLOCK_TRACKERS_CHECKBOX.visible());
    equal(await getQueryProperty(screenOnboarding.PRIVACY_BLOCK_TRACKERS_CHECKBOX, 'checked'), 'true');
    equal(await getSetting('dnsProviderFlags'), 6);

    //Clicks block trackers checkbox
    await waitForQueryAndClick(screenOnboarding.PRIVACY_BLOCK_MALWARE_CHECKBOX.visible());
    equal(await getQueryProperty(screenOnboarding.PRIVACY_BLOCK_MALWARE_CHECKBOX, 'checked'), 'true');
    //dnsProviderFlags is a bitfield mapping of hex flags that can handle bitwise operations to combine multiple flags
    //here, we combine blockAds (0x02), blockTrackers (0x04), and blockMalware (0x08) which totals to 0xE or 14 in decimal
    equal(await getSetting('dnsProviderFlags'), 14);

    //Test back button
    await waitForQueryAndClick(screenOnboarding.PRIVACY_BACK_BUTTON.visible());
    await waitForQuery(screenOnboarding.STEP_NAV_STACK_VIEW.ready());
    await waitForQuery(screenOnboarding.DATA_SLIDE.visible());
    equal(await getQueryProperty(screenOnboarding.ONBOARDING_VIEW, 'currentIndex'), 0);

    //Ensure all selections on previous slide are saved
    equal(await getQueryProperty(screenOnboarding.DATA_CHECKBOX_ROW, 'isChecked'), 'true');

    //Return to privacy slide
    await waitForQueryAndClick(screenOnboarding.DATA_NEXT_BUTTON.visible());
    await waitForQuery(screenOnboarding.STEP_NAV_STACK_VIEW.ready());

    //Test going back one slide via progress bar
    await waitForQueryAndClick(screenOnboarding.STEP_PROG_BAR_DATA_BUTTON.visible());
    await waitForQuery(screenOnboarding.STEP_NAV_STACK_VIEW.ready());

    //Proceed to devices slide
    await waitForQueryAndClick(screenOnboarding.DATA_NEXT_BUTTON.visible());
    await waitForQuery(screenOnboarding.STEP_NAV_STACK_VIEW.ready());
    await waitForQueryAndClick(screenOnboarding.PRIVACY_NEXT_BUTTON.visible());
    await waitForQuery(screenOnboarding.STEP_NAV_STACK_VIEW.ready());


    //DEVICES SLIDE
    //Do some checks on the initial state of the devices slide
    await waitForQuery(screenOnboarding.DEVICES_SLIDE.visible());
    equal(await getQueryProperty(screenOnboarding.ONBOARDING_VIEW, 'currentIndex'), 2);
    equal(await getSetting('onboardingStep'), 2);

    //Switch between toggle buttons based on users platform
    if (await getQueryProperty(screenOnboarding.DEVICES_DEVICE_TYPE_TOGGLE.visible(), 'selectedIndex') === 0) {
      //[Android, Windows, Linux] Starting with Android - switch to iOS and back to Android
      await waitForQueryAndClick(screenOnboarding.DEVICES_TOGGLE_BTN_IOS.visible());
      await waitForQuery(screenOnboarding.DEVICES_APP_STORE_QRCODE.visible());
      await waitForQueryAndClick(screenOnboarding.DEVICES_TOGGLE_BTN_ANDROID.visible());
      await waitForQuery(screenOnboarding.DEVICES_PLAY_STORE_QRCODE.visible());
    }
    else {
      //[iOS, macOS] Starting with iOS - switch to Android and back to iOS
      await waitForQueryAndClick(screenOnboarding.DEVICES_TOGGLE_BTN_ANDROID.visible());
      await waitForQuery(screenOnboarding.DEVICES_PLAY_STORE_QRCODE.visible());
      await waitForQueryAndClick(screenOnboarding.DEVICES_TOGGLE_BTN_IOS.visible());
      await waitForQuery(screenOnboarding.DEVICES_APP_STORE_QRCODE.visible());
    }

    //Test back button
    await waitForQueryAndClick(screenOnboarding.DEVICES_BACK_BUTTON.visible());
    await waitForQuery(screenOnboarding.STEP_NAV_STACK_VIEW.ready());
    await waitForQuery(screenOnboarding.PRIVACY_SLIDE.visible());
    equal(await getQueryProperty(screenOnboarding.ONBOARDING_VIEW, 'currentIndex'), 1);

    //Ensure all selections on previous slide are saved
    equal(await getQueryProperty(screenOnboarding.PRIVACY_BLOCK_ADS_CHECKBOX, 'checked'), 'true');
    equal(await getQueryProperty(screenOnboarding.PRIVACY_BLOCK_TRACKERS_CHECKBOX, 'checked'), 'true');
    equal(await getQueryProperty(screenOnboarding.PRIVACY_BLOCK_MALWARE_CHECKBOX, 'checked'), 'true');

    //Return to devices slide
    await waitForQueryAndClick(screenOnboarding.PRIVACY_NEXT_BUTTON.visible());
    await waitForQuery(screenOnboarding.STEP_NAV_STACK_VIEW.ready());

    //Test going back one slide via progress bar
    await waitForQueryAndClick(screenOnboarding.STEP_PROG_BAR_PRIVACY_BUTTON.visible());
    await waitForQuery(screenOnboarding.STEP_NAV_STACK_VIEW.ready());

    //Proceed to start slide
    await waitForQueryAndClick(screenOnboarding.PRIVACY_NEXT_BUTTON.visible());
    await waitForQuery(screenOnboarding.STEP_NAV_STACK_VIEW.ready());
    await waitForQueryAndClick(screenOnboarding.DEVICES_NEXT_BUTTON.visible());
    await waitForQuery(screenOnboarding.STEP_NAV_STACK_VIEW.ready());


    //START SLIDE
    //Do some checks on the initial state of the start slide
    await waitForQuery(screenOnboarding.START_SLIDE.visible());
    equal(await getQueryProperty(screenOnboarding.ONBOARDING_VIEW, 'currentIndex'), 3);
    equal(await getSetting('onboardingStep'), 3);
    equal(await getQueryProperty(screenOnboarding.START_START_AT_BOOT_CHECKBOX, 'checked'), 'false');
    equal(await getSetting('startAtBoot'), false);

    //Check start at boot checkbox
    await waitForQueryAndClick(screenOnboarding.START_START_AT_BOOT_CHECKBOX.visible());
    equal(await getQueryProperty(screenOnboarding.START_START_AT_BOOT_CHECKBOX, 'checked'), 'true');
    equal(await getSetting('startAtBoot'), true);

    //Test back button
    await waitForQueryAndClick(screenOnboarding.START_BACK_BUTTON.visible());
    await waitForQuery(screenOnboarding.STEP_NAV_STACK_VIEW.ready());
    await waitForQuery(screenOnboarding.DEVICES_SLIDE.visible());
    equal(await getQueryProperty(screenOnboarding.ONBOARDING_VIEW, 'currentIndex'), 2);

    //Return to start slide
    await waitForQueryAndClick(screenOnboarding.DEVICES_NEXT_BUTTON.visible());
    await waitForQuery(screenOnboarding.STEP_NAV_STACK_VIEW.ready());

    //Ensure all selections on start slide are saved
    equal(await getQueryProperty(screenOnboarding.START_START_AT_BOOT_CHECKBOX, 'checked'), 'true');
    equal(await getSetting('startAtBoot'), true);

    //Test going back one slide via progress bar
    await waitForQueryAndClick(screenOnboarding.STEP_PROG_BAR_DEVICES_BUTTON.visible());
    await waitForQuery(screenOnboarding.STEP_NAV_STACK_VIEW.ready());

    //Return to start slide
    await waitForQueryAndClick(screenOnboarding.DEVICES_NEXT_BUTTON.visible());
    await waitForQuery(screenOnboarding.STEP_NAV_STACK_VIEW.ready());

    //Test going all the way back to the first slide
    await waitForQueryAndClick(screenOnboarding.STEP_PROG_BAR_DATA_BUTTON.visible());
    await waitForQuery(screenOnboarding.STEP_NAV_STACK_VIEW.ready());

    //Proceed by completing onboarding
    await waitForQueryAndClick(screenOnboarding.DATA_NEXT_BUTTON.visible());
    await waitForQuery(screenOnboarding.STEP_NAV_STACK_VIEW.ready());
    await waitForQueryAndClick(screenOnboarding.PRIVACY_NEXT_BUTTON.visible());
    await waitForQuery(screenOnboarding.STEP_NAV_STACK_VIEW.ready());
    await waitForQueryAndClick(screenOnboarding.DEVICES_NEXT_BUTTON.visible());
    await waitForQuery(screenOnboarding.STEP_NAV_STACK_VIEW.ready());
    await waitForQueryAndClick(screenOnboarding.START_NEXT_BUTTON.visible());
    await waitForQuery(screenHome.SCREEN.visible());
    equal(await getSetting('onboardingCompleted'), true);


    //Test that changes made during onboarding persist
    await waitForQueryAndClick(navBar.SETTINGS);

    //Check privacy settings
    await waitForQueryAndClick(screenSettings.PRIVACY.visible());
    equal(await getQueryProperty(screenSettings.privacyView.BLOCK_ADS_CHECKBOX, 'checked'), 'true');
    equal(await getQueryProperty(screenSettings.privacyView.BLOCK_TRACKERS_CHECKBOX, 'checked'), 'true');
    equal(await getQueryProperty(screenSettings.privacyView.BLOCK_MALWARE_CHECKBOX, 'checked'), 'true');
    equal(await getSetting('dnsProviderFlags'), 14);
    await waitForQueryAndClick(screenSettings.BACK.visible());
    await waitForQuery(screenSettings.STACKVIEW.ready());

    //Check data collection and start at boot
    await waitForQueryAndClick(screenSettings.APP_PREFERENCES.visible());
    equal(await getQueryProperty(screenSettings.appPreferencesView.DATA_COLLECTION, 'isChecked'), 'true');
    equal(await getSetting('gleanEnabled'), true);
    equal(await getQueryProperty(screenSettings.appPreferencesView.START_AT_BOOT, 'isChecked'), 'true');
    equal(await getSetting('startAtBoot'), true);

  });

  // Tests restoring onboarding to current step after quitting
  it('Quitting app during onboarding', async () => {
    // Skip WASM because this test does a lot of quitting and re-launching
    if (this.ctx.wasm) {
      return;
    }

    await waitForQuery(screenOnboarding.SCREEN.visible());
    await waitForQuery(screenOnboarding.ONBOARDING_VIEW.visible());

    //Ensure we are on the first onboarding step
    await waitForQuery(screenOnboarding.DATA_SLIDE.visible());
    equal(await getQueryProperty(screenOnboarding.ONBOARDING_VIEW, 'currentIndex'), 0);
    equal(await getSetting('onboardingStep'), 0);

    //Quit and relaunch the app
    await quit();
    await startAndConnect();

    //Ensure we start where we left off
    await waitForQuery(screenOnboarding.DATA_SLIDE.visible());
    equal(await getQueryProperty(screenOnboarding.ONBOARDING_VIEW, 'currentIndex'), 0);
    equal(await getSetting('onboardingStep'), 0);

    //Proceed to second onboarding step
    await waitForQuery(screenOnboarding.STEP_NAV_STACK_VIEW.ready());
    await waitForQueryAndClick(screenOnboarding.DATA_NEXT_BUTTON.visible());
    await waitForQuery(screenOnboarding.ONBOARDING_VIEW.prop('currentIndex', 1));

    //Quit and relaunch the app
    await quit();
    await startAndConnect();

    //Ensure we start where we left off
    await waitForQuery(screenOnboarding.PRIVACY_SLIDE.visible());
    equal(await getQueryProperty(screenOnboarding.ONBOARDING_VIEW, 'currentIndex'), 1);
    equal(await getSetting('onboardingStep'), 1);

    //Proceed to third onboarding step
    await waitForQuery(screenOnboarding.STEP_NAV_STACK_VIEW.ready());
    await waitForQueryAndClick(screenOnboarding.PRIVACY_NEXT_BUTTON.visible());
    await waitForQuery(screenOnboarding.ONBOARDING_VIEW.prop('currentIndex', 2));

    //Quit and relaunch the app
    await quit();
    await startAndConnect();

    //Ensure we start where we left off
    await waitForQuery(screenOnboarding.DEVICES_SLIDE.visible());
    equal(await getQueryProperty(screenOnboarding.ONBOARDING_VIEW, 'currentIndex'), 2);
    equal(await getSetting('onboardingStep'), 2);

    //Proceed to fourth/final onboarding step
    await waitForQuery(screenOnboarding.STEP_NAV_STACK_VIEW.ready());
    await waitForQueryAndClick(screenOnboarding.DEVICES_NEXT_BUTTON.visible());
    await waitForQuery(screenOnboarding.ONBOARDING_VIEW.prop('currentIndex', 3));

    //Quit and relaunch the app
    await quit();
    await startAndConnect();

    //Ensure we start where we left off
    await waitForQuery(screenOnboarding.START_SLIDE.visible());
    equal(await getQueryProperty(screenOnboarding.ONBOARDING_VIEW, 'currentIndex'), 3);
    equal(await getSetting('onboardingStep'), 3);
  });

  it('Complete onboarding with data collection disabled', async () => {

    //Ensure we are in onboarding
    await waitForQuery(screenOnboarding.DATA_SLIDE.visible());

    //Ensure that the data collection checkbox state is bound to onboardingDataCollectionEnabled, which defaults to false
    //and that this checkboxes state does not affect gleanEnabled
    equal(await getQueryProperty(screenOnboarding.DATA_CHECKBOX_ROW, 'isChecked'), 'false');
    equal(await getSetting('onboardingDataCollectionEnabled'), false);
    equal(await getSetting('gleanEnabled'), true);

    //Complete onboarding and get to the home screen
    await completeOnboarding();

    //Ensure glean is disabled
    equal(await getSetting('gleanEnabled'), false);
  });

  it('Complete onboarding with data collection enabled', async () => {

    //Ensure we are in onboarding
    await waitForQuery(screenOnboarding.DATA_SLIDE.visible());

    //Ensure that the data collection checkbox state remains bound to onboardingDataCollectionEnabled when clicked
    //and that this checkboxes state does not affect gleanEnabled
    await waitForQueryAndClick(screenOnboarding.DATA_CHECKBOX.visible());
    equal(await getQueryProperty(screenOnboarding.DATA_CHECKBOX_ROW, 'isChecked'), 'true');
    equal(await getSetting('onboardingDataCollectionEnabled'), true);
    equal(await getSetting('gleanEnabled'), true);

    //Complete onboarding and get to the home screen
    await completeOnboarding();

    //Ensure glean is enabled
    equal(await getSetting('gleanEnabled'), true);
  });

  it('Data collection checkbox state persists across app sessions', async () => {
    // Skip WASM because this test involves quitting and relaunching
    if (this.ctx.wasm) {
      return;
    }
    //Ensure we are in onboarding
    await waitForQuery(screenOnboarding.DATA_SLIDE.visible());

    //Click the data collection checkbox to set onboardingDataCollectionEnabled to true
    await waitForQueryAndClick(screenOnboarding.DATA_CHECKBOX.visible());

    //Quit and relaunch the app
    await quit();
    await startAndConnect();

    //Ensure we are still in onboarding
    await waitForQuery(screenOnboarding.DATA_SLIDE.visible());

    //Ensure that the data collection checkbox state remains bound to onboardingDataCollectionEnabled upon relaunch
    //and that this checkboxes state does not affect gleanEnabled
    equal(await getQueryProperty(screenOnboarding.DATA_CHECKBOX_ROW, 'isChecked'), 'true');
    equal(await getSetting('onboardingDataCollectionEnabled'), true);
    equal(await getSetting('gleanEnabled'), true);

    //Ensure the data collection checkbox is checked and onboardingDataCollectionEnabled is set to true
    equal(await getQueryProperty(screenOnboarding.DATA_CHECKBOX_ROW, 'isChecked'), 'true');
    equal(await getSetting('onboardingDataCollectionEnabled'), true);
  });

  describe('initialize onboarding telemetry tests', () => {
    // No Glean on WASM.
    if(runningOnWasm()) {
      return;
    }

    const dataScreenTelemetryId = "data_collection";
    const privacyScreenTelemetryId = "get_more_privacy";
    const devicesScreenTelemetryId = "install_on_5_devices";
    const startScreenTelemetryId = "connect_on_startup";

    it('Onboarding started event is recorded', async () => {
      await waitForQuery(screenOnboarding.ONBOARDING_VIEW.visible());
      let onboardingStartedEvents = await gleanTestGetValue("outcome", "onboardingStarted", "main");
      equal(onboardingStartedEvents.length, 1);

      //Quit and relaunch the app
      await quit();
      await startAndConnect();

      //Ensure the event is only recorded the first time it is shown (glean store is reset on re-launch)
      onboardingStartedEvents = await gleanTestGetValue("outcome", "onboardingStarted", "main");
      equal(onboardingStartedEvents.length, 0);
    });

    it('Onboarding completed event is recorded', async () => {
      //Make sure data collection is enabled so our metrics don't get deleted after we complete onboarding
      await setSetting('onboardingDataCollectionEnabled', true);

      await waitForQuery(screenOnboarding.ONBOARDING_VIEW.visible());

      await completeOnboarding()

      let onboardingCompletedEvents = await gleanTestGetValue("outcome", "onboardingCompleted", "main");
      equal(onboardingCompletedEvents.length, 1);
    });

    it('Onboarding slide impression events are recorded', async () => {
      //Verify that dataCollectionScreen event is recorded
      const dataCollectionScreenEvents = await gleanTestGetValue("impression", "dataCollectionScreen", "main");
      equal(dataCollectionScreenEvents.length, 1);
      const dataCollectionScreenEventsExtras = dataCollectionScreenEvents[0].extra;
      equal(dataScreenTelemetryId, dataCollectionScreenEventsExtras.screen);

      await advanceToSlide(1)

      //Verify that getMorePrivacyScreen event is recorded
      const getMorePrivacyScreenEvents = await gleanTestGetValue("impression", "getMorePrivacyScreen", "main");
      equal(getMorePrivacyScreenEvents.length, 1);
      const getMorePrivacyScreenEventsExtras = getMorePrivacyScreenEvents[0].extra;
      equal(privacyScreenTelemetryId, getMorePrivacyScreenEventsExtras.screen);

      await advanceToSlide(2)

      //Verify that installOn5DevicesScreen event is recorded
      const installOn5DevicesScreenEvents = await gleanTestGetValue("impression", "installOn5DevicesScreen", "main");
      equal(installOn5DevicesScreenEvents.length, 1);
      const installOn5DevicesScreenEventsExtras = installOn5DevicesScreenEvents[0].extra;
      equal(devicesScreenTelemetryId, installOn5DevicesScreenEventsExtras.screen);

      await advanceToSlide(3)

      //Verify that connectOnStartupScreen event is recorded
      const connectOnStartupScreenEvents = await gleanTestGetValue("impression", "connectOnStartupScreen", "main");
      equal(connectOnStartupScreenEvents.length, 1);
      const connectOnStartupScreenEventsExtras = connectOnStartupScreenEvents[0].extra;
      equal(startScreenTelemetryId, connectOnStartupScreenEventsExtras.screen);
    });

    it('Progress bar button events are recorded', async () => {
      //We call these more than once throughout the test, lets keep track of the aggregate and avoid resetting glean
      let dataUseSelectedEventCounter = 0
      let morePrivacySelectedEventCounter = 0

      await waitForQuery(screenOnboarding.DATA_SLIDE.visible());

      //Ensure telemetry is not generated by clicking on the current step
      await waitForQueryAndClick(screenOnboarding.STEP_PROG_BAR_DATA_BUTTON.visible());
      let dataUseSelectedEvents = await gleanTestGetValue("interaction", "dataUseSelected", "main");
      equal(dataUseSelectedEvents.length, 0);


      //Advance to privacy slide
      await advanceToSlide(1)

      //Ensure telemetry is not generated by clicking on the current step
      await waitForQueryAndClick(screenOnboarding.STEP_PROG_BAR_PRIVACY_BUTTON.visible());
      let morePrivacySelectedEvents = await gleanTestGetValue("interaction", "morePrivacySelected", "main");
      equal(morePrivacySelectedEvents.length, 0);

      //Ensure dataUseSelected event is recorded from the privacy screen
      await waitForQueryAndClick(screenOnboarding.STEP_PROG_BAR_DATA_BUTTON.visible());
      dataUseSelectedEventCounter++
      dataUseSelectedEvents = await gleanTestGetValue("interaction", "dataUseSelected", "main");
      equal(dataUseSelectedEvents.length, dataUseSelectedEventCounter);
      let dataUseSelectedEventsExtras = dataUseSelectedEvents[dataUseSelectedEventCounter - 1].extra;
      equal(privacyScreenTelemetryId, dataUseSelectedEventsExtras.screen);


      //Advance to devices slide
      await advanceToSlide(2)

      //Ensure telemetry is not generated by clicking on the current step
      await waitForQueryAndClick(screenOnboarding.STEP_PROG_BAR_DEVICES_BUTTON.visible());
      let addDevicesSelectedEvents = await gleanTestGetValue("interaction", "addDevicesSelected", "main");
      equal(addDevicesSelectedEvents.length, 0);

      //Ensure morePrivacySelected event is recorded from the devices screen
      await waitForQueryAndClick(screenOnboarding.STEP_PROG_BAR_PRIVACY_BUTTON.visible());
      morePrivacySelectedEventCounter++
      morePrivacySelectedEvents = await gleanTestGetValue("interaction", "morePrivacySelected", "main");
      equal(morePrivacySelectedEvents.length, morePrivacySelectedEventCounter);
      let morePrivacySelectedEventsExtras = morePrivacySelectedEvents[morePrivacySelectedEventCounter - 1].extra;
      equal(devicesScreenTelemetryId, morePrivacySelectedEventsExtras.screen);

      await advanceToSlide(2)

      //Ensure dataUseSelected event is recorded from the devices screen
      await waitForQueryAndClick(screenOnboarding.STEP_PROG_BAR_DATA_BUTTON.visible());
      dataUseSelectedEventCounter++
      dataUseSelectedEvents = await gleanTestGetValue("interaction", "dataUseSelected", "main");
      equal(dataUseSelectedEvents.length, dataUseSelectedEventCounter);
      dataUseSelectedEventsExtras = dataUseSelectedEvents[dataUseSelectedEventCounter - 1].extra;
      equal(devicesScreenTelemetryId, dataUseSelectedEventsExtras.screen);


      //Advance to start slide
      await advanceToSlide(3)

      //Start slide progress bar button does not have telemetry (because it never does anything)
      //so we don't need to ensure it doesn't generate telemetry

      //Ensure addDevicesSelected event is recorded from the start screen
      await waitForQueryAndClick(screenOnboarding.STEP_PROG_BAR_DEVICES_BUTTON.visible());
      addDevicesSelectedEvents = await gleanTestGetValue("interaction", "addDevicesSelected", "main");
      equal(addDevicesSelectedEvents.length, 1);
      let addDevicesSelectedEventsExtras = addDevicesSelectedEvents[0].extra;
      equal(startScreenTelemetryId, addDevicesSelectedEventsExtras.screen);

      await advanceToSlide(3)

      //Ensure morePrivacySelected event is recorded from the start screen
      await waitForQueryAndClick(screenOnboarding.STEP_PROG_BAR_PRIVACY_BUTTON.visible());
      morePrivacySelectedEventCounter++
      morePrivacySelectedEvents = await gleanTestGetValue("interaction", "morePrivacySelected", "main");
      equal(morePrivacySelectedEvents.length, morePrivacySelectedEventCounter);
      morePrivacySelectedEventsExtras = morePrivacySelectedEvents[morePrivacySelectedEventCounter - 1].extra;
      equal(startScreenTelemetryId, morePrivacySelectedEventsExtras.screen);

      await advanceToSlide(3)

      //Ensure dataUseSelected event is recorded from the start screen
      await waitForQueryAndClick(screenOnboarding.STEP_PROG_BAR_DATA_BUTTON.visible());
      dataUseSelectedEventCounter++
      dataUseSelectedEvents = await gleanTestGetValue("interaction", "dataUseSelected", "main");
      equal(dataUseSelectedEvents.length, dataUseSelectedEventCounter);
      dataUseSelectedEventsExtras = dataUseSelectedEvents[dataUseSelectedEventCounter - 1].extra;
      equal(startScreenTelemetryId, dataUseSelectedEventsExtras.screen);

    });

    it('Data slide events are recorded', async () => {
      await waitForQuery(screenOnboarding.DATA_SLIDE.visible());

      //Verify that privacyNoticedSelected event is recorded
      await waitForQueryAndClick(screenOnboarding.DATA_PRIVACY_LINK.visible());
      const privacyNoticeSelectedEvents = await gleanTestGetValue("interaction", "privacyNoticeSelected", "main");
      equal(privacyNoticeSelectedEvents.length, 1);
      const privacyNoticeSelectedEventsExtras = privacyNoticeSelectedEvents[0].extra;
      equal(dataScreenTelemetryId, privacyNoticeSelectedEventsExtras.screen);

      //Used to wipe previous continueSelected events that were recorded
      //during authentication before testing in onboarding
      await gleanTestReset();

      //Verify that continueSelected event is recorded
      await waitForQueryAndClick(screenOnboarding.DATA_NEXT_BUTTON.visible());
      const continueSelectedEvents = await gleanTestGetValue("interaction", "continueSelected", "main");
      equal(continueSelectedEvents.length, 1);
      const continueSelectedEventsExtras = continueSelectedEvents[0].extra;
      equal(dataScreenTelemetryId, continueSelectedEventsExtras.screen);
    });

    it('Privacy slide events are recorded', async () => {
      await advanceToSlide(1);

      await waitForQuery(screenOnboarding.PRIVACY_SLIDE.visible());

      //Verify that blockAdsEnabled event is recorded
      await waitForQueryAndClick(screenOnboarding.PRIVACY_BLOCK_ADS_CHECKBOX.visible());
      const blockAdsEnabledEvents = await gleanTestGetValue("interaction", "blockAdsEnabled", "main");
      equal(blockAdsEnabledEvents.length, 1);
      const blockAdsEnabledEventsExtras = blockAdsEnabledEvents[0].extra;
      equal(privacyScreenTelemetryId, blockAdsEnabledEventsExtras.screen);

      //Verify that blockAdsDisabled event is recorded
      await waitForQueryAndClick(screenOnboarding.PRIVACY_BLOCK_ADS_CHECKBOX.visible());
      const blockAdsDisabledEvents = await gleanTestGetValue("interaction", "blockAdsDisabled", "main");
      equal(blockAdsDisabledEvents.length, 1);
      const blockAdsDisabledEventsExtras = blockAdsDisabledEvents[0].extra;
      equal(privacyScreenTelemetryId, blockAdsDisabledEventsExtras.screen);

      //Verify that blockTrackersEnabled event is recorded
      await waitForQueryAndClick(screenOnboarding.PRIVACY_BLOCK_TRACKERS_CHECKBOX.visible());
      const blockTrackersEnabledEvents = await gleanTestGetValue("interaction", "blockTrackersEnabled", "main");
      equal(blockTrackersEnabledEvents.length, 1);
      const blockTrackersEnabledEventsExtras = blockTrackersEnabledEvents[0].extra;
      equal(privacyScreenTelemetryId, blockTrackersEnabledEventsExtras.screen);

      //Verify that blockTrackersDisabled event is recorded
      await waitForQueryAndClick(screenOnboarding.PRIVACY_BLOCK_TRACKERS_CHECKBOX.visible());
      const blockTrackersDisabledEvents = await gleanTestGetValue("interaction", "blockTrackersDisabled", "main");
      equal(blockTrackersDisabledEvents.length, 1);
      const blockTrackersDisabledEventsExtras = blockTrackersDisabledEvents[0].extra;
      equal(privacyScreenTelemetryId, blockTrackersDisabledEventsExtras.screen);

      //Verify that blockMalwareEnabled event is recorded
      await waitForQueryAndClick(screenOnboarding.PRIVACY_BLOCK_MALWARE_CHECKBOX.visible());
      const blockMalwareEnabledEvents = await gleanTestGetValue("interaction", "blockMalwareEnabled", "main");
      equal(blockMalwareEnabledEvents.length, 1);
      const blockMalwareEnabledEventsExtras = blockMalwareEnabledEvents[0].extra;
      equal(privacyScreenTelemetryId, blockMalwareEnabledEventsExtras.screen);

      //Verify that blockMalwareDisabled event is recorded
      await waitForQueryAndClick(screenOnboarding.PRIVACY_BLOCK_MALWARE_CHECKBOX.visible());
      const blockMalwareDisabledEvents = await gleanTestGetValue("interaction", "blockMalwareDisabled", "main");
      equal(blockMalwareDisabledEvents.length, 1);
      const blockMalwareDisabledEventsExtras = blockMalwareDisabledEvents[0].extra;
      equal(privacyScreenTelemetryId, blockMalwareDisabledEventsExtras.screen);

      //Verify that goBackSelected event is recorded
      await waitForQueryAndClick(screenOnboarding.PRIVACY_BACK_BUTTON.visible());
      const goBackSelectedEvents = await gleanTestGetValue("interaction", "goBackSelected", "main");
      equal(goBackSelectedEvents.length, 1);
      const goBackSelectedEventsExtras = goBackSelectedEvents[0].extra;
      equal(privacyScreenTelemetryId, goBackSelectedEventsExtras.screen);

      await advanceToSlide(1)

      //Used to wipe previous continueSelected events that were recorded
      //during authentication before testing in onboarding
      await gleanTestReset();

      //Verify that continueSelected event is recorded
      await waitForQueryAndClick(screenOnboarding.PRIVACY_NEXT_BUTTON.visible());
      const continueSelectedEvents = await gleanTestGetValue("interaction", "continueSelected", "main");
      equal(continueSelectedEvents.length, 1);
      const continueSelectedEventsExtras = continueSelectedEvents[0].extra;
      equal(privacyScreenTelemetryId, continueSelectedEventsExtras.screen);
    });

    it('Devices slide events are recorded', async () => {
      await advanceToSlide(2);

      await waitForQuery(screenOnboarding.DEVICES_SLIDE.visible());

      if (await getQueryProperty(screenOnboarding.DEVICES_DEVICE_TYPE_TOGGLE.visible(), 'selectedIndex') == 0) {
        //[Android, Windows, Linux] Starting with Android - switch to iOS and back to Android
        //Verify that showIosQrSelected event is recorded
        await waitForQueryAndClick(screenOnboarding.DEVICES_TOGGLE_BTN_IOS.visible());
        const showIosQrSelectedEvents = await gleanTestGetValue("interaction", "showIosQrSelected", "main");
        equal(showIosQrSelectedEvents.length, 1);
        const showIosQrSelectedEventsExtras = showIosQrSelectedEvents[0].extra;
        equal(devicesScreenTelemetryId, showIosQrSelectedEventsExtras.screen);

        //Verify that showAndroidQrSelected event is recorded
        await waitForQueryAndClick(screenOnboarding.DEVICES_TOGGLE_BTN_ANDROID.visible());
        const showAndroidQrSelectedEvents = await gleanTestGetValue("interaction", "showAndroidQrSelected", "main");
        equal(showAndroidQrSelectedEvents.length, 1);
        const showAndroidQrSelectedEventsExtras = showAndroidQrSelectedEvents[0].extra;
        equal(devicesScreenTelemetryId, showAndroidQrSelectedEventsExtras.screen);

      }
      else {
        //[iOS, macOS] Starting with iOS - switch to Android and back to iOS
        //Verify that showAndroidQrSelected event is recorded
        await waitForQueryAndClick(screenOnboarding.DEVICES_TOGGLE_BTN_ANDROID.visible());
        const showAndroidQrSelectedEvents = await gleanTestGetValue("interaction", "showAndroidQrSelected", "main");
        equal(showAndroidQrSelectedEvents.length, 1);
        const showAndroidQrSelectedEventsExtras = showAndroidQrSelectedEvents[0].extra;
        equal(devicesScreenTelemetryId, showAndroidQrSelectedEventsExtras.screen);

        //Verify that showIosQrSelected event is recorded
        await waitForQueryAndClick(screenOnboarding.DEVICES_TOGGLE_BTN_IOS.visible());
        const showIosQrSelectedEvents = await gleanTestGetValue("interaction", "showIosQrSelected", "main");
        equal(showIosQrSelectedEvents.length, 1);
        const showIosQrSelectedEventsExtras = showIosQrSelectedEvents[0].extra;
        equal(devicesScreenTelemetryId, showIosQrSelectedEventsExtras.screen);
      }

      //Verify that goBackSelected event is recorded
      await waitForQueryAndClick(screenOnboarding.DEVICES_BACK_BUTTON.visible());
      const goBackSelectedEvents = await gleanTestGetValue("interaction", "goBackSelected", "main");
      equal(goBackSelectedEvents.length, 1);
      const goBackSelectedEventsExtras = goBackSelectedEvents[0].extra;
      equal(devicesScreenTelemetryId, goBackSelectedEventsExtras.screen);

      await advanceToSlide(2)

      //Used to wipe previous continueSelected events that were recorded
      //during authentication before testing in onboarding
      await gleanTestReset();

      //Verify that continueSelected event is recorded
      await waitForQueryAndClick(screenOnboarding.DEVICES_NEXT_BUTTON.visible());
      const continueSelectedEvents = await gleanTestGetValue("interaction", "continueSelected", "main");
      equal(continueSelectedEvents.length, 1);
      const continueSelectedEventsExtras = continueSelectedEvents[0].extra;
      equal(devicesScreenTelemetryId, continueSelectedEventsExtras.screen);
    });

    it('Desktop start slide events are recorded', async () => {
      await advanceToSlide(3);

      await waitForQuery(screenOnboarding.START_SLIDE.visible());

      //Verify that connectOnStartupEnabled event is recorded
      await waitForQueryAndClick(screenOnboarding.START_START_AT_BOOT_CHECKBOX.visible());
      const connectOnStartupEnabledEvents = await gleanTestGetValue("interaction", "connectOnStartupEnabled", "main");
      equal(connectOnStartupEnabledEvents.length, 1);
      const connectOnStartupEnabledEventsExtras = connectOnStartupEnabledEvents[0].extra;
      equal(startScreenTelemetryId, connectOnStartupEnabledEventsExtras.screen);

      //Verify that connectOnStartupDisabled event is recorded
      await waitForQueryAndClick(screenOnboarding.START_START_AT_BOOT_CHECKBOX.visible());
      const connectOnStartupDisabledEvents = await gleanTestGetValue("interaction", "connectOnStartupDisabled", "main");
      equal(connectOnStartupDisabledEvents.length, 1);
      const connectOnStartupDisabledEventsExtras = connectOnStartupDisabledEvents[0].extra;
      equal(startScreenTelemetryId, connectOnStartupDisabledEventsExtras.screen);

      //Verify that goBackSelected event is recorded
      await waitForQueryAndClick(screenOnboarding.START_BACK_BUTTON.visible());
      const goBackSelectedEvents = await gleanTestGetValue("interaction", "goBackSelected", "main");
      equal(goBackSelectedEvents.length, 1);
      const goBackSelectedEventsExtras = goBackSelectedEvents[0].extra;
      equal(startScreenTelemetryId, goBackSelectedEventsExtras.screen);

      await advanceToSlide(3)

      //Used to wipe previous continueSelected events that were recorded
      //during authentication before testing in onboarding
      await gleanTestReset();

      //For the next test we need data collection enabled due to a race condition between recording
      //the getStartedSelected event and data collection being disabled at onboarding completion
      await setSetting('onboardingDataCollectionEnabled', true)

      //Verify that getStartedSelected event is recorded
      await waitForQueryAndClick(screenOnboarding.START_NEXT_BUTTON.visible());
      const getStartedSelectedEvents = await gleanTestGetValue("interaction", "getStartedSelected", "main");
      equal(getStartedSelectedEvents.length, 1);
      const getStartedSelectedEventsExtras = getStartedSelectedEvents[0].extra;
      equal(startScreenTelemetryId, getStartedSelectedEventsExtras.screen);
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
