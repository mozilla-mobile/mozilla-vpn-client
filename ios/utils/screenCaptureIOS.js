/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const fs = require('fs');
const util = require('util');
const vpn = require('./helper.js');
const FirefoxHelper = require('./firefox.js');

const dir = process.env.ARTIFACT_DIR + '/screencapture';

describe('Take screenshots for each view', function() {
  let languages = [];
  let driver;

  this.timeout(100000);

  async function screenCapture(name) {
    for (let language of languages) {
      await vpn.setSetting('language-code', language);

      // we need to give time to the app to retranslate the UI. If the number
      // is too slow we have the UI in funny states (part in 1 language, part
      // in another language, ...). But if the number is too high, the
      // "connecting" state is faster and we do not take all the screen
      // captures for all the languages.
      await new Promise(r => setTimeout(r, 30));

      const data = await vpn.screenCapture();
      const buffer = Buffer.from(data, 'base64');
      fs.writeFileSync(`${dir}/${name}_${language}.png`, buffer);
    }
  }

  before(async () => {
    // TODO: change this IP with a variable
    await vpn.connect('192.168.1.5');

    driver = await FirefoxHelper.createDriver();

    if (!fs.existsSync(dir)) {
      fs.mkdirSync(dir);
    }
  });

  beforeEach(() => {});

  afterEach(() => {});

  after(async () => {
    await driver.quit();
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
    await vpn.forceHeartbeatFailure();

    await vpn.waitForElement('heartbeatTryButton');
    await vpn.waitForElementProperty('heartbeatTryButton', 'visible', 'true');

    await screenCapture('heartbeat');

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
    await screenCapture('authenticating');
  });

  it('Wait for user-interaction', async () => {
    while (1) {
      console.log(1);
      await vpn.wait();

      if (!(await vpn.hasElement('controllerTitle'))) continue;

      if (await vpn.getElementProperty('controllerTitle', 'visible') !==
          'true') {
        continue;
      }

      break;
    }
  });

  it('main view', async () => {
    await screenCapture('vpn_off');
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
    await vpn.waitForElement('connectionInfoToggleButton');
    await vpn.clickOnElement('connectionInfoToggleButton');
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
    await vpn.clickOnElement('settingsNetworking');
    await vpn.wait();

    await screenCapture('settings_networking');

    await vpn.clickOnElement('settingsNetworkingBackButton');
    await vpn.wait();

    await vpn.waitForElement('manageAccountButton');
    await vpn.waitForElementProperty('manageAccountButton', 'visible', 'true');
  });

  it('settings / languages', async () => {
    await vpn.waitForElement('settingsLanguages');
    await vpn.waitForElementProperty('settingsLanguages', 'visible', 'true');
    await vpn.clickOnElement('settingsLanguages');
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

    await vpn.clickOnElement('settingsLanguagesBackButton');
    await vpn.wait();

    await vpn.waitForElement('manageAccountButton');
    await vpn.waitForElementProperty('manageAccountButton', 'visible', 'true');
    await vpn.waitForElementProperty(
        'manageAccountButton', 'text', 'Manage account');
  });

  // TODO: app-permission

  it('setting / about us', async () => {
    await vpn.waitForElement('settingsAboutUs');
    await vpn.waitForElementProperty('settingsAboutUs', 'visible', 'true');
    await vpn.clickOnElement('settingsAboutUs');
    await vpn.wait();

    await screenCapture('settings_about');

    await vpn.clickOnElement('aboutUsBackButton');
    await vpn.wait();

    await vpn.waitForElement('manageAccountButton');
    await vpn.waitForElementProperty('manageAccountButton', 'visible', 'true');
  });

  it('settings / help', async () => {
    await vpn.waitForElement('settingsGetHelp');
    await vpn.waitForElementProperty('settingsGetHelp', 'visible', 'true');
    await vpn.clickOnElement('settingsGetHelp');
    await vpn.wait();

    await screenCapture('settings_help');

    await vpn.clickOnElement('getHelpBack');
    await vpn.wait();

    await vpn.waitForElement('settingsGetHelp');
    await vpn.waitForElementProperty('settingsGetHelp', 'visible', 'true');
  });

  it('closing the settings view', async () => {
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
