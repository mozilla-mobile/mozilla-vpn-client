/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
const assert = require('assert');
const fs = require('fs');
const vpn = require('./helper.js');

describe('Take screenshots for each view', function() {
  this.retries(0);
  this.timeout(200000);
  let languages = [];
  let dir = '';

  async function singleScreenCapture(name, language) {
    const data = await vpn.screenCapture();
    const buffer = Buffer.from(data, 'base64');
    fs.writeFileSync(`${dir}/${name}_${language}.png`, buffer);
  }

  async function screenCapture(name, cb = null) {
    for (let language of languages) {
      await vpn.setSetting('language-code', language);
      // Give time for the app to retranslate the UI. If the number
      // is too low, the UI gets in funny states (part in 1 language, part
      // in another language, ...).
      await new Promise(r => setTimeout(r, 100));
      if (cb) await cb();
      await singleScreenCapture(name, language);
    }
  }

  before(() => {
    // Don't run these tests if no ARTIFACT_DIR
    if (('ARTIFACT_DIR' in process.env)) {
      dir = process.env.ARTIFACT_DIR + '/screencapture';
      if (!fs.existsSync(dir)) {
        fs.mkdirSync(dir);
      }
    } else {
      this.skip();
    }
  });

  beforeEach(async () => {
    // This has to be run as a beforeEach because we
    // need vpn to be running and connected.
    if (languages.length === 0) {
      // Setup the languages and servers
      const codes = await vpn.languages();
      for (let c of codes) {
        if (c !== 'en') languages.push(c);
      }
      // English at the end.
      languages.push('en');
    }
    await vpn.setSetting('language-code', 'en');
  });

  describe('Capture non settings screens', function() {
    it('initial view', async () => {
      await vpn.waitForMainView();
      await screenCapture('initialize');
    });

    it('heartbeat', async () => {
      await screenCapture('heartbeat', async () => {
        await vpn.forceHeartbeatFailure();

        await vpn.waitForElement('heartbeatTryButton');
        await vpn.waitForElementProperty(
            'heartbeatTryButton', 'visible', 'true');
      });
    });

    it('help view', async () => {
      await vpn.clickOnElement('getHelpLink');
      await vpn.waitForElementProperty('getHelpLink', 'visible', 'false');

      await vpn.waitForElement('getHelpBack');
      await vpn.waitForElementProperty('getHelpBack', 'visible', 'true');

      await screenCapture('help');
    });

    it('onboarding', async () => {
      assert(
          await vpn.getElementProperty('learnMoreLink', 'visible') === 'true');
      await vpn.clickOnElement('learnMoreLink');
      await vpn.wait();

      await vpn.waitForElement('skipOnboarding');
      await vpn.waitForElementProperty('skipOnboarding', 'visible', 'true');

      let onboardingView = 0;
      while (true) {
        await vpn.setSetting('language-code', 'en');
        await vpn.wait();
        await screenCapture(`onboarding_${++onboardingView}`);

        assert(await vpn.hasElement('onboardingNext'));
        assert(
            await vpn.getElementProperty('onboardingNext', 'visible') ===
            'true');

        if (await vpn.getElementProperty('onboardingNext', 'text') !== 'Next') {
          break;
        }
        await vpn.clickOnElement('onboardingNext');
        await vpn.wait();
      }
    });

    it('Authenticating', async () => {
      await vpn.waitForMainView();
      await vpn.clickOnElement('getStarted');
      await vpn.waitForCondition(async () => {
        const url = await vpn.getLastUrl();
        return url.includes('/api/v2/vpn/login');
      });

      await screenCapture('authenticating');
    });

    it('Post Authenticate', async () => {
      await vpn.authenticate(false, false);
      await screenCapture('post_authenticate')
    });

    it('Telemetry view', async () => {
      await vpn.authenticate(true, false);
      await screenCapture('telemetry_policy')
    });

    it('main view - logged in', async () => {
      await vpn.authenticate(true, true);
      await screenCapture('vpn_off');
    });

    // The server view is a mess for the screencapture tests
    // skipping for now.
    it.skip('server view', async () => {
      await vpn.authenticate(true, true);
      await vpn.waitForElement('serverListButton');
      await vpn.waitForElementProperty('serverListButton', 'visible', 'true');
      await vpn.clickOnElement('serverListButton');
      await vpn.wait();

      servers = await vpn.servers();

      for (let language of languages) {
        await vpn.setSetting('language-code', language);

        // Let's not open all the server items for now
        for (let server of servers) {
          const countryId = 'serverCountryList/serverCountry-' + server.code;
          await vpn.waitForElement(countryId);
          await vpn.waitForElementProperty(countryId, 'visible', 'true');

          await vpn.setElementProperty(
              'serverCountryView', 'contentY', 'i',
              parseInt(await vpn.getElementProperty(countryId, 'y')));
          await new Promise(r => setTimeout(r, 30));

          if (await vpn.getElementProperty(countryId, 'cityListVisible') ===
              'false') {
            await vpn.clickOnElement(countryId);
            await new Promise(r => setTimeout(r, 30));
          }
        }
        const contentHeight = parseInt(
            await vpn.getElementProperty('serverCountryView', 'contentHeight'))
        const height = parseInt(
            await vpn.getElementProperty('serverCountryView', 'height'));

        await vpn.setElementProperty('serverCountryView', 'contentY', 'i', 0);

        await singleScreenCapture('server', language);

        let contentY = 0;

        let scrollId = 0;
        while (true) {
          if (contentHeight <= (contentY + height)) {
            break;
          }

          contentY += height;
          await vpn.setElementProperty(
              'serverCountryView', 'contentY', 'i', contentY);
          await singleScreenCapture(`server_${++scrollId}`, language);
        }
      }
    });

    it.only('connecting and disconnecting', async () => {
      await vpn.authenticate(true, true);
      for (let language of languages.slice(0, 5)) {
        // Activate
        await vpn.setSetting('language-code', 'en');
        await new Promise(r => setTimeout(r, 100));
        await vpn.waitForCondition(async () => {
          return await vpn.getElementProperty('controllerTitle', 'text') ==
              'VPN is off';
        });
        await vpn.activate();

        // Capture connecting
        await vpn.waitForCondition(async () => {
          let connectingMsg =
              await vpn.getElementProperty('controllerTitle', 'text');
          return connectingMsg === 'Connecting…';
        });
        await vpn.setSetting('language-code', language);
        await new Promise(r => setTimeout(r, 100));
        await singleScreenCapture('vpn_connecting', language);

        // Deactivate
        await vpn.setSetting('language-code', 'en');
        await new Promise(r => setTimeout(r, 100));
        await vpn.waitForCondition(async () => {
          return await vpn.getElementProperty('controllerTitle', 'text') ==
              'VPN is on';
        });
        await vpn.deactivate();

        // Capture disconnecting
        await vpn.waitForCondition(async () => {
          return await vpn.getElementProperty('controllerTitle', 'text') ===
              'Disconnecting…';
        });
        await vpn.setSetting('language-code', language);
        await new Promise(r => setTimeout(r, 100));
        await singleScreenCapture('vpn_disconnecting', language);
      }
    });

    it('connected', async () => {
      await vpn.authenticate(true, true);
      await vpn.activate();
      await vpn.waitForCondition(async () => {
        return await vpn.getElementProperty('controllerTitle', 'text') ==
            'VPN is on';
      });
      await screenCapture('vpn_on');
    });

    it('connection info', async () => {
      await vpn.authenticate(true, true);
      await vpn.activate();
      await vpn.waitForCondition(async () => {
        return await vpn.getElementProperty('controllerTitle', 'text') ==
            'VPN is on';
      });
      await vpn.waitForElement('connectionInfoButton');
      await vpn.clickOnElement('connectionInfoButton');
      await vpn.wait();
      await screenCapture('connection_info');
    });
  });

  describe('Capture settings screens', () => {
    beforeEach(async () => {
      await vpn.authenticate(true, true);
      await vpn.waitForElement('settingsButton');
      await vpn.clickOnElement('settingsButton');
      await vpn.wait();
    });

    it('settings', async () => {
      await screenCapture('settings');

      const contentHeight = parseInt(
          await vpn.getElementProperty('settingsView', 'contentHeight'))
      const height =
          parseInt(await vpn.getElementProperty('settingsView', 'height'));
      let contentY =
          parseInt(await vpn.getElementProperty('settingsView', 'contentY'));

      let scrollId = 0;
      while (true) {
        if (contentHeight <= (contentY + height)) {
          break;
        }

        contentY += height;
        await vpn.setElementProperty('settingsView', 'contentY', 'i', contentY);
        await screenCapture(`settings_${++scrollId}`);
      }
    });

    it('settings / networking', async () => {
      await vpn.waitForElement('settingsNetworking');
      await vpn.waitForElementProperty('settingsNetworking', 'visible', 'true');

      await vpn.setElementProperty(
          'settingsView', 'contentY', 'i',
          parseInt(await vpn.getElementProperty('settingsNetworking', 'y')));
      await vpn.wait();

      await vpn.clickOnElement('settingsNetworking');
      await vpn.wait();

      await screenCapture('settings_networking');
    });


    it('settings / networking / DNS', async () => {
      await vpn.waitForElement('settingsNetworking');
      await vpn.waitForElementProperty('settingsNetworking', 'visible', 'true');

      await vpn.setElementProperty(
          'settingsView', 'contentY', 'i',
          parseInt(await vpn.getElementProperty('settingsNetworking', 'y')));
      await vpn.wait();

      await vpn.clickOnElement('settingsNetworking');
      await vpn.wait();

      await vpn.waitForElement('advancedDNSSettings');
      await vpn.waitForElementProperty(
          'advancedDNSSettings', 'visible', 'true');
      await vpn.clickOnElement('advancedDNSSettings');
      await vpn.wait();

      await screenCapture('dns_settings');
    });

    it('settings / preferences / notifications', async () => {
      await vpn.waitForElement('settingsPreferences');
      await vpn.clickOnElement('settingsPreferences');
      await vpn.wait();

      await vpn.waitForElement('settingsNotifications');
      await vpn.waitForElementProperty(
          'settingsNotifications', 'visible', 'true');

      await vpn.setElementProperty(
          'settingsView', 'contentY', 'i',
          parseInt(await vpn.getElementProperty('settingsNotifications', 'y')));
      await vpn.wait();

      await vpn.clickOnElement('settingsNotifications');
      await vpn.wait();

      await screenCapture('settings_notification');
    });

    it('settings / preferences / languages', async () => {
      await vpn.waitForElement('settingsPreferences');
      await vpn.clickOnElement('settingsPreferences');
      await vpn.wait();

      await vpn.waitForElement('settingsLanguages');
      await vpn.waitForElementProperty('settingsLanguages', 'visible', 'true');
      await vpn.clickOnElement('settingsLanguages');
      await vpn.wait();

      await vpn.setElementProperty(
          'settingsView', 'contentY', 'i',
          parseInt(await vpn.getElementProperty('settingsLanguages', 'y')));
      await vpn.wait();

      await screenCapture('settings_languages');

      const contentHeight = parseInt(await vpn.getElementProperty(
          'settingsLanguagesView', 'contentHeight'))
      const height = parseInt(
          await vpn.getElementProperty('settingsLanguagesView', 'height'));
      let contentY = parseInt(
          await vpn.getElementProperty('settingsLanguagesView', 'contentY'));

      let scrollId = 0;
      while (true) {
        if (contentHeight <= (contentY + height)) {
          break;
        }

        contentY += height;
        await vpn.setElementProperty(
            'settingsLanguagesView', 'contentY', 'i', contentY);
        await screenCapture(`settings_languages_${++scrollId}`);
      }
    });

    // TODO: app-permission

    it('settings / about us', async () => {
      await vpn.waitForElement('settingsAboutUs');
      await vpn.waitForElementProperty('settingsAboutUs', 'visible', 'true');

      await vpn.setElementProperty(
          'settingsView', 'contentY', 'i',
          parseInt(await vpn.getElementProperty('settingsAboutUs', 'y')));
      await vpn.wait();

      await vpn.clickOnElement('settingsAboutUs');
      await vpn.wait();

      await screenCapture('settings_about');
    });

    it('settings / help', async () => {
      await vpn.waitForElement('settingsGetHelp');
      await vpn.waitForElementProperty('settingsGetHelp', 'visible', 'true');

      await vpn.setElementProperty(
          'settingsView', 'contentY', 'i',
          parseInt(await vpn.getElementProperty('settingsGetHelp', 'y')));
      await vpn.wait();

      await vpn.clickOnElement('settingsGetHelp');
      await vpn.wait();

      await screenCapture('settings_help');
    });
    it('settings / help / support', async () => {
      await vpn.waitForElement('settingsGetHelp');
      await vpn.waitForElementProperty('settingsGetHelp', 'visible', 'true');

      await vpn.setElementProperty(
          'settingsView', 'contentY', 'i',
          parseInt(await vpn.getElementProperty('settingsGetHelp', 'y')));
      await vpn.wait();

      await vpn.clickOnElement('settingsGetHelp');
      await vpn.wait();

      await vpn.waitForElement('settingsGiveFeedback');
      await vpn.waitForElementProperty(
          'settingsGiveFeedback', 'visible', 'true');
      await vpn.clickOnElement('settingsGiveFeedback');
      await vpn.wait();

      await screenCapture('settings_give_feedback');
    });
  });
});
