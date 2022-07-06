/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
const assert = require('assert');
const vpn = require('./helper.js');

describe('Settings', function() {
  this.timeout(60000);
  this.ctx.authenticationNeeded = true;

  beforeEach(async () => {
    await vpn.waitForElement('settingsButton');
    await vpn.clickOnElement('settingsButton');
    await vpn.wait();

    await vpn.flipFeatureOff('subscriptionManagement');
  });

  async function getMainStackViewDepth() {
    await vpn.waitForElement('MainStackView');
    return await vpn.getElementProperty('MainStackView', 'depth')
  }

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
    await vpn.waitForElement('settingsBackButton');
    await vpn.waitForElementProperty('settingsBackButton', 'visible', 'true');


    await vpn.waitForElement('menuIcon');
    await vpn.waitForElementProperty(
        'menuIcon', 'source', 'qrc:/nebula/resources/close-dark.svg');

    await vpn.waitForElement('settingsUserProfile-manageAccountButton');
    await vpn.waitForElementProperty(
        'settingsUserProfile-manageAccountButton', 'visible', 'true');

    await vpn.clickOnElement('settingsBackButton');
    await vpn.wait();

    await vpn.waitForElement('controllerTitle');
    await vpn.waitForElementProperty('controllerTitle', 'visible', 'true');
  });

  it('Settings is pushed to, and popped from, mainStackView', async () => {
    await vpn.waitForElement('settingsBackButton');
    await vpn.waitForElementProperty('settingsBackButton', 'visible', 'true');

    const stackDepth = await getMainStackViewDepth();

    await vpn.clickOnElement('settingsBackButton');
    await vpn.wait();
    await vpn.waitForElement('controllerTitle');
    await vpn.waitForElementProperty('controllerTitle', 'visible', 'true');

    assert(
        await getMainStackViewDepth() ===
        (parseInt(stackDepth) - 1).toString());
  });

  it('Checking settings entries', async () => {
    await vpn.waitForElement('settingsUserProfile-manageAccountButton');
    await vpn.waitForElementProperty(
        'settingsUserProfile-manageAccountButton', 'visible', 'true');

    if (!(await vpn.isFeatureFlippedOn('subscriptionManagement'))) {
      await vpn.clickOnElement('settingsUserProfile-manageAccountButton');
      await vpn.waitForCondition(async () => {
        const url = await vpn.getLastUrl();
        return url.includes('/r/vpn/account');
      });
    }
  });

  it('Checking the tips and tricks settings', async () => {
    await vpn.waitForElement('settingsTipsAndTricks');
    await vpn.waitForElementProperty('settingsTipsAndTricks', 'visible', 'true');

    await vpn.setElementProperty(
        'settingsView', 'contentY', 'i',
        parseInt(await vpn.getElementProperty('settingsTipsAndTricks', 'y')));
    await vpn.wait();

    await vpn.clickOnElement('settingsTipsAndTricks');
    await vpn.wait();

    await vpn.waitForElement('settingsTipsAndTricksPage');

    //Test feature tour
    let featureTourFeatures = await vpn.featureTourFeatures();

    await vpn.setElementProperty(
      'settingsTipsAndTricksPage', 'contentY', 'i',
      parseInt(await vpn.getElementProperty('featureTourCard', 'y')));
    await vpn.wait();

    await vpn.waitForElement('featureTourCard');
    await vpn.clickOnElement('featureTourCard');
    await vpn.wait();

    await vpn.waitForElement('featureSwipeView');

    for(var i = 0; i < featureTourFeatures.length - 1; i++) {
      if (await vpn.getElementProperty('featureTourSecondaryButton', 'visible') === 'true') {
        await vpn.clickOnElement('featureTourSecondaryButton');
        await vpn.wait();
      }

      await vpn.clickOnElement('featureTourPrimaryButton');
      await vpn.wait();
    }

    for(var i = 0; i < featureTourFeatures.length - 1; i++) {
      await vpn.clickOnElement('backButton');
      await vpn.wait();
    }

    for(var i = 0; i < featureTourFeatures.length; i++) {
      await vpn.clickOnElement('featureTourPrimaryButton');
      await vpn.wait();
    }

    await vpn.waitForElement('featureTourCard');
    await vpn.clickOnElement('featureTourCard');
    await vpn.wait();

    await vpn.waitForElement('featureTourPopupCloseButton');
    await vpn.clickOnElement('featureTourPopupCloseButton')
    await vpn.wait();

    //Test guides
    let guides = await vpn.guides();
    let guideParent = 'guideLayout'

    for (var guide of guides) {
      guide = guideParent + "/" + guide;
      
      await vpn.setElementProperty(
        'settingsTipsAndTricksPage', 'contentY', 'i',
        parseInt(await vpn.getElementProperty(guide, 'y')) +
            parseInt(await vpn.getElementProperty(guideParent, 'y')));
      await vpn.wait();

      await vpn.waitForElement(guide);
      await vpn.clickOnElement(guide);
      await vpn.wait();

      await vpn.waitForElement('backArrow');
      await vpn.clickOnElement('backArrow')
      await vpn.wait();

      await vpn.waitForElement('settingsTipsAndTricksPage');
    }

    await vpn.waitForElement('settingsBackButton');
    await vpn.waitForElementProperty('settingsBackButton', 'visible', 'true');
    await vpn.clickOnElement('settingsBackButton');
    await vpn.wait();

    await vpn.waitForElement('settingsUserProfile-manageAccountButton');
    await vpn.waitForElementProperty(
        'settingsUserProfile-manageAccountButton', 'visible', 'true');
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

    await checkSetting('settingLocalNetworkAccess', 'local-network-access');

    await vpn.waitForElement('settingsBackButton');
    await vpn.waitForElementProperty('settingsBackButton', 'visible', 'true');
    await vpn.clickOnElement('settingsBackButton');
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
        'settingsLanguagesView', 'contentY', 'i',
        parseInt(await vpn.getElementProperty(
            'languageList/language-column-it/language-it', 'y')));
    await vpn.wait();

    await vpn.waitForElement('languageList/language-column-it/language-it');
    await vpn.waitForElementProperty(
        'languageList/language-column-it/language-it', 'visible', 'true');
    await vpn.clickOnElement('languageList/language-column-it/language-it');
    await vpn.wait();

    await vpn.waitForElement('settingsBackButton');
    await vpn.waitForElementProperty('settingsBackButton', 'visible', 'true');
    await vpn.clickOnElement('settingsBackButton');
    await vpn.wait();

    await vpn.clickOnElement('settingsBackButton');
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

    await vpn.waitForElement('settingsBackButton');
    await vpn.waitForElementProperty(
        'settingsBackButton', 'visible', 'true');
    await vpn.waitForElementProperty(
        'settingsSystemLanguageToggle', 'checked', 'false');

    await vpn.setElementProperty(
        'settingsLanguagesView', 'contentY', 'i',
        parseInt(await vpn.getElementProperty(
            'languageList/language-column-en/language-en', 'y')));
    await vpn.wait();

    await vpn.waitForElement('languageList/language-column-en/language-en');
    await vpn.waitForElementProperty(
        'languageList/language-column-en/language-en', 'visible', 'true');
    await vpn.clickOnElement('languageList/language-column-en/language-en');
    await vpn.wait();

    await vpn.clickOnElement('settingsBackButton');
    await vpn.wait();
    await vpn.clickOnElement('settingsBackButton');
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

    await vpn.waitForElement('settingsUserProfile-manageAccountButton');
    await vpn.waitForElementProperty(
        'settingsUserProfile-manageAccountButton', 'visible', 'true');
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

    await vpn.waitForElement('settingsBackButton');
    await vpn.waitForElementProperty('settingsBackButton', 'visible', 'true');
    await vpn.clickOnElement('settingsBackButton');
    await vpn.wait();

    await vpn.waitForElement('settingsBackButton');
    await vpn.clickOnElement('settingsBackButton');
    await vpn.wait();

    await vpn.waitForElement('settingsUserProfile-manageAccountButton');
    await vpn.waitForElementProperty(
        'settingsUserProfile-manageAccountButton', 'visible', 'true');
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

    await vpn.waitForElement('getHelpBack');
    await vpn.waitForElementProperty('getHelpBack', 'visible', 'true');

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

    await vpn.waitForElement('giveFeedbackBackButton');
    await vpn.waitForElementProperty(
        'giveFeedbackBackButton', 'visible', 'true');
    await vpn.clickOnElement('giveFeedbackBackButton');
    await vpn.wait();

    await vpn.waitForElement('getHelpLinks');
    await vpn.waitForElementProperty('getHelpLinks', 'visible', 'true');

    // TODO: checking the give feedback views

    await vpn.waitForElement('getHelpLinks/getHelpBackList-2');
    await vpn.waitForElementProperty(
        'getHelpLinks/getHelpBackList-2', 'visible', 'true');

    await vpn.clickOnElement('getHelpLinks/getHelpBackList-2');

    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.startsWith('file://') && url.includes('mozillavpn') &&
          url.endsWith('.txt');
    });

    await vpn.waitForElement('getHelpLinks/getHelpBackList-0')
        await vpn.waitForElementProperty(
            'getHelpLinks/getHelpBackList-0', 'visible', 'true');

    await vpn.clickOnElement('getHelpLinks/getHelpBackList-0');
    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.endsWith('/r/vpn/support');
    });

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
    await vpn.waitForElement('getHelpBack');
    await vpn.clickOnElement('getHelpBack');

    await vpn.waitForElement('settingsGetHelp');
    await vpn.waitForElementProperty('settingsGetHelp', 'visible', 'true');
  });

  it('Get help is pushed to, and popped from, mainStackView', async () => {
    const mainStackDepth = await getMainStackViewDepth();
    await getToGetHelpView();
    assert(
        await getMainStackViewDepth() ===
        (parseInt(mainStackDepth) + 1).toString());

    await vpn.wait();
    await vpn.waitForElement('getHelpBack');
    await vpn.clickOnElement('getHelpBack');

    await vpn.wait();
    await vpn.waitForElement('settingsGetHelp');
    await vpn.waitForElementProperty('settingsGetHelp', 'visible', 'true');
    assert(await getMainStackViewDepth() === mainStackDepth.toString());
  });

  it('Give feedback is pushed to, and popped from, mainStackView', async () => {
    await getToGetHelpView();
    await vpn.wait();
    const mainStackDepth = await getMainStackViewDepth();

    await vpn.waitForElement('getHelpLinks/settingsGiveFeedback');
    await vpn.waitForElementProperty(
        'getHelpLinks/settingsGiveFeedback', 'visible', 'true');

    await vpn.clickOnElement('getHelpLinks/settingsGiveFeedback');
    await vpn.wait();

    assert(
        await getMainStackViewDepth() ===
        (parseInt(mainStackDepth) + 1).toString());

    await vpn.wait();
    await vpn.waitForElement('giveFeedbackBackButton');
    await vpn.clickOnElement('giveFeedbackBackButton');
    await vpn.wait();

    assert(
        await getMainStackViewDepth() === parseInt(mainStackDepth).toString());
  });


  it('Contact us is pushed to, and popped from, mainStackView', async () => {
    await getToGetHelpView();
    await vpn.wait();
    const mainStackDepth = await getMainStackViewDepth();

    await vpn.waitForElement('getHelpLinks/getHelpBackList-1')
        await vpn.waitForElementProperty(
            'getHelpLinks/getHelpBackList-1', 'visible', 'true');

    await vpn.clickOnElement('getHelpLinks/getHelpBackList-1');

    await vpn.wait();

    assert(
        await getMainStackViewDepth() ===
        (parseInt(mainStackDepth) + 1).toString());

    await vpn.wait();
    await vpn.waitForElement('supportTicketScreen');
    await vpn.clickOnElement('supportTicketScreen');
    await vpn.wait();

    assert(
        await getMainStackViewDepth() === parseInt(mainStackDepth).toString());
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

    await vpn.waitForElement('settingsUserProfile-manageAccountButton');
    await vpn.waitForElementProperty(
        'settingsUserProfile-manageAccountButton', 'visible', 'true');
  });

  it('Checking the logout', async () => {
    await vpn.waitForElement('settingsLogout');
    await vpn.waitForElementProperty('settingsLogout', 'visible', 'true');
    await vpn.scrollToElement('settingsView', 'settingsLogout');

    await vpn.clickOnElement('settingsLogout');
    await vpn.waitForMainView();
  });
});
