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
        (await vpn.getQueryProperty(query, 'isChecked') === 'true'),
        await vpn.getSetting(settingKey));

    await vpn.setSetting(settingKey, true);
    assert.equal((await vpn.getSetting(settingKey)), true);
    assert.equal((await vpn.getQueryProperty(query, 'isChecked')), 'true');

    await vpn.setSetting(settingKey, false);
    assert.equal((await vpn.getSetting(settingKey)), false);
    assert.equal((await vpn.getQueryProperty(query, 'isChecked')), 'false');
  }

  async function getToGetHelpView() {
    await vpn.waitForQueryAndClick(queries.screenSettings.GET_HELP.visible());
    await vpn.waitForQuery(queries.screenGetHelp.BACK_BUTTON.visible());
    await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
  }

  describe('telemetry in App preferences in Settings', function () {
    this.ctx.authenticationNeeded = true;

    beforeEach(async () => {
      await vpn.waitForQueryAndClick(queries.navBar.SETTINGS.visible());
    });

    it.only("record telemetry when user enables and disables connecting on startup", async () => {
        // Startup on boot is not enabled by default so the first click will enable it
        await vpn.waitForQueryAndClick(
            queries.screenSettings.APP_PREFERENCES.visible());
        await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());    
        await vpn.waitForQueryAndClick(queries.screenSettings.appPreferencesView.START_AT_BOOT.visible());
        await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

        // Startup on boot is not enabled by default so the first click will enable it
        await vpn.waitForQuery(
            queries.screenSettings.appPreferencesView.START_AT_BOOT
                .visible()
                .prop('checked', false));
    
        const events = await vpn.gleanTestGetValue("interaction", "vpnOnStartupEnabled", "main");
        assert.equal(events.length, 1);
        var element = events[0];
        assert.equal(element.extra.screen, "app_preferences");

        //Disable startup on boot and check that we sent the telemetry
        await vpn.waitForQueryAndClick(queries.screenSettings.appPreferencesView.START_AT_BOOT.visible());

        events = await vpn.gleanTestGetValue("interaction", "vpnOnStartupDisabled", "main");
        assert.equal(events.length, 1);
        var element = events[0];
        assert.equal(element.extra.screen, "app_preferences");
    });

    it("record telemetry when user enables and disables telemetry", async () => {
        await vpn.waitForQueryAndClick(
            queries.screenSettings.APP_PREFERENCES.visible());
        await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());    
        await vpn.waitForQueryAndClick(queries.screenSettings.appPreferencesView.DATA_COLLECTION.visible());
        await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

        const events = await vpn.gleanTestGetValue("interaction", "dataCollectionEnabled", "main");

        assert.equal(events.length, 1);
        var element = events[0];
        assert.equal(element.extra.screen, "app_preferences");

        //Disable data collection and check that we sent the telemetry
        await vpn.waitForQueryAndClick(queries.screenSettings.appPreferencesView.START_AT_BOOT.visible());
        events = await vpn.gleanTestGetValue("interaction", "dataCollectionDisabled", "main");

        assert.equal(events.length, 1);
        var element = events[0];
        assert.equal(element.extra.screen, "app_preferences");
    });

    it("record telemetry when user clicks on Notifications in App preferences", async () => {
        await vpn.waitForQueryAndClick(
            queries.screenSettings.APP_PREFERENCES.visible());
        await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());    
        await vpn.waitForQueryAndClick(queries.screenSettings.appPreferencesView.NOTIFICATIONS.visible());
        await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());
        const events = await vpn.gleanTestGetValue("interaction", "notificationsSelected", "main");

        assert.equal(events.length, 1);
        var element = events[0];
        assert.equal(element.extra.screen, "app_preferences");
    });

    it("record telemetry when user clicks on Language in App preferences", async () => {
        await vpn.waitForQueryAndClick(
            queries.screenSettings.APP_PREFERENCES.visible());
        await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());    
        await vpn.waitForQueryAndClick(queries.screenSettings.appPreferencesView.LANGUAGE.visible());
        await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

        const events = await vpn.gleanTestGetValue("interaction", "languageSelected", "main");

        assert.equal(events.length, 1);
        var element = events[0];
        assert.equal(element.extra.screen, "app_preferences");
    });

    it("record telemetry when user clicks on DNS settings in App preferences", async () => {
        await vpn.waitForQueryAndClick(
            queries.screenSettings.APP_PREFERENCES.visible());
        await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());    
        await vpn.waitForQueryAndClick(queries.screenSettings.appPreferencesView.DNS_SETTINGS.visible());
        await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

        const events = await vpn.gleanTestGetValue("interaction", "dnsSettingsSelected", "main");

        assert.equal(events.length, 1);
        var element = events[0];
        assert.equal(element.extra.screen, "app_preferences");
    });

    it("record telemetry when user opens DNS settings in App preferences", async () => {
        await vpn.waitForQueryAndClick(
            queries.screenSettings.APP_PREFERENCES.visible());
        await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());    
        await vpn.waitForQueryAndClick(queries.screenSettings.appPreferencesView.DNS_SETTINGS.visible());
        await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

        const [ { extra: DnsSettingsScreenExtra } ] = await vpn.gleanTestGetValue("impression", "dnsSettingsScreen", "main");
        assert.strictEqual("main", DnsSettingsScreenExtra.screen);
        assert.strictEqual("impression", DnsSettingsScreenExtra.action);
    });

    it("record telemetry when user opens Language in App preferences", async () => {
        await vpn.waitForQueryAndClick(
            queries.screenSettings.APP_PREFERENCES.visible());
        await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());    
        await vpn.waitForQueryAndClick(queries.screenSettings.appPreferencesView.LANGUAGE.visible());
        await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

        const [ { extra: LanguageScreenExtra } ] = await vpn.gleanTestGetValue("impression", "languageScreen", "main")
        assert.strictEqual("language", LanguageScreenExtra.screen);
        assert.strictEqual("impression", LanguageScreenExtra.action);
    });
  });
});
