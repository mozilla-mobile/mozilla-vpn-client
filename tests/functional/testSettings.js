/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const util = require('util');
const vpn = require('./helper.js');

const exec = util.promisify(require('child_process').exec);

describe('Settings', function() {
  this.timeout(600000);

  before(async () => {
    await vpn.connect();
  });

  beforeEach(() => {});

  afterEach(vpn.dumpFailure);

  after(async () => {
    vpn.disconnect();
  });

  async function checkSetting(objectName, settingKey) {
    await vpn.waitForElement(objectName);
    await vpn.waitForElementProperty(objectName, 'visible', 'true');
    assert(
        await vpn.getElementProperty(objectName, 'isChecked') ===
        await vpn.getSetting(settingKey));

    await vpn.setSetting(settingKey, true);
    assert((await vpn.getSetting(settingKey)) === 'true');
    assert((await vpn.getElementProperty(objectName, 'isChecked')) === 'true');
    await vpn.wait();

    await vpn.setSetting(settingKey, false);
    assert((await vpn.getSetting(settingKey)) === 'false');
    assert((await vpn.getElementProperty(objectName, 'isChecked')) === 'false');
    await vpn.wait();
  }

  it('authenticate', async () => await vpn.authenticate());

  it('Post authentication view', async () => {
    await vpn.waitForElement('postAuthenticationButton');
    await vpn.clickOnElement('postAuthenticationButton');
    await vpn.wait();
  });

  it('Opening and closing the settings view', async () => {
    await vpn.waitForElement('settingsButton');
    await vpn.clickOnElement('settingsButton');
    await vpn.wait();

    await vpn.waitForElement('settingsCloseButton');
    await vpn.waitForElementProperty('settingsCloseButton', 'visible', 'true');

    await vpn.clickOnElement('settingsCloseButton');
    await vpn.wait();

    await vpn.waitForElement('controllerTitle');
    await vpn.waitForElementProperty('controllerTitle', 'visible', 'true');
  });

  it('Checking settings entries', async () => {
    await vpn.waitForElement('settingsButton');
    await vpn.clickOnElement('settingsButton');
    await vpn.wait();

    await vpn.waitForElement('manageAccountButton');
    await vpn.waitForElementProperty('manageAccountButton', 'visible', 'true');
    await vpn.clickOnElement('manageAccountButton');
    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.includes('/r/vpn/account');
    });
  });

  it('Checking the networking settings', async () => {
    await vpn.waitForElement('settingsNetworking');
    await vpn.waitForElementProperty('settingsNetworking', 'visible', 'true');

    await vpn.setElementProperty(
        'settingsView', 'contentY', 'i',
        parseInt(await vpn.getElementProperty('settingsNetworking', 'y')));
    await vpn.wait();

    await vpn.clickOnElement('settingsNetworking');
    await vpn.wait();

    await checkSetting('settingIpv6Enabled', 'ipv6-enabled');
    await checkSetting('settingLocalNetworkAccess', 'local-network-access');

    await vpn.waitForElement('settingsBackButton');
    await vpn.waitForElementProperty('settingsBackButton', 'visible', 'true');
    await vpn.clickOnElement('settingsBackButton');
    await vpn.wait();

    await vpn.waitForElement('manageAccountButton');
    await vpn.waitForElementProperty('manageAccountButton', 'visible', 'true');
  });

  it('Checking the languages settings', async () => {
    await vpn.setSetting('language-code', '');

    await vpn.waitForElement('settingsPreferences');
    await vpn.waitForElementProperty('settingsPreferences', 'visible', 'true');
    await vpn.clickOnElement('settingsPreferences');
    await vpn.wait();

    await vpn.waitForElement('settingsLanguages');
    await vpn.waitForElementProperty('settingsLanguages', 'visible', 'true');

    await vpn.clickOnElement('settingsLanguages');
    await vpn.wait();

    await vpn.waitForElement('settingsSystemLanguageToggle');
    await vpn.waitForElementProperty(
        'settingsSystemLanguageToggle', 'visible', 'true');
    await vpn.waitForElementProperty(
        'settingsSystemLanguageToggle', 'checked', 'true');

    await vpn.clickOnElement('settingsSystemLanguageToggle');
    await vpn.waitForElementProperty(
        'settingsSystemLanguageToggle', 'checked', 'false');

    await vpn.setElementProperty(
        'settingsLanguagesView', 'contentY', 'i',
        parseInt(
            await vpn.getElementProperty('languageList/language-it', 'y')));
    await vpn.wait();

    await vpn.waitForElement('languageList/language-it');
    await vpn.waitForElementProperty(
        'languageList/language-it', 'visible', 'true');
    await vpn.clickOnElement('languageList/language-it');
    await vpn.wait();

    await vpn.clickOnElement('settingsBackButton');
    await vpn.wait();

    await vpn.clickOnElement('settingsBackButton');
    await vpn.wait();

    await vpn.waitForElement('manageAccountButton');
    await vpn.waitForElementProperty('manageAccountButton', 'visible', 'true');


    await vpn.waitForElement('settingsPreferences');
    await vpn.waitForElementProperty('settingsPreferences', 'visible', 'true');
    await vpn.clickOnElement('settingsPreferences');
    await vpn.wait();

    await vpn.waitForElement('settingsLanguages');
    await vpn.waitForElementProperty('settingsLanguages', 'visible', 'true');
    await vpn.clickOnElement('settingsLanguages');
    await vpn.wait();

    await vpn.waitForElement('settingsBackButton');
    await vpn.waitForElementProperty(
        'settingsBackButton', 'visible', 'true');
    await vpn.waitForElementProperty(
        'settingsSystemLanguageToggle', 'checked', 'false');

    await vpn.setElementProperty(
        'settingsLanguagesView', 'contentY', 'i',
        parseInt(
            await vpn.getElementProperty('languageList/language-en', 'y')));
    await vpn.wait();

    await vpn.waitForElement('languageList/language-en');
    await vpn.waitForElementProperty(
        'languageList/language-en', 'visible', 'true');
    await vpn.clickOnElement('languageList/language-en');
    await vpn.wait();

    await vpn.clickOnElement('settingsBackButton');
    await vpn.wait();
    await vpn.clickOnElement('settingsBackButton');
    await vpn.wait();

    await vpn.waitForElement('manageAccountButton');
    await vpn.waitForElementProperty('manageAccountButton', 'visible', 'true');
    await vpn.waitForElementProperty(
        'manageAccountButton', 'text', 'Manage account');

    await vpn.waitForElement('settingsPreferences');
    await vpn.waitForElementProperty('settingsPreferences', 'visible', 'true');
    await vpn.clickOnElement('settingsPreferences');
    await vpn.wait();

    await vpn.waitForElement('settingsLanguages');
    await vpn.waitForElementProperty('settingsLanguages', 'visible', 'true');
    await vpn.clickOnElement('settingsLanguages');
    await vpn.wait();

    await vpn.clickOnElement('settingsLanguages');
    await vpn.wait();
    await vpn.waitForElement('settingsBackButton');

    await vpn.setElementProperty('settingsLanguagesView', 'contentY', 'i', 0);
    await vpn.wait();

    await vpn.waitForElementProperty(
        'settingsBackButton', 'visible', 'true');
    await vpn.waitForElementProperty(
        'settingsSystemLanguageToggle', 'checked', 'false');

    await vpn.clickOnElement('settingsSystemLanguageToggle');
    await vpn.waitForElementProperty(
        'settingsSystemLanguageToggle', 'checked', 'true');

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

  it('Checking the about us', async () => {
    await vpn.waitForElement('settingsAboutUs');
    await vpn.waitForElementProperty('settingsAboutUs', 'visible', 'true');

    await vpn.setElementProperty(
        'settingsView', 'contentY', 'i',
        parseInt(await vpn.getElementProperty('settingsAboutUs', 'y')));
    await vpn.wait();

    await vpn.clickOnElement('settingsAboutUs');
    await vpn.wait();

    await vpn.waitForElement('settingsBackButton');
    await vpn.waitForElementProperty('settingsBackButton', 'visible', 'true');

    await vpn.waitForElement('aboutUsList');

    await vpn.waitForElement('aboutUsList/aboutUsList-tos');
    await vpn.waitForElementProperty(
        'aboutUsList/aboutUsList-tos', 'visible', 'true');
    await vpn.waitForElement('aboutUsList/aboutUsList-privacy');
    await vpn.waitForElementProperty(
        'aboutUsList/aboutUsList-privacy', 'visible', 'true');
    await vpn.waitForElement('aboutUsList/aboutUsList-license');
    await vpn.waitForElementProperty(
        'aboutUsList/aboutUsList-license', 'visible', 'true');

    await vpn.clickOnElement('aboutUsList/aboutUsList-tos');
    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.endsWith('/r/vpn/terms');
    });

    await vpn.clickOnElement('aboutUsList/aboutUsList-privacy');
    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.endsWith('/r/vpn/privacy');
    });

    await vpn.clickOnElement('aboutUsList/aboutUsList-license');
    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url ===
          'https://github.com/mozilla-mobile/mozilla-vpn-client/blob/main/LICENSE.md';
    });

    await vpn.waitForElement('settingsBackButton');
    await vpn.clickOnElement('settingsBackButton');
    await vpn.wait();

    await vpn.waitForElement('manageAccountButton');
    await vpn.waitForElementProperty('manageAccountButton', 'visible', 'true');
  });


  it('Checking the get help', async () => {
    await vpn.waitForElement('settingsGetHelp');
    await vpn.waitForElementProperty('settingsGetHelp', 'visible', 'true');

    await vpn.setElementProperty(
        'settingsView', 'contentY', 'i',
        parseInt(await vpn.getElementProperty('settingsGetHelp', 'y')));
    await vpn.wait();

    await vpn.clickOnElement('settingsGetHelp');
    await vpn.wait();

    await vpn.waitForElement('settingsBackButton');
    await vpn.waitForElementProperty('settingsBackButton', 'visible', 'true');

    await vpn.waitForElement('getHelpLinks');
    await vpn.waitForElementProperty('getHelpLinks', 'visible', 'true');

    await vpn.waitForElement('getHelpLinks/getHelpBackList');
    await vpn.waitForElementProperty(
        'getHelpLinks/getHelpBackList', 'visible', 'true');

    await vpn.waitForElement('getHelpLinks/settingsGiveFeedback');
    await vpn.waitForElementProperty(
      'getHelpLinks/settingsGiveFeedback', 'visible', 'true');

    await vpn.clickOnElement('getHelpLinks/settingsGiveFeedback');
    await vpn.wait();

    await vpn.waitForElement('giveFeedbackView');
    await vpn.waitForElementProperty(
      'giveFeedbackView', 'visible', 'true');
    await vpn.clickOnElement('giveFeedbackView');
    await vpn.wait();

    await vpn.waitForElement('settingsBackButton');
    await vpn.waitForElementProperty('settingsBackButton', 'visible', 'true');
    await vpn.clickOnElement('settingsBackButton');


    await vpn.waitForElement('getHelpBackList');
    await vpn.waitForElementProperty('getHelpBackList', 'visible', 'true');

    // TODO: checking the give feedback views

  /* TODO: Reinstate these tests correctly
      await vpn.waitForElement('getHelpBackList/getHelpBackList-2');
      await vpn.waitForElementProperty(
          'getHelpBackList/getHelpBackList-2', 'visible', 'true');

      await vpn.clickOnElement('getHelpBackList/getHelpBackList-2');

      await vpn.waitForCondition(async () => {
        const url = await vpn.getLastUrl();
        return url.startsWith('file://') && url.includes('mozillavpn') &&
            url.endsWith('.txt');
      });

      await vpn.waitForElement('getHelpBackList/getHelpBackList-0')
      await vpn.waitForElementProperty(
        'getHelpBackList/getHelpBackList-0', 'visible', 'true');

      await vpn.clickOnElement('getHelpBackList/getHelpBackList-0');
      await vpn.waitForCondition(async () => {
        const url = await vpn.getLastUrl();
        return url.endsWith('/r/vpn/support');
      });
    /*

    /* TODO:  Reinstate this test correctly
        https://github.com/mozilla-mobile/mozilla-vpn-client/issues/1638
        Disabling this test will let us continue running all testSettings tests
        to make sure nothing else is broken.

        await vpn.clickOnElement('getHelpBackList/getHelpBackList-2');
        await vpn.waitForCondition(async () => {
            const url = await vpn.getLastUrl();
            return url.endsWith('/r/vpn/contact');
        });
    */

    await vpn.wait();
    await vpn.waitForElement('settingsBackButton');
    await vpn.clickOnElement('settingsBackButton');

    await vpn.waitForElement('settingsGetHelp');
    await vpn.waitForElementProperty('settingsGetHelp', 'visible', 'true');
  });

  it('Checking the preferences settings', async () => {
    await vpn.waitForElement('settingsPreferences');
    await vpn.waitForElementProperty(
        'settingsPreferences', 'visible', 'true');

    await vpn.setElementProperty(
        'settingsView', 'contentY', 'i',
        parseInt(await vpn.getElementProperty('settingsPreferences', 'y')));
    await vpn.wait();

    await vpn.clickOnElement('settingsPreferences');
    await vpn.wait();

    await checkSetting('dataCollection', 'glean-enabled');

    await vpn.clickOnElement('settingsBackButton');
    await vpn.wait();

    await vpn.waitForElement('manageAccountButton');
    await vpn.waitForElementProperty('manageAccountButton', 'visible', 'true');
  });

  it('Checking the notifications settings', async () => {
    await vpn.waitForElement('settingsPreferences');
    await vpn.waitForElementProperty(
        'settingsPreferences', 'visible', 'true');

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

    /* TODO: captive-portal disabled
        await checkSetting('settingCaptivePortalAlert', 'captive-portal-alert');
        await checkSetting(
            'settingUnsecuredNetworkAlert', 'unsecured-network-alert');
    */
    await checkSetting('switchServersAlert', 'server-switch-notification');
    await checkSetting('connectionChangeAlert','connection-change-notification');

    await vpn.clickOnElement('settingsBackButton');
    await vpn.wait();

    await vpn.clickOnElement('settingsBackButton');

    await vpn.waitForElement('manageAccountButton');
    await vpn.waitForElementProperty('manageAccountButton', 'visible', 'true');
  });

  it('Checking the logout', async () => {
    await vpn.waitForElement('settingsLogout');
    await vpn.waitForElementProperty('settingsLogout', 'visible', 'true');
  });

  it('Logout', async () => {
    await vpn.logout();
    await vpn.wait();
  });

  it('quit the app', async () => await vpn.quit());
});
