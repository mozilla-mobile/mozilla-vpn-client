/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const vpn = require('./helper.js');

describe('Settings', function () {
  this.timeout(60000);
  this.ctx.authenticationNeeded = true;

  beforeEach(async () => {
    await vpn.waitForElement('navigationLayout/navButton-settings');
    await vpn.clickOnElement('navigationLayout/navButton-settings');
    await vpn.wait();

    if (!(await vpn.isFeatureFlippedOff('subscriptionManagement'))) {
      await vpn.flipFeatureOff('subscriptionManagement');
    }
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

  async function getToGetHelpView() {
    await vpn.waitForElement('settingsGetHelp');
    await vpn.waitForElementProperty('settingsGetHelp', 'visible', 'true');
    await vpn.clickOnElement('settingsGetHelp');

    await vpn.wait;
    await vpn.waitForElement('getHelpBack');
    await vpn.waitForElementProperty('getHelpBack', 'visible', 'true');
  }

  it('Opening and closing the settings view', async () => {
    await vpn.waitForElement('navigationLayout/navButton-home');
    await vpn.waitForElementProperty(
      'navigationLayout/navButton-home', 'visible', 'true');


    await vpn.waitForElement('menuIcon');
    await vpn.waitForElementProperty(
      'menuIcon', 'source', 'qrc:/nebula/resources/close-dark.svg');

    await vpn.waitForElement('settingsUserProfile-manageAccountButton');
    await vpn.waitForElementProperty(
      'settingsUserProfile-manageAccountButton', 'visible', 'true');

    await vpn.waitForElementProperty(
      'settingsUserProfile-manageAccountButton', 'enabled', 'true');

    await vpn.clickOnElement('navigationLayout/navButton-home');
    await vpn.wait();

    await vpn.waitForElement('controllerTitle');
    await vpn.waitForElementProperty('controllerTitle', 'visible', 'true');
  });

  it('Checking settings entries', async () => {
    await vpn.waitForElement('settingsUserProfile-manageAccountButton');
    await vpn.waitForElementProperty(
      'settingsUserProfile-manageAccountButton', 'visible', 'true');

    if ((await vpn.isFeatureFlippedOff('subscriptionManagement'))) {
      await vpn.clickOnElement('settingsUserProfile-manageAccountButton');
      await vpn.waitForCondition(async () => {
        const url = await vpn.getLastUrl();
        return url.includes('https://accounts.stage.mozaws.net') &&
          url.includes('?email=test@mozilla.com');
      });
    }
  });

  it('Checking the tips and tricks settings', async () => {
    await vpn.waitForElement('settingsTipsAndTricks');
    await vpn.waitForElementProperty('settingsTipsAndTricks', 'visible', 'true');

    await vpn.setElementProperty(
      'settingsView-flickable', 'contentY', 'i',
      parseInt(await vpn.getElementProperty('settingsTipsAndTricks', 'y')));
    await vpn.wait();

    await vpn.clickOnElement('settingsTipsAndTricks');
    await vpn.wait();

    await vpn.waitForElement('settingsTipsAndTricksPage');


    // TODO: (VPN-2749)
    // //Test guides
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

    await vpn.waitForElement('tipsAndTricks-back');
    await vpn.waitForElementProperty('tipsAndTricks-back', 'visible', 'true');
    await vpn.clickOnElement('tipsAndTricks-back');
    await vpn.wait();

    await vpn.waitForElement('settingsUserProfile-manageAccountButton');
    await vpn.waitForElementProperty(
      'settingsUserProfile-manageAccountButton', 'visible', 'true');
  });

  it('Checking the networking settings', async () => {
    await vpn.waitForElement('settingsNetworking');
    await vpn.waitForElementProperty('settingsNetworking', 'visible', 'true');

    await vpn.setElementProperty(
      'settingsView-flickable', 'contentY', 'i',
      parseInt(await vpn.getElementProperty('settingsNetworking', 'y')));
    await vpn.wait();

    await vpn.clickOnElement('settingsNetworking');
    await vpn.wait();

    await checkSetting('settingLocalNetworkAccess', 'local-network-access');

    await vpn.waitForElement('settings-back');
    await vpn.waitForElementProperty('settings-back', 'visible', 'true');
    await vpn.clickOnElement('settings-back');
    await vpn.wait();

    await vpn.waitForElement('settingsUserProfile-manageAccountButton');
    await vpn.waitForElementProperty(
      'settingsUserProfile-manageAccountButton', 'visible', 'true');
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

    await vpn.getElementProperty(
      'languageList/language-column-it/language-it', 'y');
    await vpn.setElementProperty(
      'settingsLanguagesView-flickable', 'contentY', 'i',
      parseInt(await vpn.getElementProperty(
        'languageList/language-column-it/language-it', 'y')));
    await vpn.wait();

    await vpn.waitForElement('languageList/language-column-it/language-it');
    await vpn.waitForElementProperty(
      'languageList/language-column-it/language-it', 'visible', 'true');
    await vpn.clickOnElement('languageList/language-column-it/language-it');
    await vpn.wait();

    await vpn.waitForElement('settings-back');
    await vpn.waitForElementProperty('settings-back', 'visible', 'true');
    await vpn.clickOnElement('settings-back');
    await vpn.wait();

    await vpn.clickOnElement('settings-back');
    await vpn.wait();

    await vpn.waitForElement('settingsUserProfile-manageAccountButton');
    await vpn.waitForElementProperty(
      'settingsUserProfile-manageAccountButton', 'visible', 'true');

    await vpn.waitForElement('settingsPreferences');
    await vpn.waitForElementProperty('settingsPreferences', 'visible', 'true');
    await vpn.clickOnElement('settingsPreferences');
    await vpn.wait();

    await vpn.waitForElement('settingsLanguages');
    await vpn.waitForElementProperty('settingsLanguages', 'visible', 'true');
    await vpn.clickOnElement('settingsLanguages');
    await vpn.wait();

    await vpn.waitForElement('settings-back');
    await vpn.waitForElementProperty('settings-back', 'visible', 'true');
    await vpn.waitForElementProperty(
      'settingsSystemLanguageToggle', 'checked', 'false');

    await vpn.setElementProperty(
      'settingsLanguagesView-flickable', 'contentY', 'i',
      parseInt(await vpn.getElementProperty(
        'languageList/language-column-en/language-en', 'y')));
    await vpn.wait();

    await vpn.waitForElement('languageList/language-column-en/language-en');
    await vpn.waitForElementProperty(
      'languageList/language-column-en/language-en', 'visible', 'true');
    await vpn.clickOnElement('languageList/language-column-en/language-en');
    await vpn.wait();

    await vpn.clickOnElement('settings-back');
    await vpn.wait();
    await vpn.clickOnElement('settings-back');
    await vpn.wait();

    await vpn.waitForElement('settingsUserProfile-manageAccountButton');
    await vpn.waitForElementProperty(
      'settingsUserProfile-manageAccountButton', 'visible', 'true');

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
    await vpn.waitForElement('settings-back');

    await vpn.setElementProperty(
      'settingsLanguagesView-flickable', 'contentY', 'i', 0);
    await vpn.wait();

    await vpn.waitForElementProperty('settings-back', 'visible', 'true');
    await vpn.waitForElementProperty(
      'settingsSystemLanguageToggle', 'checked', 'false');

    await vpn.clickOnElement('settingsSystemLanguageToggle');
    await vpn.waitForElementProperty(
      'settingsSystemLanguageToggle', 'checked', 'true');

    await vpn.clickOnElement('settings-back');
    await vpn.wait();
    await vpn.clickOnElement('settings-back');
    await vpn.wait();

    await vpn.waitForElement('settingsUserProfile-manageAccountButton');
    await vpn.waitForElementProperty(
      'settingsUserProfile-manageAccountButton', 'visible', 'true');
  });

  // TODO: app-permission

  it('Checking the about us', async () => {
    await vpn.waitForElement('settingsAboutUs');
    await vpn.waitForElementProperty('settingsAboutUs', 'visible', 'true');

    await vpn.setElementProperty(
      'settingsView-flickable', 'contentY', 'i',
      parseInt(await vpn.getElementProperty('settingsAboutUs', 'y')));
    await vpn.wait();

    await vpn.clickOnElement('settingsAboutUs');
    await vpn.wait();

    await vpn.waitForElement('settings-back');
    await vpn.waitForElementProperty('settings-back', 'visible', 'true');

    await vpn.waitForElement('aboutUsList');

    await vpn.waitForElement('viewAboutUs');
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

    await vpn.waitForElement('settings-back');
    await vpn.waitForElementProperty('settings-back', 'visible', 'true');
    await vpn.clickOnElement('settings-back');
    await vpn.wait();

    await vpn.waitForElement('settings-back');
    await vpn.clickOnElement('settings-back');
    await vpn.wait();

    await vpn.waitForElement('settingsUserProfile-manageAccountButton');
    await vpn.waitForElementProperty(
      'settingsUserProfile-manageAccountButton', 'visible', 'true');
  });

  it('Checking the get help', async () => {
    await vpn.waitForElement('settingsGetHelp');
    await vpn.waitForElementProperty('settingsGetHelp', 'visible', 'true');

    await vpn.setElementProperty(
      'settingsView-flickable', 'contentY', 'i',
      parseInt(await vpn.getElementProperty('settingsGetHelp', 'y')));
    await vpn.wait();

    await vpn.clickOnElement('settingsGetHelp');
    await vpn.wait();

    await vpn.waitForElement('getHelpBack');
    await vpn.waitForElementProperty('getHelpBack', 'visible', 'true');

    await vpn.waitForElement('getHelpLinks');
    await vpn.waitForElementProperty('getHelpLinks', 'visible', 'true');

    await vpn.waitForElement('settingsGiveFeedback');
    await vpn.waitForElementProperty('settingsGiveFeedback', 'visible', 'true');

    await vpn.clickOnElement('settingsGiveFeedback');
    await vpn.wait();

    await vpn.waitForElement('giveFeedbackView');
    await vpn.waitForElementProperty(
      'giveFeedbackView', 'visible', 'true');
    await vpn.clickOnElement('giveFeedbackView');
    await vpn.wait();

    await vpn.waitForElement('getHelpBack');
    await vpn.clickOnElement('getHelpBack');
    await vpn.wait();

    await vpn.waitForElement('getHelpLinks');
    await vpn.waitForElementProperty('getHelpLinks', 'visible', 'true');

    // TODO: checking the give feedback views

    await vpn.waitForElement('viewLogs');
    await vpn.waitForElementProperty('viewLogs', 'visible', 'true');
    await vpn.clickOnElement('viewLogs');

    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.startsWith('file://') && url.includes('mozillavpn') &&
        url.endsWith('.txt');
    });

    await vpn.waitForElement('helpCenter');
    await vpn.waitForElementProperty('helpCenter', 'visible', 'true');

    await vpn.clickOnElement('helpCenter');
    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.endsWith('/r/vpn/support');
    });

    await vpn.wait();
    await vpn.waitForElement('getHelpBack');
    await vpn.clickOnElement('getHelpBack');

    await vpn.waitForElement('settingsGetHelp');
    await vpn.waitForElementProperty('settingsGetHelp', 'visible', 'true');
  });

  it('Get help is opened and closed', async () => {
    await getToGetHelpView();

    await vpn.wait();
    await vpn.waitForElement('getHelpBack');
    await vpn.clickOnElement('getHelpBack');

    await vpn.wait();
    await vpn.waitForElement('settingsGetHelp');
    await vpn.waitForElementProperty('settingsGetHelp', 'visible', 'true');
  });

  it('Give feedback is opened and closed', async () => {
    await getToGetHelpView();
    await vpn.wait();

    await vpn.waitForElement('settingsGiveFeedback');
    await vpn.waitForElementProperty('settingsGiveFeedback', 'visible', 'true');

    await vpn.clickOnElement('settingsGiveFeedback');
    await vpn.wait();

    await vpn.wait();
    await vpn.waitForElement('getHelpBack');
    await vpn.clickOnElement('getHelpBack');
    await vpn.wait();
  });

  it('Contact us is opened and closed', async () => {
    await getToGetHelpView();
    await vpn.wait();

    await vpn.waitForElement('inAppSupport');
    await vpn.waitForElementProperty('inAppSupport', 'visible', 'true');

    await vpn.clickOnElement('inAppSupport');

    await vpn.wait();
    await vpn.waitForElement('contactUs-userInfo');
    await vpn.clickOnElement('getHelpBack');
    await vpn.wait();
  });

  it('Checking the preferences settings', async () => {
    await vpn.waitForElement('settingsPreferences');
    await vpn.waitForElementProperty(
      'settingsPreferences', 'visible', 'true');

    await vpn.setElementProperty(
      'settingsView-flickable', 'contentY', 'i',
      parseInt(await vpn.getElementProperty('settingsPreferences', 'y')));
    await vpn.wait();

    await vpn.clickOnElement('settingsPreferences');
    await vpn.wait();

    await checkSetting('dataCollection', 'glean-enabled');

    await vpn.clickOnElement('settings-back');
    await vpn.wait();

    await vpn.waitForElement('settingsUserProfile-manageAccountButton');
    await vpn.waitForElementProperty(
      'settingsUserProfile-manageAccountButton', 'visible', 'true');
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
      'settingsView-flickable', 'contentY', 'i',
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
    await checkSetting('connectionChangeAlert', 'connection-change-notification');

    await vpn.clickOnElement('settings-back');
    await vpn.wait();

    await vpn.clickOnElement('settings-back');

    await vpn.waitForElement('settingsUserProfile-manageAccountButton');
    await vpn.waitForElementProperty(
      'settingsUserProfile-manageAccountButton', 'visible', 'true');
  });

  it('Checking the logout', async () => {
    await vpn.waitForElement('settingsLogout');
    await vpn.waitForElementProperty('settingsLogout', 'visible', 'true');
    await vpn.scrollToElement('settingsView-flickable', 'settingsLogout');

    await vpn.clickOnElement('settingsLogout');
    await vpn.waitForMainView();
  });
});
