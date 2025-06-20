/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const queries = require('./queries.js');
const vpn = require('./helper.js');
const setup = require('./setupVpn.js');

describe('Settings', function() {
  this.timeout(60000);
  this.ctx.authenticationNeeded = true;

  beforeEach(async () => {
    await vpn.waitForQueryAndClick(queries.navBar.SETTINGS.visible());

    if (!(await vpn.isFeatureFlippedOff('subscriptionManagement'))) {
      await vpn.flipFeatureOff('subscriptionManagement');
    }
  });

  async function checkSetting(query, settingKey) {
    await vpn.waitForQuery(query.visible());
    assert.equal(
        (await vpn.getQueryProperty(query, 'checked') === 'true'),
        await vpn.getSetting(settingKey));

    await vpn.setSetting(settingKey, true);
    assert.equal((await vpn.getSetting(settingKey)), true);
    assert.equal((await vpn.getQueryProperty(query, 'checked')), 'true');

    await vpn.setSetting(settingKey, false);
    assert.equal((await vpn.getSetting(settingKey)), false);
    assert.equal((await vpn.getQueryProperty(query, 'checked')), 'false');
  }

  async function getToGetHelpView() {
    await vpn.waitForQueryAndClick(queries.screenSettings.GET_HELP.visible());
    await vpn.waitForQuery(queries.screenGetHelp.BACK_BUTTON.visible());
    await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
  }

  it('Checking settings entries', async () => {
    await vpn.waitForQuery(queries.screenSettings.USER_PROFILE.visible());

    if ((await vpn.isFeatureFlippedOff('subscriptionManagement'))) {
      await vpn.clickOnQuery(queries.screenSettings.USER_PROFILE.visible());
      await vpn.waitForCondition(async () => {
        const url = await vpn.getLastUrl();
        return url.includes('https://accounts.stage.mozaws.net') &&
            url.includes('?email=test@mozilla.com');
      });
    }

    await vpn.waitForQuery(queries.screenSettings.PRIVACY.visible());
    if (await vpn.isFeatureFlippedOn('splitTunnel')) {
      await vpn.waitForQuery(queries.screenSettings.APP_EXCLUSIONS.visible());
    }
    await vpn.waitForQuery(queries.screenSettings.MY_DEVICES.visible());
    await vpn.waitForQuery(queries.screenSettings.APP_PREFERENCES.visible());
    await vpn.waitForQuery(queries.screenSettings.GET_HELP.visible());
    await vpn.waitForQuery(queries.screenSettings.ABOUT_US.visible());
  });

  describe('Privacy settings tests', function() {
    beforeEach(async () => {
      await vpn.waitForQuery(queries.screenSettings.PRIVACY.visible());

      await vpn.scrollToQuery(
          queries.screenSettings.SCREEN, queries.screenSettings.PRIVACY);

      await vpn.clickOnQuery(queries.screenSettings.PRIVACY.visible());
      await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());
  });

    it('Checking the privacy settings', async () => {
      // Checking that the `These changes may affect...` warning is visible
      await vpn.waitForQuery(
          queries.screenSettings.privacyView.INFORMATION_CARD.visible());

      // Checking if the checkboxes are correctly set based on the settings prop
      await vpn.setSetting('dnsProviderFlags', 0);
      await vpn.waitForQuery(
          queries.screenSettings.privacyView.BLOCK_ADS_TOGGLE.visible().prop(
              'checked', false));
      await vpn.waitForQuery(
          queries.screenSettings.privacyView.BLOCK_TRACKERS_TOGGLE.visible().prop(
              'checked', false));
      await vpn.waitForQuery(
          queries.screenSettings.privacyView.BLOCK_MALWARE_TOGGLE.visible().prop(
              'checked', false));

      await vpn.setSetting('dnsProviderFlags', 2);
      await vpn.waitForQuery(
          queries.screenSettings.privacyView.BLOCK_ADS_TOGGLE.visible().prop(
              'checked', true));
      await vpn.waitForQuery(
          queries.screenSettings.privacyView.BLOCK_TRACKERS_TOGGLE.visible().prop(
              'checked', false));
      await vpn.waitForQuery(
          queries.screenSettings.privacyView.BLOCK_MALWARE_TOGGLE.visible().prop(
              'checked', false));

      await vpn.setSetting('dnsProviderFlags', 4);
      await vpn.waitForQuery(
          queries.screenSettings.privacyView.BLOCK_ADS_TOGGLE.visible().prop(
              'checked', false));
      await vpn.waitForQuery(
          queries.screenSettings.privacyView.BLOCK_TRACKERS_TOGGLE.visible().prop(
              'checked', true));
      await vpn.waitForQuery(
          queries.screenSettings.privacyView.BLOCK_MALWARE_TOGGLE.visible().prop(
              'checked', false));

      await vpn.setSetting('dnsProviderFlags', 6);
      await vpn.waitForQuery(
          queries.screenSettings.privacyView.BLOCK_ADS_TOGGLE.visible().prop(
              'checked', true));
      await vpn.waitForQuery(
          queries.screenSettings.privacyView.BLOCK_TRACKERS_TOGGLE.visible().prop(
              'checked', true));
      await vpn.waitForQuery(
          queries.screenSettings.privacyView.BLOCK_MALWARE_TOGGLE.visible().prop(
              'checked', false));

      await vpn.setSetting('dnsProviderFlags', 8);
      await vpn.waitForQuery(
          queries.screenSettings.privacyView.BLOCK_ADS_TOGGLE.visible().prop(
              'checked', false));
      await vpn.waitForQuery(
          queries.screenSettings.privacyView.BLOCK_TRACKERS_TOGGLE.visible().prop(
              'checked', false));
      await vpn.waitForQuery(
          queries.screenSettings.privacyView.BLOCK_MALWARE_TOGGLE.visible().prop(
              'checked', true));

      await vpn.setSetting('dnsProviderFlags', 10);
      await vpn.waitForQuery(
          queries.screenSettings.privacyView.BLOCK_ADS_TOGGLE.visible().prop(
              'checked', true));
      await vpn.waitForQuery(
          queries.screenSettings.privacyView.BLOCK_TRACKERS_TOGGLE.visible().prop(
              'checked', false));
      await vpn.waitForQuery(
          queries.screenSettings.privacyView.BLOCK_MALWARE_TOGGLE.visible().prop(
              'checked', true));

      await vpn.setSetting('dnsProviderFlags', 12);
      await vpn.waitForQuery(
          queries.screenSettings.privacyView.BLOCK_ADS_TOGGLE.visible().prop(
              'checked', false));
      await vpn.waitForQuery(
          queries.screenSettings.privacyView.BLOCK_TRACKERS_TOGGLE.visible().prop(
              'checked', true));
      await vpn.waitForQuery(
          queries.screenSettings.privacyView.BLOCK_MALWARE_TOGGLE.visible().prop(
              'checked', true));

      await vpn.setSetting('dnsProviderFlags', 14);
      await vpn.waitForQuery(
          queries.screenSettings.privacyView.BLOCK_ADS_TOGGLE.visible().prop(
              'checked', true));
      await vpn.waitForQuery(
          queries.screenSettings.privacyView.BLOCK_TRACKERS_TOGGLE.visible().prop(
              'checked', true));
      await vpn.waitForQuery(
          queries.screenSettings.privacyView.BLOCK_MALWARE_TOGGLE.visible().prop(
              'checked', true));

      // Tests the clicks
      await vpn.setSetting('dnsProviderFlags', 0);
      await vpn.waitForQueryAndClick(
          queries.screenSettings.privacyView.BLOCK_ADS_TOGGLE.visible());
      assert.equal(await vpn.getSetting('dnsProviderFlags'), 2);

      await vpn.setSetting('dnsProviderFlags', 0);
      await vpn.waitForQueryAndClick(
          queries.screenSettings.privacyView.BLOCK_TRACKERS_TOGGLE.visible());
      assert.equal(await vpn.getSetting('dnsProviderFlags'), 4);

      await vpn.setSetting('dnsProviderFlags', 0);
      await vpn.waitForQueryAndClick(
          queries.screenSettings.privacyView.BLOCK_MALWARE_TOGGLE.visible());
      assert.equal(await vpn.getSetting('dnsProviderFlags'), 8);

      // Let's test the modal
      await vpn.setSetting('dnsProviderFlags', 1);
      await vpn.waitForQueryAndClick(
          queries.screenSettings.privacyView.BLOCK_ADS_TOGGLE.visible());
      assert.equal(await vpn.getSetting('dnsProviderFlags'), 1);
      await vpn.waitForQueryAndClick(
          queries.screenSettings.privacyView.MODAL_SECONDARY_BUTTON.visible());
      await vpn.waitForQuery(
          queries.screenSettings.privacyView.MODAL_LOADER.prop('active', false));
      assert.equal(await vpn.getSetting('dnsProviderFlags'), 1);
      await vpn.waitForQueryAndClick(
          queries.screenSettings.privacyView.BLOCK_ADS_TOGGLE.visible());
      assert.equal(await vpn.getSetting('dnsProviderFlags'), 1);
      await vpn.waitForQueryAndClick(
          queries.screenSettings.privacyView.MODAL_CLOSE_BUTTON.visible());
      await vpn.waitForQuery(
          queries.screenSettings.privacyView.MODAL_LOADER.prop('active', false));
      assert.equal(await vpn.getSetting('dnsProviderFlags'), 1);
      await vpn.waitForQueryAndClick(
          queries.screenSettings.privacyView.BLOCK_ADS_TOGGLE.visible());
      assert.equal(await vpn.getSetting('dnsProviderFlags'), 1);
      await vpn.waitForQueryAndClick(
          queries.screenSettings.privacyView.MODAL_PRIMARY_BUTTON.visible());
      await vpn.waitForQuery(
          queries.screenSettings.privacyView.MODAL_LOADER.prop('active', false));
      assert.equal(await vpn.getSetting('dnsProviderFlags'), 2);

      await vpn.setSetting('dnsProviderFlags', 1);
      await vpn.waitForQueryAndClick(
          queries.screenSettings.privacyView.BLOCK_TRACKERS_TOGGLE.visible());
      assert.equal(await vpn.getSetting('dnsProviderFlags'), 1);
      await vpn.waitForQueryAndClick(
          queries.screenSettings.privacyView.MODAL_SECONDARY_BUTTON.visible());
      await vpn.waitForQuery(
          queries.screenSettings.privacyView.MODAL_LOADER.prop('active', false));
      assert.equal(await vpn.getSetting('dnsProviderFlags'), 1);
      await vpn.waitForQueryAndClick(
          queries.screenSettings.privacyView.BLOCK_TRACKERS_TOGGLE.visible());
      assert.equal(await vpn.getSetting('dnsProviderFlags'), 1);
      await vpn.waitForQueryAndClick(
          queries.screenSettings.privacyView.MODAL_CLOSE_BUTTON.visible());
      await vpn.waitForQuery(
          queries.screenSettings.privacyView.MODAL_LOADER.prop('active', false));
      assert.equal(await vpn.getSetting('dnsProviderFlags'), 1);
      await vpn.waitForQueryAndClick(
          queries.screenSettings.privacyView.BLOCK_TRACKERS_TOGGLE.visible());
      assert.equal(await vpn.getSetting('dnsProviderFlags'), 1);
      await vpn.waitForQueryAndClick(
          queries.screenSettings.privacyView.MODAL_PRIMARY_BUTTON.visible());
      await vpn.waitForQuery(
          queries.screenSettings.privacyView.MODAL_LOADER.prop('active', false));
      assert.equal(await vpn.getSetting('dnsProviderFlags'), 4);

      await vpn.setSetting('dnsProviderFlags', 1);
      await vpn.waitForQueryAndClick(
          queries.screenSettings.privacyView.BLOCK_MALWARE_TOGGLE.visible());
      assert.equal(await vpn.getSetting('dnsProviderFlags'), 1);
      await vpn.waitForQueryAndClick(
          queries.screenSettings.privacyView.MODAL_SECONDARY_BUTTON.visible());
      await vpn.waitForQuery(
          queries.screenSettings.privacyView.MODAL_LOADER.prop('active', false));
      assert.equal(await vpn.getSetting('dnsProviderFlags'), 1);
      await vpn.waitForQueryAndClick(
          queries.screenSettings.privacyView.BLOCK_MALWARE_TOGGLE.visible());
      assert.equal(await vpn.getSetting('dnsProviderFlags'), 1);
      await vpn.waitForQueryAndClick(
          queries.screenSettings.privacyView.MODAL_CLOSE_BUTTON.visible());
      await vpn.waitForQuery(
          queries.screenSettings.privacyView.MODAL_LOADER.prop('active', false));
      assert.equal(await vpn.getSetting('dnsProviderFlags'), 1);
      await vpn.waitForQueryAndClick(
          queries.screenSettings.privacyView.BLOCK_MALWARE_TOGGLE.visible());
      assert.equal(await vpn.getSetting('dnsProviderFlags'), 1);
      await vpn.waitForQueryAndClick(
          queries.screenSettings.privacyView.MODAL_PRIMARY_BUTTON.visible());
      await vpn.waitForQuery(
          queries.screenSettings.privacyView.MODAL_LOADER.prop('active', false));
      assert.equal(await vpn.getSetting('dnsProviderFlags'), 8);

      // Let's go back
      await vpn.waitForQueryAndClick(queries.screenSettings.BACK.visible());
      await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

      await vpn.waitForQuery(queries.screenSettings.USER_PROFILE.visible());

      // Reset
      await vpn.setSetting('dnsProviderFlags', 0);
    });

    it('Checking the privacy help sheet', async () => {
      await vpn.waitForQueryAndClick(queries.screenSettings.privacyView.HELP_BUTTON.visible());
      await vpn.waitForQuery(queries.screenSettings.privacyView.HELP_SHEET.opened());
      await vpn.waitForQueryAndClick(queries.screenSettings.privacyView.HELP_SHEET_LEARN_MORE_BUTTON.visible());
      await vpn.waitForCondition(async () => {
          const url = await vpn.getLastUrl();
          return url === 'https://support.mozilla.org/kb/how-do-i-change-my-privacy-features';
      });
      await vpn.waitForQueryAndClick(queries.screenSettings.privacyView.HELP_SHEET_CLOSE_BUTTON.visible());
      await vpn.waitForQuery(queries.screenSettings.privacyView.HELP_SHEET.closed());
    });
  });

  describe('DNS settings tests', function() {
    beforeEach(async () => {
      await vpn.setSetting('userDNS', '');
      await vpn.setSetting('dnsProviderFlags', 0);

      await vpn.waitForQueryAndClick(
          queries.screenSettings.APP_PREFERENCES.visible());
      await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

      await vpn.waitForQueryAndClick(
          queries.screenSettings.appPreferencesView.DNS_SETTINGS.visible());
      await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());
    });

    it('Checking the DNS settings', async () => {
      // Checking if the checkboxes are correctly set based on the settings prop
      await vpn.waitForQuery(
          queries.screenSettings.appPreferencesView.dnsSettingsView.STANDARD_DNS
              .visible()
              .prop('checked', true));
      await vpn.waitForQuery(
          queries.screenSettings.appPreferencesView.dnsSettingsView.CUSTOM_DNS
              .visible()
              .prop('checked', false));

      await vpn.waitForQueryAndClick(queries.navBar.HOME.visible());
      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());

      await vpn.setSetting('dnsProviderFlags', 1);

      await vpn.waitForQueryAndClick(queries.navBar.SETTINGS.visible());
      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());

      await vpn.waitForQueryAndClick(
          queries.screenSettings.APP_PREFERENCES.visible());
      await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

      await vpn.waitForQueryAndClick(
          queries.screenSettings.appPreferencesView.DNS_SETTINGS.visible());
      await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

      await vpn.waitForQuery(
          queries.screenSettings.appPreferencesView.dnsSettingsView.STANDARD_DNS
              .visible()
              .prop('checked', false));
      await vpn.waitForQuery(
          queries.screenSettings.appPreferencesView.dnsSettingsView.CUSTOM_DNS
              .visible()
              .prop('checked', true));
      await vpn.waitForQuery(queries.screenSettings.appPreferencesView
                                 .dnsSettingsView.CUSTOM_DNS_INPUT.visible()
                                 .prop('hasError', false));
      await vpn.setQueryProperty(
          queries.screenSettings.appPreferencesView.dnsSettingsView
              .CUSTOM_DNS_INPUT.visible(),
          'text', 'wow');
      await vpn.waitForQuery(queries.screenSettings.appPreferencesView
                                 .dnsSettingsView.CUSTOM_DNS_INPUT.visible()
                                 .prop('hasError', true));
      await vpn.setQueryProperty(
          queries.screenSettings.appPreferencesView.dnsSettingsView
              .CUSTOM_DNS_INPUT.visible(),
          'text', '1.2.3.4');
      await vpn.waitForQuery(queries.screenSettings.appPreferencesView
                                 .dnsSettingsView.CUSTOM_DNS_INPUT.visible()
                                 .prop('hasError', false));

      // Check the warning message
      assert.equal(
          await vpn.getQueryProperty(
              queries.screenSettings.appPreferencesView.dnsSettingsView
                  .INFORMATION_CARD_LOADER,
              'active'),
          'false');

      // Check the click
      await vpn.waitForQueryAndClick(
          queries.screenSettings.appPreferencesView.dnsSettingsView.STANDARD_DNS
              .visible()
              .prop('checked', false));

      await vpn.waitForQueryAndClick(queries.navBar.HOME.visible());
      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());

      assert.equal(await vpn.getSetting('dnsProviderFlags'), 0);

      await vpn.waitForQueryAndClick(queries.navBar.SETTINGS.visible());
      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());

      await vpn.waitForQueryAndClick(
          queries.screenSettings.APP_PREFERENCES.visible());
      await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

      await vpn.waitForQueryAndClick(
          queries.screenSettings.appPreferencesView.DNS_SETTINGS.visible());
      await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

      await vpn.waitForQueryAndClick(
          queries.screenSettings.appPreferencesView.dnsSettingsView.CUSTOM_DNS
              .visible()
              .prop('checked', false));

      await vpn.setQueryProperty(
          queries.screenSettings.appPreferencesView.dnsSettingsView
              .CUSTOM_DNS_INPUT.visible(),
          'text', '1.2.3.4');

      await vpn.waitForQueryAndClick(queries.navBar.HOME.visible());
      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());

      assert.equal(await vpn.getSetting('dnsProviderFlags'), 1);

      // Check the modal
      await vpn.setSetting('dnsProviderFlags', 2);

      await vpn.waitForQueryAndClick(queries.navBar.SETTINGS.visible());
      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());

      await vpn.waitForQueryAndClick(
          queries.screenSettings.APP_PREFERENCES.visible());
      await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());


      await vpn.waitForQueryAndClick(
          queries.screenSettings.appPreferencesView.DNS_SETTINGS.visible());
      await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());


      await vpn.waitForQuery(
          queries.screenSettings.appPreferencesView.dnsSettingsView.STANDARD_DNS
              .visible()
              .prop('checked', true));

      await vpn.waitForQueryAndClick(
          queries.screenSettings.appPreferencesView.dnsSettingsView.CUSTOM_DNS
              .visible()
              .prop('checked', false));

      await vpn.waitForQueryAndClick(
          queries.screenSettings.appPreferencesView.dnsSettingsView
              .MODAL_SECONDARY_BUTTON.visible());
      await vpn.waitForQuery(
          queries.screenSettings.appPreferencesView.dnsSettingsView.MODAL_LOADER
              .prop('active', false));

      await vpn.waitForQueryAndClick(queries.screenSettings.appPreferencesView
                                         .dnsSettingsView.CUSTOM_DNS.visible());

      await vpn.waitForQueryAndClick(
          queries.screenSettings.appPreferencesView.dnsSettingsView
              .MODAL_CLOSE_BUTTON.visible());
      await vpn.waitForQuery(
          queries.screenSettings.appPreferencesView.dnsSettingsView.MODAL_LOADER
              .prop('active', false));

      await vpn.waitForQueryAndClick(queries.screenSettings.appPreferencesView
                                         .dnsSettingsView.CUSTOM_DNS.visible());

      await vpn.waitForQueryAndClick(
          queries.screenSettings.appPreferencesView.dnsSettingsView
              .MODAL_PRIMARY_BUTTON.visible());
      await vpn.waitForQuery(
          queries.screenSettings.appPreferencesView.dnsSettingsView.MODAL_LOADER
              .prop('active', false));

      await vpn.setQueryProperty(
          queries.screenSettings.appPreferencesView.dnsSettingsView
              .CUSTOM_DNS_INPUT.visible(),
          'text', '1.2.3.4');

      await vpn.waitForQueryAndClick(queries.screenSettings.BACK.visible());
      await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

      assert.equal(await vpn.getSetting('dnsProviderFlags'), 1);

      await vpn.waitForQueryAndClick(queries.screenSettings.BACK.visible());
      await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

      await vpn.waitForQuery(queries.screenSettings.USER_PROFILE.visible());
    });

    it('Checking the DNS settings reset', async () => {
      // Checking if the checkboxes are correctly set based on the settings prop
      await vpn.waitForQuery(
          queries.screenSettings.appPreferencesView.dnsSettingsView.STANDARD_DNS
              .visible()
              .prop('checked', true));
      await vpn.waitForQuery(
          queries.screenSettings.appPreferencesView.dnsSettingsView.CUSTOM_DNS
              .visible()
              .prop('checked', false));

      // Click on "Custom DNS" but leaving the input field empty.
      await vpn.waitForQueryAndClick(
          queries.screenSettings.appPreferencesView.dnsSettingsView.CUSTOM_DNS
              .visible()
              .prop('checked', false));

      await vpn.setQueryProperty(
          queries.screenSettings.appPreferencesView.dnsSettingsView
              .CUSTOM_DNS_INPUT.visible(),
          'text', '');

      // Going back...
      await vpn.waitForQueryAndClick(queries.screenSettings.BACK.visible());
      await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

      // .. the DNS setting is reset to the default value.
      assert.equal(await vpn.getSetting('dnsProviderFlags'), 0);

      // Same test as before...
      await vpn.waitForQueryAndClick(
          queries.screenSettings.appPreferencesView.DNS_SETTINGS.visible());
      await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

      await vpn.waitForQuery(
          queries.screenSettings.appPreferencesView.dnsSettingsView.STANDARD_DNS
              .visible()
              .prop('checked', true));
      await vpn.waitForQuery(
          queries.screenSettings.appPreferencesView.dnsSettingsView.CUSTOM_DNS
              .visible()
              .prop('checked', false));

      await vpn.waitForQueryAndClick(
          queries.screenSettings.appPreferencesView.dnsSettingsView.CUSTOM_DNS
              .visible()
              .prop('checked', false));

      // But with a valid DNS value...
      await vpn.setQueryProperty(
          queries.screenSettings.appPreferencesView.dnsSettingsView
              .CUSTOM_DNS_INPUT.visible(),
          'text', '1.2.3.4');

      await vpn.waitForQueryAndClick(queries.screenSettings.BACK.visible());
      await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

      // We keep the custom DNS.
      assert.equal(await vpn.getSetting('dnsProviderFlags'), 1);

      // Write something invalid...
      await vpn.waitForQueryAndClick(
          queries.screenSettings.appPreferencesView.DNS_SETTINGS.visible());
      await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

      await vpn.waitForQuery(
          queries.screenSettings.appPreferencesView.dnsSettingsView.STANDARD_DNS
              .visible()
              .prop('checked', false));
      await vpn.waitForQuery(
          queries.screenSettings.appPreferencesView.dnsSettingsView.CUSTOM_DNS
              .visible()
              .prop('checked', true));

      assert.equal(
          await vpn.getQueryProperty(
              queries.screenSettings.appPreferencesView.dnsSettingsView
                  .CUSTOM_DNS_INPUT.visible(),
              'text'),
          '1.2.3.4');

      await vpn.setQueryProperty(
          queries.screenSettings.appPreferencesView.dnsSettingsView
              .CUSTOM_DNS_INPUT.visible(),
          'text', '1.2.3.4aabbcc');

      await vpn.waitForQueryAndClick(queries.screenSettings.BACK.visible());
      await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

      // We keep the custom DNS.
      assert.equal(await vpn.getSetting('dnsProviderFlags'), 1);
    });

    it('Checking the dns help sheet', async () => {
      await vpn.waitForQueryAndClick(
          queries.screenSettings.appPreferencesView.dnsSettingsView.HELP_BUTTON
              .visible());
      await vpn.waitForQuery(queries.screenSettings.appPreferencesView
                                 .dnsSettingsView.HELP_SHEET.opened());
      await vpn.waitForQueryAndClick(
          queries.screenSettings.appPreferencesView.dnsSettingsView
              .HELP_SHEET_LEARN_MORE_BUTTON.visible());
      await vpn.waitForCondition(async () => {
        const url = await vpn.getLastUrl();
        return url ===
            'https://support.mozilla.org/kb/how-do-i-change-my-dns-settings';
      });
      await vpn.waitForQueryAndClick(
          queries.screenSettings.appPreferencesView.dnsSettingsView
              .HELP_SHEET_CLOSE_BUTTON.visible());
      await vpn.waitForQuery(queries.screenSettings.appPreferencesView
                                 .dnsSettingsView.HELP_SHEET.closed());
    });
  });


  it('Checking the languages settings', async () => {
    await vpn.setSetting('languageCode', '');

    await vpn.waitForQueryAndClick(
        queries.screenSettings.APP_PREFERENCES.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    await vpn.waitForQueryAndClick(
        queries.screenSettings.appPreferencesView.LANGUAGE.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    await vpn.waitForQuery(
        queries.screenSettings.appPreferencesView.languageSettingsView
            .SYSTEM_LANGUAGE_RADIO_BUTTON.visible()
            .checked());

    await vpn.scrollToQuery(
        queries.screenSettings.appPreferencesView.languageSettingsView.SCREEN,
        queries.screenSettings.appPreferencesView.languageSettingsView
            .languageItem('it'));

    await vpn.waitForQueryAndClick(
        queries.screenSettings.appPreferencesView.languageSettingsView
            .languageItemLabel('it'));

    await vpn.waitForQuery(
        queries.screenSettings.appPreferencesView.languageSettingsView
            .SYSTEM_LANGUAGE_RADIO_BUTTON.visible()
            .unchecked());

    await vpn.waitForQueryAndClick(queries.screenSettings.BACK.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    await vpn.waitForQueryAndClick(queries.screenSettings.BACK.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    await vpn.waitForQuery(queries.screenSettings.USER_PROFILE.visible());

    await vpn.waitForQueryAndClick(
        queries.screenSettings.APP_PREFERENCES.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    await vpn.waitForQueryAndClick(
        queries.screenSettings.appPreferencesView.LANGUAGE.visible().prop(
            'settingTitle', 'Lingua'));
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    await vpn.waitForQuery(queries.screenSettings.BACK.visible());
    await vpn.waitForQuery(
        queries.screenSettings.appPreferencesView.languageSettingsView
            .SYSTEM_LANGUAGE_RADIO_BUTTON.visible()
            .unchecked());

    await vpn.scrollToQuery(
        queries.screenSettings.appPreferencesView.languageSettingsView.SCREEN,
        queries.screenSettings.appPreferencesView.languageSettingsView
            .languageItem('en'));

    await vpn.waitForQueryAndClick(
        queries.screenSettings.appPreferencesView.languageSettingsView
            .languageItemLabel('en')
            .visible());

    await vpn.waitForQueryAndClick(queries.screenSettings.BACK.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    await vpn.waitForQueryAndClick(queries.screenSettings.BACK.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    await vpn.waitForQuery(queries.screenSettings.USER_PROFILE.visible());

    await vpn.waitForQueryAndClick(
        queries.screenSettings.APP_PREFERENCES.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    await vpn.waitForQueryAndClick(
        queries.screenSettings.appPreferencesView.LANGUAGE.visible().prop(
            'settingTitle', 'Language'));
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    await vpn.setQueryProperty(
        queries.screenSettings.appPreferencesView.languageSettingsView.SCREEN,
        'contentY', 0);

    await vpn.waitForQueryAndClick(
        queries.screenSettings.appPreferencesView.languageSettingsView
            .SYSTEM_LANGUAGE_RADIO_BUTTON.visible()
            .unchecked());
    await vpn.waitForQuery(
        queries.screenSettings.appPreferencesView.languageSettingsView
            .SYSTEM_LANGUAGE_RADIO_BUTTON.visible()
            .checked());

    await vpn.waitForQueryAndClick(queries.screenSettings.BACK.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    await vpn.waitForQueryAndClick(queries.screenSettings.BACK.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    await vpn.waitForQuery(queries.screenSettings.USER_PROFILE.visible());
  });

  it('Checking the appearance settings', async () => {
    const isShowingAutomaticOption =
        await vpn.isFeatureEnabled('themeSelectionIncludesAutomatic');

    await vpn.waitForQueryAndClick(
        queries.screenSettings.APP_PREFERENCES.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    await vpn.waitForQueryAndClick(
        queries.screenSettings.appPreferencesView.APPEARANCE.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    // Initially, automatic should be checked (if available, light mode is
    // default otherwise)
    if (isShowingAutomaticOption) {
      await vpn.waitForQuery(
          queries.screenSettings.appPreferencesView.appearanceView
              .AUTOMATIC_RADIO_BUTTON.visible()
              .checked());

      await vpn.waitForQuery(queries.screenSettings.appPreferencesView
                                 .appearanceView.LIGHT_RADIO_BUTTON.visible()
                                 .unchecked());

      await vpn.waitForQuery(queries.screenSettings.appPreferencesView
                                 .appearanceView.DARK_RADIO_BUTTON.visible()
                                 .unchecked());

      assert.equal((await vpn.getSetting('usingSystemTheme')), true);
    } else {
      await vpn.waitForQuery(queries.screenSettings.appPreferencesView
                                 .appearanceView.LIGHT_RADIO_BUTTON.visible()
                                 .checked());

      await vpn.waitForQuery(queries.screenSettings.appPreferencesView
                                 .appearanceView.DARK_RADIO_BUTTON.visible()
                                 .unchecked());

      assert.equal((await vpn.getSetting('theme')), 'main');
    }

    // Click dark, then confirm
    await vpn.waitForQueryAndClick(
        queries.screenSettings.appPreferencesView.appearanceView
            .DARK_RADIO_BUTTON.visible());

    if (isShowingAutomaticOption) {
      await vpn.waitForQuery(
          queries.screenSettings.appPreferencesView.appearanceView
              .AUTOMATIC_RADIO_BUTTON.visible()
              .unchecked());
      assert.equal((await vpn.getSetting('usingSystemTheme')), false);
    }

    await vpn.waitForQuery(queries.screenSettings.appPreferencesView
                               .appearanceView.LIGHT_RADIO_BUTTON.visible()
                               .unchecked());

    await vpn.waitForQuery(queries.screenSettings.appPreferencesView
                               .appearanceView.DARK_RADIO_BUTTON.visible()
                               .checked());

    assert.equal((await vpn.getSetting('theme')), 'dark-mode');

    // Click light, then confirm
    await vpn.waitForQueryAndClick(
        queries.screenSettings.appPreferencesView.appearanceView
            .LIGHT_RADIO_BUTTON.visible());

    if (isShowingAutomaticOption) {
      await vpn.waitForQuery(
          queries.screenSettings.appPreferencesView.appearanceView
              .AUTOMATIC_RADIO_BUTTON.visible()
              .unchecked());
      assert.equal((await vpn.getSetting('usingSystemTheme')), false);
    }

    await vpn.waitForQuery(queries.screenSettings.appPreferencesView
                               .appearanceView.LIGHT_RADIO_BUTTON.visible()
                               .checked());

    await vpn.waitForQuery(queries.screenSettings.appPreferencesView
                               .appearanceView.DARK_RADIO_BUTTON.visible()
                               .unchecked());

    assert.equal((await vpn.getSetting('theme')), 'main');

    // Click automatic, then confirm
    if (isShowingAutomaticOption) {
      await vpn.waitForQueryAndClick(
          queries.screenSettings.appPreferencesView.appearanceView
              .AUTOMATIC_RADIO_BUTTON.visible());

      await vpn.waitForQuery(
          queries.screenSettings.appPreferencesView.appearanceView
              .AUTOMATIC_RADIO_BUTTON.visible()
              .checked());

      await vpn.waitForQuery(queries.screenSettings.appPreferencesView
                                 .appearanceView.LIGHT_RADIO_BUTTON.visible()
                                 .unchecked());

      await vpn.waitForQuery(queries.screenSettings.appPreferencesView
                                 .appearanceView.DARK_RADIO_BUTTON.visible()
                                 .unchecked());

      assert.equal((await vpn.getSetting('usingSystemTheme')), true);
    }
  });

  // TODO: app-permission

  it('Checking the about us', async () => {
    await vpn.waitForQuery(queries.screenSettings.ABOUT_US.visible());

    await vpn.scrollToQuery(
        queries.screenSettings.SCREEN, queries.screenSettings.ABOUT_US);

    await vpn.clickOnQuery(queries.screenSettings.ABOUT_US.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    await vpn.waitForQuery(queries.screenSettings.BACK.visible());

    await vpn.waitForQuery(queries.screenSettings.aboutUsView.SCREEN);
    await vpn.waitForQuery(queries.screenSettings.aboutUsView.LIST);
    await vpn.waitForQuery(queries.screenSettings.aboutUsView.TOS.visible());
    await vpn.waitForQuery(
        queries.screenSettings.aboutUsView.PRIVACY.visible());
    await vpn.waitForQuery(
        queries.screenSettings.aboutUsView.LICENSE.visible());

    await vpn.waitForQueryAndClick(
        queries.screenSettings.aboutUsView.TOS.visible());
    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.endsWith('/r/vpn/terms');
    });

    await vpn.waitForQueryAndClick(
        queries.screenSettings.aboutUsView.PRIVACY.visible());
    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.endsWith('/r/vpn/privacy');
    });

    await vpn.waitForQueryAndClick(
        queries.screenSettings.aboutUsView.LICENSE.visible());

    await vpn.clickOnQuery(queries.screenSettings.BACK.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    await vpn.clickOnQuery(queries.screenSettings.BACK.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    await vpn.waitForQuery(queries.screenSettings.USER_PROFILE.visible());
  });

  it('Checking the Help', async () => {
    await vpn.waitForQuery(queries.screenSettings.GET_HELP.visible());

    await vpn.scrollToQuery(
        queries.screenSettings.SCREEN, queries.screenSettings.GET_HELP);

    await vpn.clickOnQuery(queries.screenSettings.GET_HELP.visible());
    await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());

    await vpn.waitForQuery(queries.screenGetHelp.LINKS.visible());

    await vpn.waitForQueryAndClick(queries.screenGetHelp.LOGS.visible());

    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.startsWith('file://') && url.includes('mozillavpn') &&
          url.endsWith('.log');
    });

    await vpn.waitForQueryAndClick(queries.screenGetHelp.HELP_CENTER.visible());
    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.endsWith('/firefox-private-network-vpn');
    });

    await vpn.waitForQueryAndClick(queries.screenGetHelp.BACK_BUTTON.visible());
    await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());

    await vpn.waitForQuery(queries.screenSettings.GET_HELP.visible());
  });

  it('Help is opened and closed', async () => {
    await getToGetHelpView();

    await vpn.waitForQueryAndClick(queries.screenGetHelp.BACK_BUTTON.visible());
    await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());

    await vpn.waitForQuery(queries.screenSettings.GET_HELP.visible());
  });

  it('Contact us is opened and closed', async () => {
    await getToGetHelpView();
    await vpn.waitForQueryAndClick(queries.screenGetHelp.SUPPORT.visible());

    await vpn.waitForQueryAndClick(
        queries.screenGetHelp.contactSupportView.USER_INFO.visible());
  });

  it('Checking the preferences settings', async () => {
    await vpn.waitForQuery(queries.screenSettings.APP_PREFERENCES.visible());

    await vpn.scrollToQuery(
        queries.screenSettings.SCREEN, queries.screenSettings.APP_PREFERENCES);

    await vpn.clickOnQuery(queries.screenSettings.APP_PREFERENCES.visible());

    await checkSetting(
        queries.screenSettings.appPreferencesView.START_AT_BOOT_TOGGLE,
        'startAtBoot');

    await checkSetting(
        queries.screenSettings.appPreferencesView.DATA_COLLECTION_TOGGLE,
        'gleanEnabled');

    await vpn.waitForQuery(
        queries.screenSettings.appPreferencesView.LANGUAGE.visible());
    await vpn.waitForQuery(
        queries.screenSettings.appPreferencesView.DNS_SETTINGS.visible());

    await vpn.waitForQueryAndClick(queries.screenSettings.BACK.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    await vpn.waitForQuery(queries.screenSettings.USER_PROFILE.visible());
  });

  it('Checking the notifications settings', async () => {
    await vpn.waitForQuery(queries.screenSettings.APP_PREFERENCES.visible());

    await vpn.scrollToQuery(
        queries.screenSettings.SCREEN, queries.screenSettings.APP_PREFERENCES);

    await vpn.clickOnQuery(queries.screenSettings.APP_PREFERENCES.visible());

    await vpn.waitForQuery(
        queries.screenSettings.appPreferencesView.NOTIFICATIONS.visible());

    await vpn.scrollToQuery(
        queries.screenSettings.SCREEN,
        queries.screenSettings.appPreferencesView.NOTIFICATIONS);

    await vpn.clickOnQuery(
        queries.screenSettings.appPreferencesView.NOTIFICATIONS.visible());

    await checkSetting(
        queries.screenSettings.appPreferencesView.notificationView
            .UNSECURE_NETWORK_ALERT,
        'unsecuredNetworkAlert');
    await checkSetting(
        queries.screenSettings.appPreferencesView.notificationView
            .SWITCH_SERVER_ALERT,
        'serverSwitchNotification');
    await checkSetting(
        queries.screenSettings.appPreferencesView.notificationView
            .CONNECTION_CHANGE_ALERT,
        'connectionChangeNotification');
    await checkSetting(
        queries.screenSettings.appPreferencesView.notificationView
            .SERVER_UNAVAILABLE_ALERT,
        'serverUnavailableNotification');

    await vpn.waitForQueryAndClick(queries.screenSettings.BACK.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    await vpn.waitForQueryAndClick(queries.screenSettings.BACK.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    await vpn.waitForQuery(queries.screenSettings.USER_PROFILE.visible());
  });
});
