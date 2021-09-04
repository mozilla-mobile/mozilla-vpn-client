/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const fs = require('fs');
const util = require('util');
const vpn = require('./helper.js');

const dir = process.env.ARTIFACT_DIR + '/screencapture';

describe('Take screenshots for each view', function() {
  let languages = [];
  let servers;

  this.timeout(2000000);

  async function singleScreenCapture(name, language) {
    const data = await vpn.screenCapture();
    const buffer = Buffer.from(data, 'base64');
    fs.writeFileSync(`${dir}/${name}_${language}.png`, buffer);
  }


  async function screenCapture(name, cb = null) {
    for (let language of languages) {
      await vpn.setSetting('language-code', language);

      if (cb) await cb();

      // we need to give time to the app to retranslate the UI. If the number
      // is too slow we have the UI in funny states (part in 1 language, part
      // in another language, ...). But if the number is too high, the
      // "connecting" state is faster and we do not take all the screen
      // captures for all the languages.
      await new Promise(r => setTimeout(r, 30));

      await singleScreenCapture(name, language);
    }
  }

  before(async () => {
    await vpn.connect();

    if (!fs.existsSync(dir)) {
      fs.mkdirSync(dir);
    }
  });

  beforeEach(() => {});

  afterEach(vpn.dumpFailure);

  after(async () => {
    vpn.disconnect();
  });

  it('Retrieve the list of languages', async () => {
    const codes = await vpn.languages();
    for (let c of codes) {
      if (c !== 'en') languages.push(c);
    }

    // English at the end.
    languages.push('en');
  });

  it('reset the app', async () => await vpn.reset());

  it('initial view', async () => {
    await vpn.waitForElement('getHelpLink');
    await vpn.waitForElementProperty('getHelpLink', 'visible', 'true');
    await vpn.wait();

    await screenCapture('initialize');
  });

  it('heartbeat', async () => {
    await screenCapture('heartbeat', async () => {
      await vpn.reset();
      await vpn.forceHeartbeatFailure();

      await vpn.waitForElement('heartbeatTryButton');
      await vpn.waitForElementProperty('heartbeatTryButton', 'visible', 'true');
    });

    await vpn.wait();

    await vpn.clickOnElement('heartbeatTryButton');
    await vpn.wait();

    await vpn.waitForElement('getHelpLink');
    await vpn.waitForElementProperty('getHelpLink', 'visible', 'true');
  });

  it('help view', async () => {
    await vpn.clickOnElement('getHelpLink');
    await vpn.waitForElementProperty('getHelpLink', 'visible', 'false');

    await vpn.waitForElement('getHelpBack');
    await vpn.waitForElementProperty('getHelpBack', 'visible', 'true');

    await screenCapture('help');
  });

  it('Go back to the main view', async () => {
    await vpn.clickOnElement('getHelpBack');

    await vpn.waitForElement('getHelpLink');
    await vpn.waitForElementProperty('getHelpLink', 'visible', 'true');
    await vpn.wait();
  });

  it('Onboarding', async () => {
    assert(await vpn.getElementProperty('learnMoreLink', 'visible') === 'true');
    await vpn.clickOnElement('learnMoreLink');
    await vpn.wait();

    await vpn.waitForElement('skipOnboarding');
    await vpn.waitForElementProperty('skipOnboarding', 'visible', 'true');

    let onboardingView = 0;
    while (true) {
      await screenCapture(`onboarding_${++onboardingView}`);

      assert(await vpn.hasElement('onboardingNext'));
      assert(
          await vpn.getElementProperty('onboardingNext', 'visible') === 'true');

      await vpn.setSetting('language-code', 'en');

      if (await vpn.getElementProperty('onboardingNext', 'text') !== 'Next') {
        break;
      }

      await vpn.clickOnElement('onboardingNext');
      await vpn.wait();
    }
  });

  it('Authenticating', async () => {
    await vpn.clickOnElement('onboardingNext');
    await vpn.wait();

    await vpn.waitForElement('telemetryPolicyButton');
    await vpn.waitForElementProperty(
        'telemetryPolicyButton', 'visible', 'true');
    await vpn.wait();

    await screenCapture('telemetry_policy');

    await vpn.clickOnElement('telemetryPolicyButton');
    await vpn.wait();

    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.includes('/api/v2/vpn/login');
    });

    await screenCapture('authenticating');
  });

  it('authenticate', async () => await vpn.authenticate());

  it('post authentication view', async () => {
    await vpn.waitForElement('postAuthenticationButton');
    await vpn.wait();

    await screenCapture('post_authenticate');

    await vpn.clickOnElement('postAuthenticationButton');
    await vpn.wait();
  });

  it('main view', async () => {
    await screenCapture('vpn_off');
  });

  it('retrieve list of servers and the current one', async () => {
    servers = await vpn.servers();
  });

  it('server view', async () => {
    await vpn.waitForElement('serverListButton');
    await vpn.waitForElementProperty('serverListButton', 'visible', 'true');
    await vpn.clickOnElement('serverListButton');
    await vpn.wait();

    for (let language of languages) {
      await vpn.setSetting('language-code', language);

      // Let's open all the server items
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
      const height =
          parseInt(await vpn.getElementProperty('serverCountryView', 'height'));

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

    await vpn.waitForElement('serverListBackButton');
    await vpn.waitForElementProperty('serverListBackButton', 'visible', 'true');
    await vpn.clickOnElement('serverListBackButton');
    await vpn.wait();

    await vpn.waitForElement('serverListButton');
    await vpn.waitForElementProperty('serverListButton', 'visible', 'true');
    await vpn.wait();
  });

  it('connecting', async () => {
    await vpn.activate();

    await vpn.waitForCondition(async () => {
      let connectingMsg =
          await vpn.getElementProperty('controllerTitle', 'text');
      return connectingMsg === 'Connecting…';
    });

    await screenCapture('vpn_connecting');
  });

  it('connected', async () => {
    await vpn.waitForCondition(async () => {
      return await vpn.getElementProperty('controllerTitle', 'text') ==
          'VPN is on';
    });

    await screenCapture('vpn_on');
  });

  it('connection info', async () => {
    await vpn.waitForElement('connectionInfoButton');
    await vpn.clickOnElement('connectionInfoButton');
    await vpn.wait();

    await screenCapture('connection_info');

    await vpn.waitForElement('connectionInfoBackButton');
    await vpn.clickOnElement('connectionInfoBackButton');
    await vpn.wait();
  });

  it('settings', async () => {
    await vpn.waitForElement('settingsButton');
    await vpn.clickOnElement('settingsButton');
    await vpn.wait();

    await screenCapture('settings');

    const contentHeight =
        parseInt(await vpn.getElementProperty('settingsView', 'contentHeight'))
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

    await vpn.waitForElement('settingsBackButton');
    await vpn.clickOnElement('settingsBackButton');
    await vpn.wait();

    await vpn.waitForElement('manageAccountButton');
    await vpn.waitForElementProperty('manageAccountButton', 'visible', 'true');
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
    await vpn.waitForElementProperty('advancedDNSSettings', 'visible', 'true');
    await vpn.clickOnElement('advancedDNSSettings');
    await vpn.wait();

    await screenCapture('dns_settings');

    await vpn.clickOnElement('settingsBackButton');
    await vpn.wait();

    await vpn.clickOnElement('settingsBackButton');
    await vpn.wait();

    await vpn.waitForElement('manageAccountButton');
    await vpn.waitForElementProperty('manageAccountButton', 'visible', 'true');
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

    await vpn.clickOnElement('settingsBackButton');
    await vpn.wait();

    await vpn.clickOnElement('settingsBackButton');
    await vpn.wait();

    await vpn.waitForElement('manageAccountButton');
    await vpn.waitForElementProperty('manageAccountButton', 'visible', 'true');
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

    const contentHeight = parseInt(
        await vpn.getElementProperty('settingsLanguagesView', 'contentHeight'))
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

    await vpn.clickOnElement('settingsBackButton');
    await vpn.wait();
    await vpn.clickOnElement('settingsBackButton');
    await vpn.wait();

    await vpn.waitForElement('manageAccountButton');
    await vpn.waitForElementProperty('manageAccountButton', 'visible', 'true');
    await vpn.waitForElementProperty(
        'manageAccountButton', 'text', 'Manage account');
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

    await vpn.clickOnElement('settingsBackButton');
    await vpn.wait();

    await vpn.waitForElement('manageAccountButton');
    await vpn.waitForElementProperty('manageAccountButton', 'visible', 'true');
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

    await vpn.clickOnElement('getHelpBack');
    await vpn.wait();

    await vpn.waitForElement('settingsGetHelp');
    await vpn.waitForElementProperty('settingsGetHelp', 'visible', 'true');
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
    await vpn.waitForElementProperty('settingsGiveFeedback', 'visible', 'true');
    await vpn.clickOnElement('settingsGiveFeedback');
    await vpn.wait();

    await screenCapture('settings_give_feedback');

    await vpn.waitForElement('giveFeedbackBackButton');
    await vpn.waitForElementProperty('giveFeedbackBackButton', 'visible', 'true');
    await vpn.clickOnElement('giveFeedbackBackButton');
    await vpn.wait();

    await vpn.waitForElement('settingsBackButton');
    await vpn.waitForElementProperty('settingsBackButton', 'visible', 'true');
    await vpn.clickOnElement('settingsBackButton');
    await vpn.wait();

    await vpn.waitForElement('settingsGetHelp');
    await vpn.waitForElementProperty('settingsGetHelp', 'visible', 'true');
  });

  it('closing the settings view', async () => {
    await vpn.setElementProperty('settingsView', 'contentY', 'i', 0);
    await vpn.wait();

    await vpn.clickOnElement('settingsCloseButton');
    await vpn.wait();

    await vpn.waitForElement('controllerTitle');
    await vpn.waitForElementProperty('controllerTitle', 'visible', 'true');
  });

  it('disconnecting', async () => {
    await vpn.deactivate();

    await vpn.waitForCondition(async () => {
      return await vpn.getElementProperty('controllerTitle', 'text') ===
          'Disconnecting…';
    });

    await screenCapture('vpn_disconnecting');

    await vpn.waitForCondition(async () => {
      return await vpn.getElementProperty('controllerTitle', 'text') ===
          'VPN is off';
    });
  });

  it('quit the app', async () => await vpn.quit());
});
