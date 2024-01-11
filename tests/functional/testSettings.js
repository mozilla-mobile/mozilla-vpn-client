/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { equal, strictEqual } from 'assert';
import { navBar, screenSettings, screenGetHelp, global, screenDeveloperMenu } from './queries.js';
import { waitForQueryAndClick, isFeatureFlippedOff, flipFeatureOff, waitForQuery, getQueryProperty, getSetting, setSetting, clickOnQuery, waitForCondition, getLastUrl, scrollToQuery, setQueryProperty, waitForInitialView, clickOnQueryAndAcceptAnyResults, authenticateInApp, setGleanAutomationHeader, activateViaToggle, awaitSuccessfulConnection, gleanTestGetValue } from './helper.js';
import { vpnIsInactive, startAndConnect } from './setupVpn.js';

describe('Settings', function() {
  this.timeout(60000);
  this.ctx.authenticationNeeded = true;

  beforeEach(async () => {
    await waitForQueryAndClick(navBar.SETTINGS.visible());

    if (!(await isFeatureFlippedOff('subscriptionManagement'))) {
      await flipFeatureOff('subscriptionManagement');
    }
  });

  async function checkSetting(query, settingKey) {
    await waitForQuery(query.visible());
    equal(
        (await getQueryProperty(query, 'isChecked') === 'true'),
        await getSetting(settingKey));

    await setSetting(settingKey, true);
    equal((await getSetting(settingKey)), true);
    equal((await getQueryProperty(query, 'isChecked')), 'true');

    await setSetting(settingKey, false);
    equal((await getSetting(settingKey)), false);
    equal((await getQueryProperty(query, 'isChecked')), 'false');
  }

  async function getToGetHelpView() {
    await waitForQueryAndClick(screenSettings.GET_HELP.visible());
    await waitForQuery(screenGetHelp.BACK_BUTTON.visible());
    await waitForQuery(global.SCREEN_LOADER.ready());
  }

  it('Checking settings entries', async () => {
    await waitForQuery(screenSettings.USER_PROFILE.visible());

    if ((await isFeatureFlippedOff('subscriptionManagement'))) {
      await clickOnQuery(screenSettings.USER_PROFILE.visible());
      await waitForCondition(async () => {
        const url = await getLastUrl();
        return url.includes('https://accounts.stage.mozaws.net') &&
            url.includes('?email=test@mozilla.com');
      });
    }

    await waitForQuery(screenSettings.PRIVACY.visible());
    await waitForQuery(screenSettings.APP_EXCLUSIONS.visible());
    await waitForQuery(screenSettings.TIPS_AND_TRICKS.visible());
    await waitForQuery(screenSettings.MY_DEVICES.visible());
    await waitForQuery(screenSettings.APP_PREFERENCES.visible());
    await waitForQuery(screenSettings.GET_HELP.visible());
    await waitForQuery(screenSettings.ABOUT_US.visible());
    await waitForQuery(screenSettings.SIGN_OUT.visible());
  });

  it('Checking the tips and tricks settings', async () => {
    await waitForQuery(screenSettings.TIPS_AND_TRICKS.visible());
    await scrollToQuery(
        screenSettings.SCREEN, screenSettings.TIPS_AND_TRICKS);

    await clickOnQuery(screenSettings.TIPS_AND_TRICKS.visible());
    await waitForQuery(screenSettings.tipsAndTricksView.SCREEN);


    // TODO: (VPN-2749)
    // Test guides
    // let guides = await vpn.guides();
    // let guideParent = 'guideLayout'

    // for (var guide of guides) {
    //   guide = guideParent + "/" + guide;

    //   await vpn.setElementProperty(
    //     'allTab', 'contentY', 'i',
    //     parseInt(await vpn.getElementProperty(guide, 'y')) +
    //     parseInt(await vpn.getElementProperty(guide, 'height')) +
    //     parseInt(await vpn.getElementProperty(guideParent, 'y')));
    //   await vpn.wait();

    //   await vpn.waitForElement(guide);
    //   await vpn.clickOnElement(guide);
    //   await vpn.wait();

    //   await vpn.waitForElement('backArrow');
    //   await vpn.clickOnElement('backArrow')
    //   await vpn.wait();

    //   await vpn.waitForElement('settingsTipsAndTricksPage');
    // }

    await waitForQueryAndClick(
        screenSettings.tipsAndTricksView.BACK.visible());

    await waitForQuery(screenSettings.USER_PROFILE.visible());
  });

  it('Checking the privacy settings', async () => {
    await waitForQuery(screenSettings.PRIVACY.visible());

    await scrollToQuery(
        screenSettings.SCREEN, screenSettings.PRIVACY);

    await clickOnQuery(screenSettings.PRIVACY.visible());
    await waitForQuery(screenSettings.STACKVIEW.ready());

    // Checking that the `These changes may affect...` warning is visible
    await waitForQuery(
        screenSettings.privacyView.INFORMATION_CARD.visible());

    // Checking if the checkboxes are correctly set based on the settings prop
    await setSetting('dnsProviderFlags', 0);
    await waitForQuery(
        screenSettings.privacyView.BLOCK_ADS.visible().prop(
            'isChecked', false));
    await waitForQuery(
        screenSettings.privacyView.BLOCK_TRACKERS.visible().prop(
            'isChecked', false));
    await waitForQuery(
        screenSettings.privacyView.BLOCK_MALWARE.visible().prop(
            'isChecked', false));

    await setSetting('dnsProviderFlags', 2);
    await waitForQuery(
        screenSettings.privacyView.BLOCK_ADS.visible().prop(
            'isChecked', true));
    await waitForQuery(
        screenSettings.privacyView.BLOCK_TRACKERS.visible().prop(
            'isChecked', false));
    await waitForQuery(
        screenSettings.privacyView.BLOCK_MALWARE.visible().prop(
            'isChecked', false));

    await setSetting('dnsProviderFlags', 4);
    await waitForQuery(
        screenSettings.privacyView.BLOCK_ADS.visible().prop(
            'isChecked', false));
    await waitForQuery(
        screenSettings.privacyView.BLOCK_TRACKERS.visible().prop(
            'isChecked', true));
    await waitForQuery(
        screenSettings.privacyView.BLOCK_MALWARE.visible().prop(
            'isChecked', false));

    await setSetting('dnsProviderFlags', 6);
    await waitForQuery(
        screenSettings.privacyView.BLOCK_ADS.visible().prop(
            'isChecked', true));
    await waitForQuery(
        screenSettings.privacyView.BLOCK_TRACKERS.visible().prop(
            'isChecked', true));
    await waitForQuery(
        screenSettings.privacyView.BLOCK_MALWARE.visible().prop(
            'isChecked', false));

    await setSetting('dnsProviderFlags', 8);
    await waitForQuery(
        screenSettings.privacyView.BLOCK_ADS.visible().prop(
            'isChecked', false));
    await waitForQuery(
        screenSettings.privacyView.BLOCK_TRACKERS.visible().prop(
            'isChecked', false));
    await waitForQuery(
        screenSettings.privacyView.BLOCK_MALWARE.visible().prop(
            'isChecked', true));

    await setSetting('dnsProviderFlags', 10);
    await waitForQuery(
        screenSettings.privacyView.BLOCK_ADS.visible().prop(
            'isChecked', true));
    await waitForQuery(
        screenSettings.privacyView.BLOCK_TRACKERS.visible().prop(
            'isChecked', false));
    await waitForQuery(
        screenSettings.privacyView.BLOCK_MALWARE.visible().prop(
            'isChecked', true));

    await setSetting('dnsProviderFlags', 12);
    await waitForQuery(
        screenSettings.privacyView.BLOCK_ADS.visible().prop(
            'isChecked', false));
    await waitForQuery(
        screenSettings.privacyView.BLOCK_TRACKERS.visible().prop(
            'isChecked', true));
    await waitForQuery(
        screenSettings.privacyView.BLOCK_MALWARE.visible().prop(
            'isChecked', true));

    await setSetting('dnsProviderFlags', 14);
    await waitForQuery(
        screenSettings.privacyView.BLOCK_ADS.visible().prop(
            'isChecked', true));
    await waitForQuery(
        screenSettings.privacyView.BLOCK_TRACKERS.visible().prop(
            'isChecked', true));
    await waitForQuery(
        screenSettings.privacyView.BLOCK_MALWARE.visible().prop(
            'isChecked', true));

    // Tests the clicks
    await setSetting('dnsProviderFlags', 0);
    await waitForQueryAndClick(
        screenSettings.privacyView.BLOCK_ADS_CHECKBOX.visible());
    equal(await getSetting('dnsProviderFlags'), 2);

    await setSetting('dnsProviderFlags', 0);
    await waitForQueryAndClick(
        screenSettings.privacyView.BLOCK_TRACKERS_CHECKBOX.visible());
    equal(await getSetting('dnsProviderFlags'), 4);

    await setSetting('dnsProviderFlags', 0);
    await waitForQueryAndClick(
        screenSettings.privacyView.BLOCK_MALWARE_CHECKBOX.visible());
    equal(await getSetting('dnsProviderFlags'), 8);

    // Let's test the modal
    await setSetting('dnsProviderFlags', 1);
    await waitForQueryAndClick(
        screenSettings.privacyView.BLOCK_ADS_CHECKBOX.visible());
    equal(await getSetting('dnsProviderFlags'), 1);
    await waitForQueryAndClick(
        screenSettings.privacyView.MODAL_SECONDARY_BUTTON.visible());
    await waitForQuery(
        screenSettings.privacyView.MODAL_LOADER.prop('active', false));
    equal(await getSetting('dnsProviderFlags'), 1);
    await waitForQueryAndClick(
        screenSettings.privacyView.BLOCK_ADS_CHECKBOX.visible());
    equal(await getSetting('dnsProviderFlags'), 1);
    await waitForQueryAndClick(
        screenSettings.privacyView.MODAL_CLOSE_BUTTON.visible());
    await waitForQuery(
        screenSettings.privacyView.MODAL_LOADER.prop('active', false));
    equal(await getSetting('dnsProviderFlags'), 1);
    await waitForQueryAndClick(
        screenSettings.privacyView.BLOCK_ADS_CHECKBOX.visible());
    equal(await getSetting('dnsProviderFlags'), 1);
    await waitForQueryAndClick(
        screenSettings.privacyView.MODAL_PRIMARY_BUTTON.visible());
    await waitForQuery(
        screenSettings.privacyView.MODAL_LOADER.prop('active', false));
    equal(await getSetting('dnsProviderFlags'), 2);

    await setSetting('dnsProviderFlags', 1);
    await waitForQueryAndClick(
        screenSettings.privacyView.BLOCK_TRACKERS_CHECKBOX.visible());
    equal(await getSetting('dnsProviderFlags'), 1);
    await waitForQueryAndClick(
        screenSettings.privacyView.MODAL_SECONDARY_BUTTON.visible());
    await waitForQuery(
        screenSettings.privacyView.MODAL_LOADER.prop('active', false));
    equal(await getSetting('dnsProviderFlags'), 1);
    await waitForQueryAndClick(
        screenSettings.privacyView.BLOCK_TRACKERS_CHECKBOX.visible());
    equal(await getSetting('dnsProviderFlags'), 1);
    await waitForQueryAndClick(
        screenSettings.privacyView.MODAL_CLOSE_BUTTON.visible());
    await waitForQuery(
        screenSettings.privacyView.MODAL_LOADER.prop('active', false));
    equal(await getSetting('dnsProviderFlags'), 1);
    await waitForQueryAndClick(
        screenSettings.privacyView.BLOCK_TRACKERS_CHECKBOX.visible());
    equal(await getSetting('dnsProviderFlags'), 1);
    await waitForQueryAndClick(
        screenSettings.privacyView.MODAL_PRIMARY_BUTTON.visible());
    await waitForQuery(
        screenSettings.privacyView.MODAL_LOADER.prop('active', false));
    equal(await getSetting('dnsProviderFlags'), 4);

    await setSetting('dnsProviderFlags', 1);
    await waitForQueryAndClick(
        screenSettings.privacyView.BLOCK_MALWARE_CHECKBOX.visible());
    equal(await getSetting('dnsProviderFlags'), 1);
    await waitForQueryAndClick(
        screenSettings.privacyView.MODAL_SECONDARY_BUTTON.visible());
    await waitForQuery(
        screenSettings.privacyView.MODAL_LOADER.prop('active', false));
    equal(await getSetting('dnsProviderFlags'), 1);
    await waitForQueryAndClick(
        screenSettings.privacyView.BLOCK_MALWARE_CHECKBOX.visible());
    equal(await getSetting('dnsProviderFlags'), 1);
    await waitForQueryAndClick(
        screenSettings.privacyView.MODAL_CLOSE_BUTTON.visible());
    await waitForQuery(
        screenSettings.privacyView.MODAL_LOADER.prop('active', false));
    equal(await getSetting('dnsProviderFlags'), 1);
    await waitForQueryAndClick(
        screenSettings.privacyView.BLOCK_MALWARE_CHECKBOX.visible());
    equal(await getSetting('dnsProviderFlags'), 1);
    await waitForQueryAndClick(
        screenSettings.privacyView.MODAL_PRIMARY_BUTTON.visible());
    await waitForQuery(
        screenSettings.privacyView.MODAL_LOADER.prop('active', false));
    equal(await getSetting('dnsProviderFlags'), 8);

    // Let's go back
    await waitForQueryAndClick(screenSettings.BACK.visible());
    await waitForQuery(screenSettings.STACKVIEW.ready());

    await waitForQuery(screenSettings.USER_PROFILE.visible());

    // Reset
    await setSetting('dnsProviderFlags', 0);
  });

  it('Checking the DNS settings', async () => {
    await setSetting('userDNS', '');
    await setSetting('dnsProviderFlags', 0);

    await waitForQueryAndClick(
        screenSettings.APP_PREFERENCES.visible());
    await waitForQuery(screenSettings.STACKVIEW.ready());

    await waitForQueryAndClick(
        screenSettings.appPreferencesView.DNS_SETTINGS.visible());
    await waitForQuery(screenSettings.STACKVIEW.ready());

    // Checking if the checkboxes are correctly set based on the settings prop
    await waitForQuery(
        screenSettings.appPreferencesView.dnsSettingsView.STANDARD_DNS
            .visible()
            .prop('checked', true));
    await waitForQuery(
        screenSettings.appPreferencesView.dnsSettingsView.CUSTOM_DNS
            .visible()
            .prop('checked', false));

    await waitForQueryAndClick(navBar.HOME.visible());
    await waitForQuery(global.SCREEN_LOADER.ready());

    await setSetting('dnsProviderFlags', 1);

    await waitForQueryAndClick(navBar.SETTINGS.visible());
    await waitForQuery(global.SCREEN_LOADER.ready());

    await waitForQuery(
        screenSettings.appPreferencesView.dnsSettingsView.STANDARD_DNS
            .visible()
            .prop('checked', false));
    await waitForQuery(
        screenSettings.appPreferencesView.dnsSettingsView.CUSTOM_DNS
            .visible()
            .prop('checked', true));
    await waitForQuery(screenSettings.appPreferencesView
                               .dnsSettingsView.CUSTOM_DNS_INPUT.visible()
                               .prop('hasError', false));
    await setQueryProperty(
        screenSettings.appPreferencesView.dnsSettingsView
            .CUSTOM_DNS_INPUT.visible(),
        'text', 'wow');
    await waitForQuery(screenSettings.appPreferencesView
                               .dnsSettingsView.CUSTOM_DNS_INPUT.visible()
                               .prop('hasError', true));
    await setQueryProperty(
        screenSettings.appPreferencesView.dnsSettingsView
            .CUSTOM_DNS_INPUT.visible(),
        'text', '1.2.3.4');
    await waitForQuery(screenSettings.appPreferencesView
                               .dnsSettingsView.CUSTOM_DNS_INPUT.visible()
                               .prop('hasError', false));

    // Check the warning message
    equal(
        await getQueryProperty(
            screenSettings.appPreferencesView.dnsSettingsView
                .INFORMATION_CARD_LOADER,
            'active'), 'false');

    // Check the click
    await waitForQueryAndClick(
        screenSettings.appPreferencesView.dnsSettingsView.STANDARD_DNS
            .visible()
            .prop('checked', false));

    await waitForQueryAndClick(navBar.HOME.visible());
    await waitForQuery(global.SCREEN_LOADER.ready());

    equal(await getSetting('dnsProviderFlags'), 0);

    await waitForQueryAndClick(navBar.SETTINGS.visible());
    await waitForQuery(global.SCREEN_LOADER.ready());

    await waitForQueryAndClick(
        screenSettings.appPreferencesView.dnsSettingsView.CUSTOM_DNS
            .visible()
            .prop('checked', false));

    await waitForQueryAndClick(navBar.HOME.visible());
    await waitForQuery(global.SCREEN_LOADER.ready());

    equal(await getSetting('dnsProviderFlags'), 1);

    // Check the modal
    await setSetting('dnsProviderFlags', 2);

    await waitForQueryAndClick(navBar.SETTINGS.visible());
    await waitForQuery(global.SCREEN_LOADER.ready());

    await waitForQuery(
        screenSettings.appPreferencesView.dnsSettingsView.STANDARD_DNS
            .visible()
            .prop('checked', true));

    await waitForQueryAndClick(
        screenSettings.appPreferencesView.dnsSettingsView.CUSTOM_DNS
            .visible()
            .prop('checked', false));

    await waitForQueryAndClick(
        screenSettings.appPreferencesView.dnsSettingsView
            .MODAL_SECONDARY_BUTTON.visible());
    await waitForQuery(
        screenSettings.appPreferencesView.dnsSettingsView
            .MODAL_LOADER.prop('active', false));

    await waitForQueryAndClick(screenSettings.appPreferencesView
                                       .dnsSettingsView.CUSTOM_DNS.visible());

    await waitForQueryAndClick(
        screenSettings.appPreferencesView.dnsSettingsView
            .MODAL_CLOSE_BUTTON.visible());
    await waitForQuery(
        screenSettings.appPreferencesView.dnsSettingsView
            .MODAL_LOADER.prop('active', false));

    await waitForQueryAndClick(screenSettings.appPreferencesView
                                       .dnsSettingsView.CUSTOM_DNS.visible());

    await waitForQueryAndClick(
        screenSettings.appPreferencesView.dnsSettingsView
            .MODAL_PRIMARY_BUTTON.visible());
    await waitForQuery(
        screenSettings.appPreferencesView.dnsSettingsView
            .MODAL_LOADER.prop('active', false));

    await setQueryProperty(
        screenSettings.appPreferencesView.dnsSettingsView
            .CUSTOM_DNS_INPUT.visible(),
        'text', '1.2.3.4');

    await waitForQueryAndClick(screenSettings.BACK.visible());
    await waitForQuery(screenSettings.STACKVIEW.ready());

    equal(await getSetting('dnsProviderFlags'), 1);

    await waitForQueryAndClick(screenSettings.BACK.visible());
    await waitForQuery(screenSettings.STACKVIEW.ready());

    await waitForQuery(screenSettings.USER_PROFILE.visible());
  });

  it('Checking the DNS settings reset', async () => {
    await setSetting('dnsProviderFlags', 0);
    await setSetting('userDNS', '');

    await waitForQueryAndClick(
        screenSettings.APP_PREFERENCES.visible());
    await waitForQuery(screenSettings.STACKVIEW.ready());

    await waitForQueryAndClick(
        screenSettings.appPreferencesView.DNS_SETTINGS.visible());
    await waitForQuery(screenSettings.STACKVIEW.ready());

    // Checking if the checkboxes are correctly set based on the settings prop
    await waitForQuery(
        screenSettings.appPreferencesView.dnsSettingsView.STANDARD_DNS
            .visible()
            .prop('checked', true));
    await waitForQuery(
        screenSettings.appPreferencesView.dnsSettingsView.CUSTOM_DNS
            .visible()
            .prop('checked', false));

    // Click on "Custom DNS" but leaving the input field empty.
    await waitForQueryAndClick(
        screenSettings.appPreferencesView.dnsSettingsView.CUSTOM_DNS
            .visible()
            .prop('checked', false));

    await setQueryProperty(
        screenSettings.appPreferencesView.dnsSettingsView
            .CUSTOM_DNS_INPUT.visible(),
        'text', '');

    // Going back...
    await waitForQueryAndClick(screenSettings.BACK.visible());
    await waitForQuery(screenSettings.STACKVIEW.ready());

    // .. the DNS setting is reset to the default value.
    equal(await getSetting('dnsProviderFlags'), 0);

    // Same test as before...
    await waitForQueryAndClick(
        screenSettings.appPreferencesView.DNS_SETTINGS.visible());
    await waitForQuery(screenSettings.STACKVIEW.ready());

    await waitForQuery(
        screenSettings.appPreferencesView.dnsSettingsView.STANDARD_DNS
            .visible()
            .prop('checked', true));
    await waitForQuery(
        screenSettings.appPreferencesView.dnsSettingsView.CUSTOM_DNS
            .visible()
            .prop('checked', false));

    await waitForQueryAndClick(
        screenSettings.appPreferencesView.dnsSettingsView.CUSTOM_DNS
            .visible()
            .prop('checked', false));

    // But with a valid DNS value...
    await setQueryProperty(
        screenSettings.appPreferencesView.dnsSettingsView
            .CUSTOM_DNS_INPUT.visible(),
        'text', '1.2.3.4');

    await waitForQueryAndClick(screenSettings.BACK.visible());
    await waitForQuery(screenSettings.STACKVIEW.ready());

    // We keep the custom DNS.
    equal(await getSetting('dnsProviderFlags'), 1);

    // Write something invalid...
    await waitForQueryAndClick(
        screenSettings.appPreferencesView.DNS_SETTINGS.visible());
    await waitForQuery(screenSettings.STACKVIEW.ready());

    await waitForQuery(
        screenSettings.appPreferencesView.dnsSettingsView.STANDARD_DNS
            .visible()
            .prop('checked', false));
    await waitForQuery(
        screenSettings.appPreferencesView.dnsSettingsView.CUSTOM_DNS
            .visible()
            .prop('checked', true));

    equal(
        await getQueryProperty(
            screenSettings.appPreferencesView.dnsSettingsView
                .CUSTOM_DNS_INPUT.visible(),
            'text'),
        '1.2.3.4');

    await setQueryProperty(
        screenSettings.appPreferencesView.dnsSettingsView
            .CUSTOM_DNS_INPUT.visible(),
        'text', '1.2.3.4aabbcc');

    await waitForQueryAndClick(screenSettings.BACK.visible());
    await waitForQuery(screenSettings.STACKVIEW.ready());

    // We keep the custom DNS.
    equal(await getSetting('dnsProviderFlags'), 1);
  });

  it('Checking the languages settings', async () => {
    await setSetting('languageCode', '');

    await waitForQueryAndClick(
        screenSettings.APP_PREFERENCES.visible());
    await waitForQuery(screenSettings.STACKVIEW.ready());

    await waitForQueryAndClick(
        screenSettings.appPreferencesView.LANGUAGE.visible());
    await waitForQuery(screenSettings.STACKVIEW.ready());

    await waitForQuery(
        screenSettings.appPreferencesView.languageSettingsView
            .SYSTEM_LANGUAGE_RADIO_BUTTON.visible()
            .checked());

    await scrollToQuery(
        screenSettings.appPreferencesView.languageSettingsView.SCREEN,
        screenSettings.appPreferencesView.languageSettingsView
            .languageItem('it'));

    await waitForQueryAndClick(
        screenSettings.appPreferencesView.languageSettingsView
            .languageItemLabel('it'));

    await waitForQuery(
        screenSettings.appPreferencesView.languageSettingsView
            .SYSTEM_LANGUAGE_RADIO_BUTTON.visible()
            .unchecked());

    await waitForQueryAndClick(screenSettings.BACK.visible());
    await waitForQuery(screenSettings.STACKVIEW.ready());

    await waitForQueryAndClick(screenSettings.BACK.visible());
    await waitForQuery(screenSettings.STACKVIEW.ready());

    await waitForQuery(screenSettings.USER_PROFILE.visible());

    await waitForQueryAndClick(
        screenSettings.APP_PREFERENCES.visible());
    await waitForQuery(screenSettings.STACKVIEW.ready());

    await waitForQueryAndClick(
        screenSettings.appPreferencesView.LANGUAGE.visible().prop(
            'settingTitle', 'Lingua'));
    await waitForQuery(screenSettings.STACKVIEW.ready());

    await waitForQuery(screenSettings.BACK.visible());
    await waitForQuery(
        screenSettings.appPreferencesView.languageSettingsView
            .SYSTEM_LANGUAGE_RADIO_BUTTON.visible()
            .unchecked());

    await scrollToQuery(
        screenSettings.appPreferencesView.languageSettingsView.SCREEN,
        screenSettings.appPreferencesView.languageSettingsView
            .languageItem('en'));

    await waitForQueryAndClick(
        screenSettings.appPreferencesView.languageSettingsView
            .languageItemLabel('en')
            .visible());

    await waitForQueryAndClick(screenSettings.BACK.visible());
    await waitForQuery(screenSettings.STACKVIEW.ready());

    await waitForQueryAndClick(screenSettings.BACK.visible());
    await waitForQuery(screenSettings.STACKVIEW.ready());

    await waitForQuery(screenSettings.USER_PROFILE.visible());

    await waitForQueryAndClick(
        screenSettings.APP_PREFERENCES.visible());
    await waitForQuery(screenSettings.STACKVIEW.ready());

    await waitForQueryAndClick(
        screenSettings.appPreferencesView.LANGUAGE.visible().prop(
            'settingTitle', 'Language'));
    await waitForQuery(screenSettings.STACKVIEW.ready());

    await setQueryProperty(
        screenSettings.appPreferencesView.languageSettingsView.SCREEN,
        'contentY', 0);

    await waitForQueryAndClick(
        screenSettings.appPreferencesView.languageSettingsView
            .SYSTEM_LANGUAGE_RADIO_BUTTON.visible()
            .unchecked());
    await waitForQuery(
        screenSettings.appPreferencesView.languageSettingsView
            .SYSTEM_LANGUAGE_RADIO_BUTTON.visible()
            .checked());

    await waitForQueryAndClick(screenSettings.BACK.visible());
    await waitForQuery(screenSettings.STACKVIEW.ready());

    await waitForQueryAndClick(screenSettings.BACK.visible());
    await waitForQuery(screenSettings.STACKVIEW.ready());

    await waitForQuery(screenSettings.USER_PROFILE.visible());
  });

  // TODO: app-permission

  it('Checking the about us', async () => {
    await waitForQuery(screenSettings.ABOUT_US.visible());

    await scrollToQuery(
        screenSettings.SCREEN, screenSettings.ABOUT_US);

    await clickOnQuery(screenSettings.ABOUT_US.visible());
    await waitForQuery(screenSettings.STACKVIEW.ready());

    await waitForQuery(screenSettings.BACK.visible());

    await waitForQuery(screenSettings.aboutUsView.SCREEN);
    await waitForQuery(screenSettings.aboutUsView.LIST);
    await waitForQuery(screenSettings.aboutUsView.TOS.visible());
    await waitForQuery(
        screenSettings.aboutUsView.PRIVACY.visible());
    await waitForQuery(
        screenSettings.aboutUsView.LICENSE.visible());

    await waitForQueryAndClick(
        screenSettings.aboutUsView.TOS.visible());
    await waitForCondition(async () => {
      const url = await getLastUrl();
      return url.endsWith('/r/vpn/terms');
    });

    await waitForQueryAndClick(
        screenSettings.aboutUsView.PRIVACY.visible());
    await waitForCondition(async () => {
      const url = await getLastUrl();
      return url.endsWith('/r/vpn/privacy');
    });

    await waitForQueryAndClick(
        screenSettings.aboutUsView.LICENSE.visible());

    await clickOnQuery(screenSettings.BACK.visible());
    await waitForQuery(screenSettings.STACKVIEW.ready());

    await clickOnQuery(screenSettings.BACK.visible());
    await waitForQuery(screenSettings.STACKVIEW.ready());

    await waitForQuery(screenSettings.USER_PROFILE.visible());
  });

  it('Checking the get help', async () => {
    await waitForQuery(screenSettings.GET_HELP.visible());

    await scrollToQuery(
        screenSettings.SCREEN, screenSettings.GET_HELP);

    await clickOnQuery(screenSettings.GET_HELP.visible());
    await waitForQuery(global.SCREEN_LOADER.ready());

    await waitForQuery(screenGetHelp.LINKS.visible());

    await waitForQueryAndClick(screenGetHelp.LOGS.visible());

    await waitForCondition(async () => {
      const url = await getLastUrl();
      return url.startsWith('file://') && url.includes('mozillavpn') &&
          url.endsWith('.log');
    });

    await waitForQueryAndClick(screenGetHelp.HELP_CENTER.visible());
    await waitForCondition(async () => {
      const url = await getLastUrl();
      return url.endsWith('/firefox-private-network-vpn');
    });

    await waitForQueryAndClick(screenGetHelp.BACK_BUTTON.visible());
    await waitForQuery(global.SCREEN_LOADER.ready());

    await waitForQuery(screenSettings.GET_HELP.visible());
  });

  it('Get help is opened and closed', async () => {
    await getToGetHelpView();

    await waitForQueryAndClick(screenGetHelp.BACK_BUTTON.visible());
    await waitForQuery(global.SCREEN_LOADER.ready());

    await waitForQuery(screenSettings.GET_HELP.visible());
  });

  it('Contact us is opened and closed', async () => {
    await getToGetHelpView();
    await waitForQueryAndClick(screenGetHelp.SUPPORT.visible());

    await waitForQueryAndClick(
        screenGetHelp.contactSupportView.USER_INFO.visible());
  });

  it('Checking the preferences settings', async () => {
    await waitForQuery(screenSettings.APP_PREFERENCES.visible());

    await scrollToQuery(
        screenSettings.SCREEN, screenSettings.APP_PREFERENCES);

    await clickOnQuery(screenSettings.APP_PREFERENCES.visible());

    await checkSetting(
        screenSettings.appPreferencesView.START_AT_BOOT, 'startAtBoot');

    await checkSetting(
        screenSettings.appPreferencesView.DATA_COLLECTION,
        'gleanEnabled');

    await waitForQuery(
        screenSettings.appPreferencesView.LANGUAGE.visible());
    await waitForQuery(
        screenSettings.appPreferencesView.DNS_SETTINGS.visible());

    await waitForQueryAndClick(screenSettings.BACK.visible());
    await waitForQuery(screenSettings.STACKVIEW.ready());

    await waitForQuery(screenSettings.USER_PROFILE.visible());
  });

  it('Checking the notifications settings', async () => {
    await waitForQuery(screenSettings.APP_PREFERENCES.visible());

    await scrollToQuery(
        screenSettings.SCREEN, screenSettings.APP_PREFERENCES);

    await clickOnQuery(screenSettings.APP_PREFERENCES.visible());

    await waitForQuery(
        screenSettings.appPreferencesView.NOTIFICATIONS.visible());

    await scrollToQuery(
        screenSettings.SCREEN,
        screenSettings.appPreferencesView.NOTIFICATIONS);

    await clickOnQuery(
        screenSettings.appPreferencesView.NOTIFICATIONS.visible());

    await checkSetting(
        screenSettings.appPreferencesView.notificationView
            .UNSECURE_NETWORK_ALERT,
        'unsecuredNetworkAlert');
    await checkSetting(
        screenSettings.appPreferencesView.notificationView
            .SWITCH_SERVER_ALERT,
        'serverSwitchNotification');
    await checkSetting(
        screenSettings.appPreferencesView.notificationView
            .CONNECTION_CHANGE_ALERT,
        'connectionChangeNotification');
    await checkSetting(
        screenSettings.appPreferencesView.notificationView
            .SERVER_UNAVAILABLE_ALERT,
        'serverUnavailableNotification');

    await waitForQueryAndClick(screenSettings.BACK.visible());
    await waitForQuery(screenSettings.STACKVIEW.ready());

    await waitForQueryAndClick(screenSettings.BACK.visible());
    await waitForQuery(screenSettings.STACKVIEW.ready());

    await waitForQuery(screenSettings.USER_PROFILE.visible());
  });

  it('Checking the logout', async () => {
    await waitForQuery(screenSettings.SIGN_OUT.visible());
    await scrollToQuery(
        screenSettings.SCREEN,
        screenSettings.SIGN_OUT.visible());

    await clickOnQuery(screenSettings.SIGN_OUT.visible());
    await waitForInitialView();
  });

  it('Checking Developer Menu Reset and Quit', async () => {
    // WASM is failing at relaunching the app, so skip this test on WASM
    if (this.ctx.wasm) {
      return;
    }

    // magically unlock dev menu
    await setSetting('developerUnlock', 'true');

    // navigate to Developer Menu
    await getToGetHelpView();
    await waitForQueryAndClick(
        screenGetHelp.DEVELOPER_MENU.visible());

    // click "reset and quit" 6 times, test will fail if app quits early
    await waitForQuery(
        screenDeveloperMenu.RESET_AND_QUIT_BUTTON.visible());
    await scrollToQuery(
        screenDeveloperMenu.SCREEN,
        screenDeveloperMenu.RESET_AND_QUIT_BUTTON);
    await waitForQueryAndClick(
        screenDeveloperMenu.RESET_AND_QUIT_BUTTON.visible());
    await waitForQueryAndClick(
        screenDeveloperMenu.RESET_AND_QUIT_BUTTON.visible());
    await waitForQueryAndClick(
        screenDeveloperMenu.RESET_AND_QUIT_BUTTON.visible());
    await waitForQueryAndClick(
        screenDeveloperMenu.RESET_AND_QUIT_BUTTON.visible());
    await waitForQueryAndClick(
        screenDeveloperMenu.RESET_AND_QUIT_BUTTON.visible());
    // can't use waitForQueryAndClick for final click because it returns an
    // error - as expected, we crashed the app - but that causes test to fail
    await clickOnQueryAndAcceptAnyResults(
        screenDeveloperMenu.RESET_AND_QUIT_BUTTON.visible());

    // Confirm the app quit
    equal(vpnIsInactive(), true);

    // relaunch app
    await startAndConnect();
    await setSetting('tipsAndTricksIntroShown', 'true');
    await setSetting('localhostRequestsOnly', 'true');
    await authenticateInApp(true, true);
    await setGleanAutomationHeader();

    // turn on VPN
    await activateViaToggle();
    await awaitSuccessfulConnection();
  });

  describe('telemetry in the settings menu', function () {
    this.ctx.authenticationNeeded = true;

    beforeEach(async () => {
      await waitForQueryAndClick(navBar.SETTINGS.visible());
    });

    it("record telemetry when user clicks on their account details", async () => {
        if (this.ctx.wasm) {
            // This test cannot run in wasm
            return;
        }
        await waitForQueryAndClick(screenSettings.USER_PROFILE.visible());
        const events = await gleanTestGetValue("interaction", "accountSelected", "main");

        equal(events.length, 1);
        var element = events[0];
        equal(element.extra.screen, "settings");
    });

    it("record telemetry when user clicks on Privacy features", async () => {
        if (this.ctx.wasm) {
            // This test cannot run in wasm
            return;
        }
        await waitForQueryAndClick(screenSettings.PRIVACY.visible());
        const events = await gleanTestGetValue("interaction", "privacyFeaturesSelected", "main");

        equal(events.length, 1);
        var element = events[0];
        equal(element.extra.screen, "settings");
    });

    it("record telemetry when user clicks on Tips and tricks", async () => {
        if (this.ctx.wasm) {
            // This test cannot run in wasm
            return;
        }
        await waitForQueryAndClick(screenSettings.TIPS_AND_TRICKS.visible());
        const events = await gleanTestGetValue("interaction", "tipsAndTricksSelected", "main");

        equal(events.length, 1);
        var element = events[0];
        equal(element.extra.screen, "settings");
    });

    it("record telemetry when user clicks on My devices", async () => {
        if (this.ctx.wasm) {
            // This test cannot run in wasm
            return;
        }
        await waitForQueryAndClick(screenSettings.MY_DEVICES.visible());
        const events = await gleanTestGetValue("interaction", "myDevicesSelected", "main");

        equal(events.length, 1);
        var element = events[0];
        equal(element.extra.screen, "settings");
    });

    it("record telemetry when user clicks on App preferences", async () => {
        if (this.ctx.wasm) {
            // This test cannot run in wasm
            return;
        }
        await waitForQueryAndClick(screenSettings.APP_PREFERENCES.visible());
        const events = await gleanTestGetValue("interaction", "appPreferencesSelected", "main");

        equal(events.length, 1);
        var element = events[0];
        equal(element.extra.screen, "settings");
    });

    it("record telemetry when user clicks on Sign out in the Settings screen", async () => {
        if (this.ctx.wasm) {
            // This test cannot run in wasm
            return;
        }

        await waitForQuery(screenSettings.SIGN_OUT.visible());
        await scrollToQuery(
            screenSettings.SCREEN,
            screenSettings.SIGN_OUT.visible());
    
        await clickOnQuery(screenSettings.SIGN_OUT.visible());

        const events = await gleanTestGetValue("interaction", "signOutSelected", "main");
        equal(events.length, 1);
        var element = events[0];
        equal(element.extra.screen, "settings");
    });

    it("record telemetry when user goes to the Settings screen", async () => {
        if (this.ctx.wasm) {
            // This test cannot run in wasm
            return;
        }
        
        await waitForQueryAndClick(navBar.SETTINGS.visible());

        const settingsScreenEvent = await gleanTestGetValue("impression", "settingsScreen", "main");
        const settingsScreenEventExtras = settingsScreenEvent[0].extra;
        strictEqual("settings", settingsScreenEventExtras.screen);
    });
  });
});
