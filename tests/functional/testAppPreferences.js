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

    it("record telemetry when user clicks on Notifications in App preferences", async () => {
        if (this.ctx.wasm) {
          // This test cannot run in wasm
          return;
        }
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
      if (this.ctx.wasm) {
        // This test cannot run in wasm
        return;
      }
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
      if (this.ctx.wasm) {
        // This test cannot run in wasm
        return;
      }
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
      if (this.ctx.wasm) {
        // This test cannot run in wasm
        return;
      }
        await vpn.waitForQueryAndClick(
            queries.screenSettings.APP_PREFERENCES.visible());
        await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());    
        await vpn.waitForQueryAndClick(queries.screenSettings.appPreferencesView.DNS_SETTINGS.visible());
        await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

        const dnsSettingsScreenEvent = await vpn.gleanTestGetValue("impression", "dnsSettingsScreen", "main");
        assert.strictEqual(dnsSettingsScreenEvent.length, 1);
        const dnsSettingsScreenEventExtras = dnsSettingsScreenEvent[0].extra;
        assert.strictEqual("dns_settings", dnsSettingsScreenEventExtras.screen);
    });

    it("record telemetry when user opens Language in App preferences", async () => {
      if (this.ctx.wasm) {
        // This test cannot run in wasm
        return;
      }
        await vpn.waitForQueryAndClick(
            queries.screenSettings.APP_PREFERENCES.visible());
        await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());    
        await vpn.waitForQueryAndClick(queries.screenSettings.appPreferencesView.LANGUAGE.visible());
        await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

        const languageScreenEvent = await vpn.gleanTestGetValue("impression", "languageScreen", "main");
        assert.strictEqual(languageScreenEvent.length, 1);
        const languageScreenEventExtras = languageScreenEvent[0].extra;
        assert.strictEqual("language", languageScreenEventExtras.screen);
    });
  });
});
