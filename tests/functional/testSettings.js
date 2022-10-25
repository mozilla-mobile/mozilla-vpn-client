/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const { navBar, settingsView, generalElements } = require('./elements.js');
const vpn = require('./helper.js');

describe('Settings', function () {
  this.timeout(60000);
  this.ctx.authenticationNeeded = true;

  beforeEach(async () => {
    await vpn.waitForElement(navBar.SETTINGS);
    await vpn.clickOnElement(navBar.SETTINGS);
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
    await vpn.waitForElement(settingsView.GET_HELP);
    await vpn.waitForElementProperty(settingsView.GET_HELP, 'visible', 'true');
    await vpn.clickOnElement(settingsView.GET_HELP);

    await vpn.wait();
    await vpn.waitForElement(settingsView.getHelpView.BACK);
    await vpn.waitForElementProperty(settingsView.getHelpView.BACK, 'visible', 'true');
  }

  it('Opening and closing the settings view', async () => {
    await vpn.waitForElement(navBar.HOME);
    await vpn.waitForElementProperty(navBar.HOME, 'visible', 'true');


    await vpn.waitForElement('menuIcon');
    await vpn.waitForElementProperty('menuIcon', 'source', 'qrc:/nebula/resources/close-dark.svg');

    await vpn.waitForElement(settingsView.USER_PROFILE);
    await vpn.waitForElementProperty(settingsView.USER_PROFILE, 'visible', 'true');
    await vpn.waitForElementProperty(settingsView.USER_PROFILE, 'enabled', 'true');

    await vpn.clickOnElement(navBar.HOME);
    await vpn.wait();

    await vpn.waitForElement(generalElements.CONTROLLER_TITLE);
    await vpn.waitForElementProperty(generalElements.CONTROLLER_TITLE, 'visible', 'true');
  });

  it('Checking settings entries', async () => {
    await vpn.waitForElement(settingsView.USER_PROFILE);
    await vpn.waitForElementProperty(settingsView.USER_PROFILE, 'visible', 'true');

    if ((await vpn.isFeatureFlippedOff('subscriptionManagement'))) {
      await vpn.clickOnElement(settingsView.USER_PROFILE);
      await vpn.waitForCondition(async () => {
        const url = await vpn.getLastUrl();
        return url.includes('https://accounts.stage.mozaws.net') &&
          url.includes('?email=test@mozilla.com');
      });
    }
  });

  it('Checking the tips and tricks settings', async () => {
    await vpn.waitForElement(settingsView.TIPS_AND_TRICKS);
    await vpn.waitForElementProperty(settingsView.TIPS_AND_TRICKS, 'visible', 'true');
    await vpn.setElementProperty(settingsView.SCREEN, 'contentY', 'i', parseInt(await vpn.getElementProperty(settingsView.TIPS_AND_TRICKS, 'y')));
    await vpn.wait();

    await vpn.clickOnElement(settingsView.TIPS_AND_TRICKS);
    await vpn.wait();
    await vpn.waitForElement(settingsView.tipsAndTricksView.SCREEN);


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

    await vpn.waitForElement(settingsView.tipsAndTricksView.BACK);
    await vpn.waitForElementProperty(settingsView.tipsAndTricksView.BACK, 'visible', 'true');
    await vpn.clickOnElement(settingsView.tipsAndTricksView.BACK);
    await vpn.wait();

    await vpn.waitForElement(settingsView.USER_PROFILE);
    await vpn.waitForElementProperty(settingsView.USER_PROFILE, 'visible', 'true');
  });

  it('Checking the networking settings', async () => {
    await vpn.waitForElement(settingsView.NETWORK_SETTINGS);
    await vpn.waitForElementProperty(settingsView.NETWORK_SETTINGS, 'visible', 'true');

    await vpn.setElementProperty(settingsView.networkSettingsView.SCREEN, 'contentY', 'i',
      parseInt(await vpn.getElementProperty(settingsView.NETWORK_SETTINGS, 'y')));
    await vpn.wait();

    await vpn.clickOnElement(settingsView.NETWORK_SETTINGS);
    await vpn.wait();

    await checkSetting('settingLocalNetworkAccess', 'local-network-access');

    await vpn.waitForElement(settingsView.BACK);
    await vpn.waitForElementProperty(settingsView.BACK, 'visible', 'true');
    await vpn.clickOnElement(settingsView.BACK);
    await vpn.wait();

    await vpn.waitForElement(settingsView.USER_PROFILE);
    await vpn.waitForElementProperty(settingsView.USER_PROFILE, 'visible', 'true');
  });

  it('Checking the languages settings', async () => {
    await vpn.setSetting('language-code', '');

    await vpn.waitForElement(settingsView.SYSTEM_PREFERENCE);
    await vpn.waitForElementProperty(settingsView.SYSTEM_PREFERENCE, 'visible', 'true');
    await vpn.clickOnElement(settingsView.SYSTEM_PREFERENCE);
    await vpn.wait();

    await vpn.waitForElement(settingsView.systemPreferenceView.LANGUAGE);
    await vpn.waitForElementProperty(settingsView.systemPreferenceView.LANGUAGE, 'visible', 'true');
    await vpn.clickOnElement(settingsView.systemPreferenceView.LANGUAGE);
    await vpn.wait();
  
    await vpn.waitForElement(settingsView.systemPreferenceView.languageSettingsView.systemLanguageToggle);
    await vpn.waitForElementProperty(
      settingsView.systemPreferenceView.languageSettingsView.systemLanguageToggle, 'visible', 'true');
    await vpn.waitForElementProperty(
      settingsView.systemPreferenceView.languageSettingsView.systemLanguageToggle, 'checked', 'true');

    await vpn.clickOnElement(settingsView.systemPreferenceView.languageSettingsView.systemLanguageToggle);
    await vpn.waitForElementProperty(
      settingsView.systemPreferenceView.languageSettingsView.systemLanguageToggle, 'checked', 'false');

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

    await vpn.waitForElement(settingsView.BACK);
    await vpn.waitForElementProperty(settingsView.BACK, 'visible', 'true');
    await vpn.clickOnElement(settingsView.BACK);
    await vpn.wait();

    await vpn.clickOnElement(settingsView.BACK);
    await vpn.wait();

    await vpn.waitForElement(settingsView.USER_PROFILE);
    await vpn.waitForElementProperty(
      settingsView.USER_PROFILE, 'visible', 'true');

    await vpn.waitForElement(settingsView.SYSTEM_PREFERENCE);
    await vpn.waitForElementProperty(settingsView.SYSTEM_PREFERENCE, 'visible', 'true');
    await vpn.clickOnElement(settingsView.SYSTEM_PREFERENCE);
    await vpn.wait();

    await vpn.waitForElement(settingsView.systemPreferenceView.LANGUAGE);
    await vpn.waitForElementProperty(settingsView.systemPreferenceView.LANGUAGE, 'visible', 'true');
    await vpn.clickOnElement(settingsView.systemPreferenceView.LANGUAGE);
    await vpn.wait();

    await vpn.waitForElement(settingsView.BACK);
    await vpn.waitForElementProperty(settingsView.BACK, 'visible', 'true');
    await vpn.waitForElementProperty(
      settingsView.systemPreferenceView.languageSettingsView.systemLanguageToggle, 'checked', 'false');

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

    await vpn.clickOnElement(settingsView.BACK);
    await vpn.wait();
    await vpn.clickOnElement(settingsView.BACK);
    await vpn.wait();

    await vpn.waitForElement(settingsView.USER_PROFILE);
    await vpn.waitForElementProperty(
      settingsView.USER_PROFILE, 'visible', 'true');

    await vpn.waitForElement(settingsView.SYSTEM_PREFERENCE);
    await vpn.waitForElementProperty(settingsView.SYSTEM_PREFERENCE, 'visible', 'true');
    await vpn.clickOnElement(settingsView.SYSTEM_PREFERENCE);
    await vpn.wait();

    await vpn.waitForElement(settingsView.systemPreferenceView.LANGUAGE);
    await vpn.waitForElementProperty(settingsView.systemPreferenceView.LANGUAGE, 'visible', 'true');
    await vpn.clickOnElement(settingsView.systemPreferenceView.LANGUAGE);
    await vpn.wait();

    await vpn.clickOnElement(settingsView.systemPreferenceView.LANGUAGE);
    await vpn.wait();
    await vpn.waitForElement(settingsView.BACK);

    await vpn.setElementProperty(
      'settingsLanguagesView-flickable', 'contentY', 'i', 0);
    await vpn.wait();

    await vpn.waitForElementProperty(settingsView.BACK, 'visible', 'true');
    await vpn.waitForElementProperty(
      settingsView.systemPreferenceView.languageSettingsView.systemLanguageToggle, 'checked', 'false');

    await vpn.clickOnElement(settingsView.systemPreferenceView.languageSettingsView.systemLanguageToggle);
    await vpn.waitForElementProperty(
      settingsView.systemPreferenceView.languageSettingsView.systemLanguageToggle, 'checked', 'true');

    await vpn.clickOnElement(settingsView.BACK);
    await vpn.wait();
    await vpn.clickOnElement(settingsView.BACK);
    await vpn.wait();

    await vpn.waitForElement(settingsView.USER_PROFILE);
    await vpn.waitForElementProperty(
      settingsView.USER_PROFILE, 'visible', 'true');
  });

  // TODO: app-permission

  it('Checking the about us', async () => {
    await vpn.waitForElement(settingsView.ABOUT_US);
    await vpn.waitForElementProperty(settingsView.ABOUT_US, 'visible', 'true');

    await vpn.setElementProperty(settingsView.SCREEN, 'contentY', 'i',
      parseInt(await vpn.getElementProperty(settingsView.ABOUT_US, 'y')));
    await vpn.wait();

    await vpn.clickOnElement(settingsView.ABOUT_US);
    await vpn.wait();

    await vpn.waitForElement(settingsView.BACK);
    await vpn.waitForElementProperty(settingsView.BACK, 'visible', 'true');

    await vpn.waitForElement(settingsView.aboutUsView.LIST);

    await vpn.waitForElement('viewAboutUs');
    await vpn.waitForElement(settingsView.aboutUsView.TOS);
    await vpn.waitForElementProperty(
      settingsView.aboutUsView.TOS, 'visible', 'true');
    await vpn.waitForElement(settingsView.aboutUsView.PRIVACY);
    await vpn.waitForElementProperty(
      settingsView.aboutUsView.PRIVACY, 'visible', 'true');
    await vpn.waitForElement(settingsView.aboutUsView.LICENSE);
    await vpn.waitForElementProperty(
      settingsView.aboutUsView.LICENSE, 'visible', 'true');

    await vpn.clickOnElement(settingsView.aboutUsView.TOS);
    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.endsWith('/r/vpn/terms');
    });

    await vpn.clickOnElement(settingsView.aboutUsView.PRIVACY);
    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.endsWith('/r/vpn/privacy');
    });

    await vpn.clickOnElement(settingsView.aboutUsView.LICENSE);

    await vpn.waitForElement(settingsView.BACK);
    await vpn.waitForElementProperty(settingsView.BACK, 'visible', 'true');
    await vpn.clickOnElement(settingsView.BACK);
    await vpn.wait();

    await vpn.waitForElement(settingsView.BACK);
    await vpn.clickOnElement(settingsView.BACK);
    await vpn.wait();

    await vpn.waitForElement(settingsView.USER_PROFILE);
    await vpn.waitForElementProperty(
      settingsView.USER_PROFILE, 'visible', 'true');
  });

  it('Checking the get help', async () => {
    await vpn.waitForElement(settingsView.GET_HELP);
    await vpn.waitForElementProperty(settingsView.GET_HELP, 'visible', 'true');

    await vpn.setElementProperty(
      settingsView.SCREEN, 'contentY', 'i',
      parseInt(await vpn.getElementProperty(settingsView.GET_HELP, 'y')));
    await vpn.wait();

    await vpn.clickOnElement(settingsView.GET_HELP);
    await vpn.wait();

    await vpn.waitForElement(settingsView.getHelpView.BACK);
    await vpn.waitForElementProperty(settingsView.getHelpView.BACK, 'visible', 'true');

    await vpn.waitForElement(settingsView.getHelpView.LINKS);
    await vpn.waitForElementProperty(settingsView.getHelpView.LINKS, 'visible', 'true');

    await vpn.waitForElement(settingsView.getHelpView.FEEDBACK);
    await vpn.waitForElementProperty(settingsView.getHelpView.FEEDBACK, 'visible', 'true');

    await vpn.clickOnElement(settingsView.getHelpView.FEEDBACK);
    await vpn.wait();

    await vpn.waitForElement(settingsView.getHelpView.giveFeedbackView.SCREEN);
    await vpn.waitForElementProperty(settingsView.getHelpView.giveFeedbackView.SCREEN, 'visible', 'true');
    await vpn.clickOnElement(settingsView.getHelpView.giveFeedbackView.SCREEN);
    await vpn.wait();

    await vpn.waitForElement(settingsView.getHelpView.BACK);
    await vpn.clickOnElement(settingsView.getHelpView.BACK);
    await vpn.wait();

    await vpn.waitForElement(settingsView.getHelpView.LINKS);
    await vpn.waitForElementProperty(settingsView.getHelpView.LINKS, 'visible', 'true');

    // TODO: checking the give feedback views

    await vpn.waitForElement(settingsView.getHelpView.LOGS);
    await vpn.waitForElementProperty(settingsView.getHelpView.LOGS, 'visible', 'true');
    await vpn.clickOnElement(settingsView.getHelpView.LOGS);

    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.startsWith('file://') && url.includes('mozillavpn') &&
        url.endsWith('.txt');
    });

    await vpn.waitForElement(settingsView.getHelpView.HELP_CENTER);
    await vpn.waitForElementProperty(settingsView.getHelpView.HELP_CENTER, 'visible', 'true');

    await vpn.clickOnElement(settingsView.getHelpView.HELP_CENTER);
    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.endsWith('/r/vpn/support');
    });

    await vpn.wait();
    await vpn.waitForElement(settingsView.getHelpView.BACK);
    await vpn.clickOnElement(settingsView.getHelpView.BACK);

    await vpn.waitForElement(settingsView.GET_HELP);
    await vpn.waitForElementProperty(settingsView.GET_HELP, 'visible', 'true');
  });

  it('Get help is opened and closed', async () => {
    await getToGetHelpView();

    await vpn.wait();
    await vpn.waitForElement(settingsView.getHelpView.BACK);
    await vpn.clickOnElement(settingsView.getHelpView.BACK);

    await vpn.wait();
    await vpn.waitForElement(settingsView.GET_HELP);
    await vpn.waitForElementProperty(settingsView.GET_HELP, 'visible', 'true');
  });

  it('Give feedback is opened and closed', async () => {
    await getToGetHelpView();
    await vpn.wait();

    await vpn.waitForElement(settingsView.getHelpView.FEEDBACK);
    await vpn.waitForElementProperty(settingsView.getHelpView.FEEDBACK, 'visible', 'true');
    await vpn.clickOnElement(settingsView.getHelpView.FEEDBACK);
    await vpn.wait();

    await vpn.wait();
    await vpn.waitForElement(settingsView.getHelpView.BACK);
    await vpn.clickOnElement(settingsView.getHelpView.BACK);
    await vpn.wait();
  });

  it('Contact us is opened and closed', async () => {
    await getToGetHelpView();
    await vpn.wait();

    await vpn.waitForElement(settingsView.getHelpView.SUPPORT);
    await vpn.waitForElementProperty(settingsView.getHelpView.SUPPORT, 'visible', 'true');
    await vpn.clickOnElement(settingsView.getHelpView.SUPPORT);

    await vpn.wait();
    await vpn.waitForElement(settingsView.getHelpView.contactSupportView.USER_INFO);
    await vpn.clickOnElement(settingsView.getHelpView.BACK);
    await vpn.wait();
  });

  it('Checking the preferences settings', async () => {
    await vpn.waitForElement(settingsView.SYSTEM_PREFERENCE);
    await vpn.waitForElementProperty(settingsView.SYSTEM_PREFERENCE, 'visible', 'true');

    await vpn.setElementProperty(settingsView.SCREEN, 'contentY', 'i',
      parseInt(await vpn.getElementProperty(settingsView.SYSTEM_PREFERENCE, 'y')));
    await vpn.wait();

    await vpn.clickOnElement(settingsView.SYSTEM_PREFERENCE);
    await vpn.wait();

    await checkSetting('dataCollection', 'glean-enabled');

    await vpn.clickOnElement(settingsView.BACK);
    await vpn.wait();

    await vpn.waitForElement(settingsView.USER_PROFILE);
    await vpn.waitForElementProperty(settingsView.USER_PROFILE, 'visible', 'true');
  });

  it('Checking the notifications settings', async () => {
    await vpn.waitForElement(settingsView.SYSTEM_PREFERENCE);
    await vpn.waitForElementProperty(settingsView.SYSTEM_PREFERENCE, 'visible', 'true');

    await vpn.clickOnElement(settingsView.SYSTEM_PREFERENCE);
    await vpn.wait();

    await vpn.waitForElement(settingsView.systemPreferenceView.NOTIFICATIONS);
    await vpn.waitForElementProperty(settingsView.systemPreferenceView.NOTIFICATIONS, 'visible', 'true');

    await vpn.setElementProperty(settingsView.SCREEN, 'contentY', 'i',
      parseInt(await vpn.getElementProperty(settingsView.systemPreferenceView.NOTIFICATIONS, 'y')));
    await vpn.wait();

    await vpn.clickOnElement(settingsView.systemPreferenceView.NOTIFICATIONS);
    await vpn.wait();

    /* TODO: captive-portal disabled
        await checkSetting('settingCaptivePortalAlert', 'captive-portal-alert');
        await checkSetting(
            'settingUnsecuredNetworkAlert', 'unsecured-network-alert');
    */
    await checkSetting('switchServersAlert', 'server-switch-notification');
    await checkSetting('connectionChangeAlert', 'connection-change-notification');

    await vpn.clickOnElement(settingsView.BACK);
    await vpn.wait();

    await vpn.clickOnElement(settingsView.BACK);

    await vpn.waitForElement(settingsView.USER_PROFILE);
    await vpn.waitForElementProperty(settingsView.USER_PROFILE, 'visible', 'true');
  });

  it.only('Checking the logout', async () => {
    await vpn.waitForElement(settingsView.SIGN_OUT);
    await vpn.waitForElementProperty(settingsView.SIGN_OUT, 'visible', 'true');
    await vpn.scrollToElement(settingsView.SCREEN, settingsView.SIGN_OUT);

    await vpn.clickOnElement(settingsView.SIGN_OUT);
    await vpn.waitForMainView();
  });
});
