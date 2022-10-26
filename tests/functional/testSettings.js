/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const { navBar, settingsScreen, generalElements, getHelpScreen } = require('./elements.js');
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
    await vpn.waitForElement(settingsScreen.GET_HELP);
    await vpn.waitForElementProperty(settingsScreen.GET_HELP, 'visible', 'true');
    await vpn.clickOnElement(settingsScreen.GET_HELP);

    await vpn.wait();
    await vpn.waitForElement(getHelpScreen.BACK);
    await vpn.waitForElementProperty(getHelpScreen.BACK, 'visible', 'true');
  }

  it('Opening and closing the settings view', async () => {
    await vpn.waitForElement(navBar.HOME);
    await vpn.waitForElementProperty(navBar.HOME, 'visible', 'true');


    await vpn.waitForElement('menuIcon');
    await vpn.waitForElementProperty('menuIcon', 'source', 'qrc:/nebula/resources/close-dark.svg');

    await vpn.waitForElement(settingsScreen.USER_PROFILE);
    await vpn.waitForElementProperty(settingsScreen.USER_PROFILE, 'visible', 'true');
    await vpn.waitForElementProperty(settingsScreen.USER_PROFILE, 'enabled', 'true');

    await vpn.clickOnElement(navBar.HOME);
    await vpn.wait();

    await vpn.waitForElement(generalElements.CONTROLLER_TITLE);
    await vpn.waitForElementProperty(generalElements.CONTROLLER_TITLE, 'visible', 'true');
  });

  it('Checking settings entries', async () => {
    await vpn.waitForElement(settingsScreen.USER_PROFILE);
    await vpn.waitForElementProperty(settingsScreen.USER_PROFILE, 'visible', 'true');

    if ((await vpn.isFeatureFlippedOff('subscriptionManagement'))) {
      await vpn.clickOnElement(settingsScreen.USER_PROFILE);
      await vpn.waitForCondition(async () => {
        const url = await vpn.getLastUrl();
        return url.includes('https://accounts.stage.mozaws.net') &&
          url.includes('?email=test@mozilla.com');
      });
    }
  });

  it('Checking the tips and tricks settings', async () => {
    await vpn.waitForElement(settingsScreen.TIPS_AND_TRICKS);
    await vpn.waitForElementProperty(settingsScreen.TIPS_AND_TRICKS, 'visible', 'true');
    await vpn.setElementProperty(settingsScreen.SCREEN, 'contentY', 'i', parseInt(await vpn.getElementProperty(settingsScreen.TIPS_AND_TRICKS, 'y')));
    await vpn.wait();

    await vpn.clickOnElement(settingsScreen.TIPS_AND_TRICKS);
    await vpn.wait();
    await vpn.waitForElement(settingsScreen.tipsAndTricksView.SCREEN);


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

    await vpn.waitForElement(settingsScreen.tipsAndTricksView.BACK);
    await vpn.waitForElementProperty(settingsScreen.tipsAndTricksView.BACK, 'visible', 'true');
    await vpn.clickOnElement(settingsScreen.tipsAndTricksView.BACK);
    await vpn.wait();

    await vpn.waitForElement(settingsScreen.USER_PROFILE);
    await vpn.waitForElementProperty(settingsScreen.USER_PROFILE, 'visible', 'true');
  });

  it('Checking the networking settings', async () => {
    await vpn.waitForElement(settingsScreen.NETWORK_SETTINGS);
    await vpn.waitForElementProperty(settingsScreen.NETWORK_SETTINGS, 'visible', 'true');

    await vpn.setElementProperty(settingsScreen.networkSettingsView.SCREEN, 'contentY', 'i',
      parseInt(await vpn.getElementProperty(settingsScreen.NETWORK_SETTINGS, 'y')));
    await vpn.wait();

    await vpn.clickOnElement(settingsScreen.NETWORK_SETTINGS);
    await vpn.wait();

    await checkSetting('settingLocalNetworkAccess', 'local-network-access');

    await vpn.waitForElement(settingsScreen.BACK);
    await vpn.waitForElementProperty(settingsScreen.BACK, 'visible', 'true');
    await vpn.clickOnElement(settingsScreen.BACK);
    await vpn.wait();

    await vpn.waitForElement(settingsScreen.USER_PROFILE);
    await vpn.waitForElementProperty(settingsScreen.USER_PROFILE, 'visible', 'true');
  });

  it('Checking the languages settings', async () => {
    await vpn.setSetting('language-code', '');

    await vpn.waitForElement(settingsScreen.SYSTEM_PREFERENCE);
    await vpn.waitForElementProperty(settingsScreen.SYSTEM_PREFERENCE, 'visible', 'true');
    await vpn.clickOnElement(settingsScreen.SYSTEM_PREFERENCE);
    await vpn.wait();

    await vpn.waitForElement(settingsScreen.systemPreferenceView.LANGUAGE);
    await vpn.waitForElementProperty(settingsScreen.systemPreferenceView.LANGUAGE, 'visible', 'true');
    await vpn.clickOnElement(settingsScreen.systemPreferenceView.LANGUAGE);
    await vpn.wait();
  
    await vpn.waitForElement(settingsScreen.systemPreferenceView.languageSettingsView.systemLanguageToggle);
    await vpn.waitForElementProperty(
      settingsScreen.systemPreferenceView.languageSettingsView.systemLanguageToggle, 'visible', 'true');
    await vpn.waitForElementProperty(
      settingsScreen.systemPreferenceView.languageSettingsView.systemLanguageToggle, 'checked', 'true');

    await vpn.clickOnElement(settingsScreen.systemPreferenceView.languageSettingsView.systemLanguageToggle);
    await vpn.waitForElementProperty(
      settingsScreen.systemPreferenceView.languageSettingsView.systemLanguageToggle, 'checked', 'false');

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

    await vpn.waitForElement(settingsScreen.BACK);
    await vpn.waitForElementProperty(settingsScreen.BACK, 'visible', 'true');
    await vpn.clickOnElement(settingsScreen.BACK);
    await vpn.wait();

    await vpn.clickOnElement(settingsScreen.BACK);
    await vpn.wait();

    await vpn.waitForElement(settingsScreen.USER_PROFILE);
    await vpn.waitForElementProperty(
      settingsScreen.USER_PROFILE, 'visible', 'true');

    await vpn.waitForElement(settingsScreen.SYSTEM_PREFERENCE);
    await vpn.waitForElementProperty(settingsScreen.SYSTEM_PREFERENCE, 'visible', 'true');
    await vpn.clickOnElement(settingsScreen.SYSTEM_PREFERENCE);
    await vpn.wait();

    await vpn.waitForElement(settingsScreen.systemPreferenceView.LANGUAGE);
    await vpn.waitForElementProperty(settingsScreen.systemPreferenceView.LANGUAGE, 'visible', 'true');
    await vpn.clickOnElement(settingsScreen.systemPreferenceView.LANGUAGE);
    await vpn.wait();

    await vpn.waitForElement(settingsScreen.BACK);
    await vpn.waitForElementProperty(settingsScreen.BACK, 'visible', 'true');
    await vpn.waitForElementProperty(
      settingsScreen.systemPreferenceView.languageSettingsView.systemLanguageToggle, 'checked', 'false');

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

    await vpn.clickOnElement(settingsScreen.BACK);
    await vpn.wait();
    await vpn.clickOnElement(settingsScreen.BACK);
    await vpn.wait();

    await vpn.waitForElement(settingsScreen.USER_PROFILE);
    await vpn.waitForElementProperty(
      settingsScreen.USER_PROFILE, 'visible', 'true');

    await vpn.waitForElement(settingsScreen.SYSTEM_PREFERENCE);
    await vpn.waitForElementProperty(settingsScreen.SYSTEM_PREFERENCE, 'visible', 'true');
    await vpn.clickOnElement(settingsScreen.SYSTEM_PREFERENCE);
    await vpn.wait();

    await vpn.waitForElement(settingsScreen.systemPreferenceView.LANGUAGE);
    await vpn.waitForElementProperty(settingsScreen.systemPreferenceView.LANGUAGE, 'visible', 'true');
    await vpn.clickOnElement(settingsScreen.systemPreferenceView.LANGUAGE);
    await vpn.wait();

    await vpn.clickOnElement(settingsScreen.systemPreferenceView.LANGUAGE);
    await vpn.wait();
    await vpn.waitForElement(settingsScreen.BACK);

    await vpn.setElementProperty(
      'settingsLanguagesView-flickable', 'contentY', 'i', 0);
    await vpn.wait();

    await vpn.waitForElementProperty(settingsScreen.BACK, 'visible', 'true');
    await vpn.waitForElementProperty(
      settingsScreen.systemPreferenceView.languageSettingsView.systemLanguageToggle, 'checked', 'false');

    await vpn.clickOnElement(settingsScreen.systemPreferenceView.languageSettingsView.systemLanguageToggle);
    await vpn.waitForElementProperty(
      settingsScreen.systemPreferenceView.languageSettingsView.systemLanguageToggle, 'checked', 'true');

    await vpn.clickOnElement(settingsScreen.BACK);
    await vpn.wait();
    await vpn.clickOnElement(settingsScreen.BACK);
    await vpn.wait();

    await vpn.waitForElement(settingsScreen.USER_PROFILE);
    await vpn.waitForElementProperty(
      settingsScreen.USER_PROFILE, 'visible', 'true');
  });

  // TODO: app-permission

  it('Checking the about us', async () => {
    await vpn.waitForElement(settingsScreen.ABOUT_US);
    await vpn.waitForElementProperty(settingsScreen.ABOUT_US, 'visible', 'true');

    await vpn.setElementProperty(settingsScreen.SCREEN, 'contentY', 'i',
      parseInt(await vpn.getElementProperty(settingsScreen.ABOUT_US, 'y')));
    await vpn.wait();

    await vpn.clickOnElement(settingsScreen.ABOUT_US);
    await vpn.wait();

    await vpn.waitForElement(settingsScreen.BACK);
    await vpn.waitForElementProperty(settingsScreen.BACK, 'visible', 'true');

    await vpn.waitForElement(settingsScreen.aboutUsView.LIST);

    await vpn.waitForElement('viewAboutUs');
    await vpn.waitForElement(settingsScreen.aboutUsView.TOS);
    await vpn.waitForElementProperty(
      settingsScreen.aboutUsView.TOS, 'visible', 'true');
    await vpn.waitForElement(settingsScreen.aboutUsView.PRIVACY);
    await vpn.waitForElementProperty(
      settingsScreen.aboutUsView.PRIVACY, 'visible', 'true');
    await vpn.waitForElement(settingsScreen.aboutUsView.LICENSE);
    await vpn.waitForElementProperty(
      settingsScreen.aboutUsView.LICENSE, 'visible', 'true');

    await vpn.clickOnElement(settingsScreen.aboutUsView.TOS);
    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.endsWith('/r/vpn/terms');
    });

    await vpn.clickOnElement(settingsScreen.aboutUsView.PRIVACY);
    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.endsWith('/r/vpn/privacy');
    });

    await vpn.clickOnElement(settingsScreen.aboutUsView.LICENSE);

    await vpn.waitForElement(settingsScreen.BACK);
    await vpn.waitForElementProperty(settingsScreen.BACK, 'visible', 'true');
    await vpn.clickOnElement(settingsScreen.BACK);
    await vpn.wait();

    await vpn.waitForElement(settingsScreen.BACK);
    await vpn.clickOnElement(settingsScreen.BACK);
    await vpn.wait();

    await vpn.waitForElement(settingsScreen.USER_PROFILE);
    await vpn.waitForElementProperty(
      settingsScreen.USER_PROFILE, 'visible', 'true');
  });

  it('Checking the get help', async () => {
    await vpn.waitForElement(settingsScreen.GET_HELP);
    await vpn.waitForElementProperty(settingsScreen.GET_HELP, 'visible', 'true');

    await vpn.setElementProperty(
      settingsScreen.SCREEN, 'contentY', 'i',
      parseInt(await vpn.getElementProperty(settingsScreen.GET_HELP, 'y')));
    await vpn.wait();

    await vpn.clickOnElement(settingsScreen.GET_HELP);
    await vpn.wait();

    await vpn.waitForElement(getHelpScreen.BACK);
    await vpn.waitForElementProperty(getHelpScreen.BACK, 'visible', 'true');

    await vpn.waitForElement(getHelpScreen.LINKS);
    await vpn.waitForElementProperty(getHelpScreen.LINKS, 'visible', 'true');

    await vpn.waitForElement(getHelpScreen.FEEDBACK);
    await vpn.waitForElementProperty(getHelpScreen.FEEDBACK, 'visible', 'true');

    await vpn.clickOnElement(getHelpScreen.FEEDBACK);
    await vpn.wait();

    await vpn.waitForElement(getHelpScreen.giveFeedbackView.SCREEN);
    await vpn.waitForElementProperty(getHelpScreen.giveFeedbackView.SCREEN, 'visible', 'true');
    await vpn.clickOnElement(getHelpScreen.giveFeedbackView.SCREEN);
    await vpn.wait();

    await vpn.waitForElement(getHelpScreen.BACK);
    await vpn.clickOnElement(getHelpScreen.BACK);
    await vpn.wait();

    await vpn.waitForElement(getHelpScreen.LINKS);
    await vpn.waitForElementProperty(getHelpScreen.LINKS, 'visible', 'true');

    // TODO: checking the give feedback views

    await vpn.waitForElement(getHelpScreen.LOGS);
    await vpn.waitForElementProperty(getHelpScreen.LOGS, 'visible', 'true');
    await vpn.clickOnElement(getHelpScreen.LOGS);

    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.startsWith('file://') && url.includes('mozillavpn') &&
        url.endsWith('.txt');
    });

    await vpn.waitForElement(getHelpScreen.HELP_CENTER);
    await vpn.waitForElementProperty(getHelpScreen.HELP_CENTER, 'visible', 'true');

    await vpn.clickOnElement(getHelpScreen.HELP_CENTER);
    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.endsWith('/r/vpn/support');
    });

    await vpn.wait();
    await vpn.waitForElement(getHelpScreen.BACK);
    await vpn.clickOnElement(getHelpScreen.BACK);

    await vpn.waitForElement(settingsScreen.GET_HELP);
    await vpn.waitForElementProperty(settingsScreen.GET_HELP, 'visible', 'true');
  });

  it('Get help is opened and closed', async () => {
    await getToGetHelpView();

    await vpn.wait();
    await vpn.waitForElement(getHelpScreen.BACK);
    await vpn.clickOnElement(getHelpScreen.BACK);

    await vpn.wait();
    await vpn.waitForElement(settingsScreen.GET_HELP);
    await vpn.waitForElementProperty(settingsScreen.GET_HELP, 'visible', 'true');
  });

  it('Give feedback is opened and closed', async () => {
    await getToGetHelpView();
    await vpn.wait();

    await vpn.waitForElement(getHelpScreen.FEEDBACK);
    await vpn.waitForElementProperty(getHelpScreen.FEEDBACK, 'visible', 'true');
    await vpn.clickOnElement(getHelpScreen.FEEDBACK);
    await vpn.wait();

    await vpn.wait();
    await vpn.waitForElement(getHelpScreen.BACK);
    await vpn.clickOnElement(getHelpScreen.BACK);
    await vpn.wait();
  });

  it('Contact us is opened and closed', async () => {
    await getToGetHelpView();
    await vpn.wait();

    await vpn.waitForElement(getHelpScreen.SUPPORT);
    await vpn.waitForElementProperty(getHelpScreen.SUPPORT, 'visible', 'true');
    await vpn.clickOnElement(getHelpScreen.SUPPORT);

    await vpn.wait();
    await vpn.waitForElement(getHelpScreen.contactSupportView.USER_INFO);
    await vpn.clickOnElement(getHelpScreen.BACK);
    await vpn.wait();
  });

  it('Checking the preferences settings', async () => {
    await vpn.waitForElement(settingsScreen.SYSTEM_PREFERENCE);
    await vpn.waitForElementProperty(settingsScreen.SYSTEM_PREFERENCE, 'visible', 'true');

    await vpn.setElementProperty(settingsScreen.SCREEN, 'contentY', 'i',
      parseInt(await vpn.getElementProperty(settingsScreen.SYSTEM_PREFERENCE, 'y')));
    await vpn.wait();

    await vpn.clickOnElement(settingsScreen.SYSTEM_PREFERENCE);
    await vpn.wait();

    await checkSetting('dataCollection', 'glean-enabled');

    await vpn.clickOnElement(settingsScreen.BACK);
    await vpn.wait();

    await vpn.waitForElement(settingsScreen.USER_PROFILE);
    await vpn.waitForElementProperty(settingsScreen.USER_PROFILE, 'visible', 'true');
  });

  it('Checking the notifications settings', async () => {
    await vpn.waitForElement(settingsScreen.SYSTEM_PREFERENCE);
    await vpn.waitForElementProperty(settingsScreen.SYSTEM_PREFERENCE, 'visible', 'true');

    await vpn.clickOnElement(settingsScreen.SYSTEM_PREFERENCE);
    await vpn.wait();

    await vpn.waitForElement(settingsScreen.systemPreferenceView.NOTIFICATIONS);
    await vpn.waitForElementProperty(settingsScreen.systemPreferenceView.NOTIFICATIONS, 'visible', 'true');

    await vpn.setElementProperty(settingsScreen.SCREEN, 'contentY', 'i',
      parseInt(await vpn.getElementProperty(settingsScreen.systemPreferenceView.NOTIFICATIONS, 'y')));
    await vpn.wait();

    await vpn.clickOnElement(settingsScreen.systemPreferenceView.NOTIFICATIONS);
    await vpn.wait();

    /* TODO: captive-portal disabled
        await checkSetting('settingCaptivePortalAlert', 'captive-portal-alert');
        await checkSetting(
            'settingUnsecuredNetworkAlert', 'unsecured-network-alert');
    */
    await checkSetting('switchServersAlert', 'server-switch-notification');
    await checkSetting('connectionChangeAlert', 'connection-change-notification');

    await vpn.clickOnElement(settingsScreen.BACK);
    await vpn.wait();

    await vpn.clickOnElement(settingsScreen.BACK);

    await vpn.waitForElement(settingsScreen.USER_PROFILE);
    await vpn.waitForElementProperty(settingsScreen.USER_PROFILE, 'visible', 'true');
  });

  it('Checking the logout', async () => {
    await vpn.waitForElement(settingsScreen.SIGN_OUT);
    await vpn.waitForElementProperty(settingsScreen.SIGN_OUT, 'visible', 'true');
    await vpn.scrollToElement(settingsScreen.SCREEN, settingsScreen.SIGN_OUT);

    await vpn.clickOnElement(settingsScreen.SIGN_OUT);
    await vpn.waitForMainView();
  });
});
