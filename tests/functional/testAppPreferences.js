/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { equal, strictEqual } from 'assert';
import { navBar, screenSettings, screenGetHelp, global } from './queries.js';
import { waitForQueryAndClick, isFeatureFlippedOff, flipFeatureOff, waitForQuery, getQueryProperty, getSetting, setSetting, gleanTestGetValue } from './helper.js';

describe('Settings', function() {
  this.timeout(60000);
  this.ctx.authenticationNeeded = true;

  beforeEach(async () => {
    await waitForQueryAndClick(navBar.SETTINGS.visible());

    if (!(await isFeatureFlippedOff('subscriptionManagement'))) {
      await flipFeatureOff('subscriptionManagement');
    }
  });

  describe('telemetry in App preferences in Settings', function () {
    this.ctx.authenticationNeeded = true;

    beforeEach(async () => {
      await waitForQueryAndClick(navBar.SETTINGS.visible());
    });

    it("record telemetry when user clicks on Notifications in App preferences", async () => {
        if (this.ctx.wasm) {
          // This test cannot run in wasm
          return;
        }
        await waitForQueryAndClick(
            screenSettings.APP_PREFERENCES.visible());
        await waitForQuery(screenSettings.STACKVIEW.ready());    
        await waitForQueryAndClick(screenSettings.appPreferencesView.NOTIFICATIONS.visible());
        await waitForQuery(screenSettings.STACKVIEW.ready());
        const events = await gleanTestGetValue("interaction", "notificationsSelected", "main");

        equal(events.length, 1);
        var element = events[0];
        equal(element.extra.screen, "app_preferences");
    });

    it("record telemetry when user clicks on Language in App preferences", async () => {
      if (this.ctx.wasm) {
        // This test cannot run in wasm
        return;
      }
        await waitForQueryAndClick(
            screenSettings.APP_PREFERENCES.visible());
        await waitForQuery(screenSettings.STACKVIEW.ready());    
        await waitForQueryAndClick(screenSettings.appPreferencesView.LANGUAGE.visible());
        await waitForQuery(screenSettings.STACKVIEW.ready());

        const events = await gleanTestGetValue("interaction", "languageSelected", "main");

        equal(events.length, 1);
        var element = events[0];
        equal(element.extra.screen, "app_preferences");
    });

    it("record telemetry when user clicks on DNS settings in App preferences", async () => {
      if (this.ctx.wasm) {
        // This test cannot run in wasm
        return;
      }
        await waitForQueryAndClick(
            screenSettings.APP_PREFERENCES.visible());
        await waitForQuery(screenSettings.STACKVIEW.ready());    
        await waitForQueryAndClick(screenSettings.appPreferencesView.DNS_SETTINGS.visible());
        await waitForQuery(screenSettings.STACKVIEW.ready());

        const events = await gleanTestGetValue("interaction", "dnsSettingsSelected", "main");

        equal(events.length, 1);
        var element = events[0];
        equal(element.extra.screen, "app_preferences");
    });

    it("record telemetry when user opens DNS settings in App preferences", async () => {
      if (this.ctx.wasm) {
        // This test cannot run in wasm
        return;
      }
        await waitForQueryAndClick(
            screenSettings.APP_PREFERENCES.visible());
        await waitForQuery(screenSettings.STACKVIEW.ready());    
        await waitForQueryAndClick(screenSettings.appPreferencesView.DNS_SETTINGS.visible());
        await waitForQuery(screenSettings.STACKVIEW.ready());

        const dnsSettingsScreenEvent = await gleanTestGetValue("impression", "dnsSettingsScreen", "main");
        strictEqual(dnsSettingsScreenEvent.length, 1);
        const dnsSettingsScreenEventExtras = dnsSettingsScreenEvent[0].extra;
        strictEqual("dns_settings", dnsSettingsScreenEventExtras.screen);
    });

    it("record telemetry when user opens Language in App preferences", async () => {
      if (this.ctx.wasm) {
        // This test cannot run in wasm
        return;
      }
        await waitForQueryAndClick(
            screenSettings.APP_PREFERENCES.visible());
        await waitForQuery(screenSettings.STACKVIEW.ready());    
        await waitForQueryAndClick(screenSettings.appPreferencesView.LANGUAGE.visible());
        await waitForQuery(screenSettings.STACKVIEW.ready());

        const languageScreenEvent = await gleanTestGetValue("impression", "languageScreen", "main");
        strictEqual(languageScreenEvent.length, 1);
        const languageScreenEventExtras = languageScreenEvent[0].extra;
        strictEqual("language", languageScreenEventExtras.screen);
    });
  });
});
