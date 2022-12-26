/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const queries = require('./queries.js');
const vpn = require('./helper.js');

describe('Settings', function () {
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
    assert(
        (await vpn.getQueryProperty(query, 'isChecked') === 'true') ===
        await vpn.getSetting(settingKey));

    await vpn.setSetting(settingKey, true);
    assert((await vpn.getSetting(settingKey)) === true);
    assert((await vpn.getQueryProperty(query, 'isChecked')) === 'true');

    await vpn.setSetting(settingKey, false);
    assert((await vpn.getSetting(settingKey)) === false);
    assert((await vpn.getQueryProperty(query, 'isChecked')) === 'false');
  }

  async function getToGetHelpView() {
    await vpn.waitForQueryAndClick(queries.screenSettings.GET_HELP.visible());
    await vpn.waitForQuery(queries.screenGetHelp.BACK_BUTTON.visible());
    await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
  }

  it('Checking settings entries', async () => {
    await vpn.waitForQuery(queries.screenSettings.USER_PROFILE.visible());

    if ((await vpn.isFeatureFlippedOff('subscriptionManagement'))) {
      await vpn.clickOnQuery(queries.screenSettings.USER_PROFILE.visible());
      await vpn.waitForCondition(async () => {
        const url = await vpn.getLastUrl();
        return url.includes('https://accounts.stage.mozaws.net') &&
          url.includes('?email=test@mozilla.com');
      });
    }
  });

  it('Checking the tips and tricks settings', async () => {
    await vpn.waitForQuery(queries.screenSettings.TIPS_AND_TRICKS.visible());
    await vpn.scrollToQuery(
        queries.screenSettings.SCREEN, queries.screenSettings.TIPS_AND_TRICKS);

    await vpn.clickOnQuery(queries.screenSettings.TIPS_AND_TRICKS.visible());
    await vpn.waitForQuery(queries.screenSettings.tipsAndTricksView.SCREEN);


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

    await vpn.waitForQueryAndClick(
        queries.screenSettings.tipsAndTricksView.BACK.visible());

    await vpn.waitForQuery(queries.screenSettings.USER_PROFILE.visible());
  });

  it('Checking the networking settings', async () => {
    await vpn.waitForQuery(queries.screenSettings.NETWORK_SETTINGS.visible());

    await vpn.scrollToQuery(
        queries.screenSettings.networkSettingsView.SCREEN,
        queries.screenSettings.NETWORK_SETTINGS);

    await vpn.clickOnQuery(queries.screenSettings.NETWORK_SETTINGS.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    await vpn.waitForQueryAndClick(queries.screenSettings.BACK.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    await vpn.waitForQuery(queries.screenSettings.USER_PROFILE.visible());
  });

  it('Checking the languages settings', async () => {
    await vpn.setSetting('languageCode', '');

    await vpn.waitForQueryAndClick(
        queries.screenSettings.SYSTEM_PREFERENCE.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    await vpn.waitForQueryAndClick(
        queries.screenSettings.systemPreferenceView.LANGUAGE.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    await vpn.waitForQueryAndClick(
        queries.screenSettings.systemPreferenceView.languageSettingsView
            .SYSTEM_LANGUAGE_TOGGLE.visible()
            .checked());
    await vpn.waitForQuery(
        queries.screenSettings.systemPreferenceView.languageSettingsView
            .SYSTEM_LANGUAGE_TOGGLE.visible()
            .unchecked());

    await vpn.scrollToQuery(
        queries.screenSettings.systemPreferenceView.languageSettingsView.SCREEN,
        queries.screenSettings.systemPreferenceView.languageSettingsView
            .languageItem('it'));

    await vpn.waitForQueryAndClick(
        queries.screenSettings.systemPreferenceView.languageSettingsView
            .languageItemLabel('it'));

    await vpn.waitForQueryAndClick(queries.screenSettings.BACK.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    await vpn.waitForQueryAndClick(queries.screenSettings.BACK.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    await vpn.waitForQuery(queries.screenSettings.USER_PROFILE.visible());

    await vpn.waitForQueryAndClick(
        queries.screenSettings.SYSTEM_PREFERENCE.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    await vpn.waitForQueryAndClick(
        queries.screenSettings.systemPreferenceView.LANGUAGE.visible().prop(
            'settingTitle', 'Lingua'));
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    await vpn.waitForQuery(queries.screenSettings.BACK.visible());
    await vpn.waitForQuery(
        queries.screenSettings.systemPreferenceView.languageSettingsView
            .SYSTEM_LANGUAGE_TOGGLE.unchecked());

    await vpn.scrollToQuery(
        queries.screenSettings.systemPreferenceView.languageSettingsView.SCREEN,
        queries.screenSettings.systemPreferenceView.languageSettingsView
            .languageItem('en'));

    await vpn.waitForQueryAndClick(
        queries.screenSettings.systemPreferenceView.languageSettingsView
            .languageItemLabel('en')
            .visible());

    await vpn.waitForQueryAndClick(queries.screenSettings.BACK.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    await vpn.waitForQueryAndClick(queries.screenSettings.BACK.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    await vpn.waitForQuery(queries.screenSettings.USER_PROFILE.visible());

    await vpn.waitForQueryAndClick(
        queries.screenSettings.SYSTEM_PREFERENCE.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    await vpn.waitForQueryAndClick(
        queries.screenSettings.systemPreferenceView.LANGUAGE.visible().prop(
            'settingTitle', 'Language'));
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    await vpn.setQueryProperty(
        queries.screenSettings.systemPreferenceView.languageSettingsView.SCREEN,
        'contentY', 0);

    await vpn.waitForQueryAndClick(
        queries.screenSettings.systemPreferenceView.languageSettingsView
            .SYSTEM_LANGUAGE_TOGGLE.visible()
            .unchecked());
    await vpn.waitForQuery(
        queries.screenSettings.systemPreferenceView.languageSettingsView
            .SYSTEM_LANGUAGE_TOGGLE.checked());

    await vpn.waitForQueryAndClick(queries.screenSettings.BACK.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    await vpn.waitForQueryAndClick(queries.screenSettings.BACK.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    await vpn.waitForQuery(queries.screenSettings.USER_PROFILE.visible());
  });

  // TODO: app-permission

  it('Checking the about us', async () => {
    await vpn.waitForQuery(queries.screenSettings.ABOUT_US.visible());

    await vpn.scrollToQuery(
        queries.screenSettings.SCREEN, queries.screenSettings.ABOUT_US);

    await vpn.clickOnQuery(queries.screenSettings.ABOUT_US.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    await vpn.waitForQuery(queries.screenSettings.BACK.visible());

    await vpn.waitForQuery(queries.screenSettings.aboutUsView.SCREEN);
    await vpn.waitForQuery(queries.screenSettings.aboutUsView.LIST);
    await vpn.waitForQuery(queries.screenSettings.aboutUsView.TOS.visible());
    await vpn.waitForQuery(
        queries.screenSettings.aboutUsView.PRIVACY.visible());
    await vpn.waitForQuery(
        queries.screenSettings.aboutUsView.LICENSE.visible());

    await vpn.waitForQueryAndClick(
        queries.screenSettings.aboutUsView.TOS.visible());
    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.endsWith('/r/vpn/terms');
    });

    await vpn.waitForQueryAndClick(
        queries.screenSettings.aboutUsView.PRIVACY.visible());
    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.endsWith('/r/vpn/privacy');
    });

    await vpn.waitForQueryAndClick(
        queries.screenSettings.aboutUsView.LICENSE.visible());

    await vpn.clickOnQuery(queries.screenSettings.BACK.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    await vpn.clickOnQuery(queries.screenSettings.BACK.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    await vpn.waitForQuery(queries.screenSettings.USER_PROFILE.visible());
  });

  it('Checking the get help', async () => {
    await vpn.waitForQuery(queries.screenSettings.GET_HELP.visible());

    await vpn.scrollToQuery(
        queries.screenSettings.SCREEN, queries.screenSettings.GET_HELP);

    await vpn.clickOnQuery(queries.screenSettings.GET_HELP.visible());
    await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());

    await vpn.waitForQuery(queries.screenGetHelp.BACK_BUTTON.visible());
    await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());

    await vpn.waitForQuery(queries.screenGetHelp.LINKS.visible());

    await vpn.waitForQuery(queries.screenGetHelp.FEEDBACK.visible());
    await vpn.waitForQueryAndClick(queries.screenGetHelp.FEEDBACK.visible());
    await vpn.waitForQueryAndClick(
        queries.screenGetHelp.giveFeedbackView.SCREEN.visible());

    await vpn.waitForQueryAndClick(queries.screenGetHelp.BACK_BUTTON.visible());
    await vpn.waitForQuery(queries.screenGetHelp.STACKVIEW.ready());

    await vpn.waitForQuery(queries.screenGetHelp.LINKS.visible());

    // TODO: checking the give feedback views

    await vpn.waitForQueryAndClick(queries.screenGetHelp.LOGS.visible());

    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.startsWith('file://') && url.includes('mozillavpn') &&
        url.endsWith('.txt');
    });

    await vpn.waitForQueryAndClick(queries.screenGetHelp.HELP_CENTER.visible());
    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.endsWith('/firefox-private-network-vpn');
    });

    await vpn.waitForQueryAndClick(queries.screenGetHelp.BACK_BUTTON.visible());
    await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());

    await vpn.waitForQuery(queries.screenSettings.GET_HELP.visible());
  });

  it('Get help is opened and closed', async () => {
    await getToGetHelpView();

    await vpn.waitForQueryAndClick(queries.screenGetHelp.BACK_BUTTON.visible());
    await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());

    await vpn.waitForQuery(queries.screenSettings.GET_HELP.visible());
  });

  it('Give feedback is opened and closed', async () => {
    await getToGetHelpView();
    await vpn.waitForQueryAndClick(queries.screenGetHelp.FEEDBACK.visible());
    await vpn.waitForQueryAndClick(queries.screenGetHelp.BACK_BUTTON.visible());
  });

  it('Contact us is opened and closed', async () => {
    await getToGetHelpView();
    await vpn.waitForQueryAndClick(queries.screenGetHelp.SUPPORT.visible());

    await vpn.waitForQueryAndClick(
        queries.screenGetHelp.contactSupportView.USER_INFO.visible());
  });

  it('Checking the preferences settings', async () => {
    await vpn.waitForQuery(queries.screenSettings.SYSTEM_PREFERENCE.visible());

    await vpn.scrollToQuery(
        queries.screenSettings.SCREEN,
        queries.screenSettings.SYSTEM_PREFERENCE);

    await vpn.clickOnQuery(queries.screenSettings.SYSTEM_PREFERENCE.visible());

    await checkSetting(
        queries.screenSettings.systemPreferenceView.DATA_COLLECTION,
        'gleanEnabled');

    await vpn.waitForQueryAndClick(queries.screenSettings.BACK.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    await vpn.waitForQuery(queries.screenSettings.USER_PROFILE.visible());
  });

  it('Checking the notifications settings', async () => {
    await vpn.waitForQuery(queries.screenSettings.SYSTEM_PREFERENCE.visible());

    await vpn.scrollToQuery(
        queries.screenSettings.SCREEN,
        queries.screenSettings.SYSTEM_PREFERENCE);

    await vpn.clickOnQuery(queries.screenSettings.SYSTEM_PREFERENCE.visible());

    await vpn.waitForQuery(
        queries.screenSettings.systemPreferenceView.NOTIFICATIONS.visible());

    await vpn.scrollToQuery(
        queries.screenSettings.SCREEN,
        queries.screenSettings.systemPreferenceView.NOTIFICATIONS);

    await vpn.clickOnQuery(
        queries.screenSettings.systemPreferenceView.NOTIFICATIONS.visible());

    await checkSetting(
        queries.screenSettings.systemPreferenceView.notificationView
            .CAPTIVE_PORTAL_ALERT,
        'captivePortalAlert');
    await checkSetting(
        queries.screenSettings.systemPreferenceView.notificationView
            .UNSECURE_NETWORK_ALERT,
        'unsecuredNetworkAlert');
    await checkSetting(
        queries.screenSettings.systemPreferenceView.notificationView
            .SWITCH_SERVER_ALERT,
        'serverSwitchNotification');
    await checkSetting(
        queries.screenSettings.systemPreferenceView.notificationView
            .CONNECTION_CHANGE_ALERT,
        'connectionChangeNotification');
    await checkSetting(
        queries.screenSettings.systemPreferenceView.notificationView
            .SERVER_UNAVAILABLE_ALERT,
        'serverUnavailableNotification');

    await vpn.waitForQueryAndClick(queries.screenSettings.BACK.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    await vpn.waitForQueryAndClick(queries.screenSettings.BACK.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    await vpn.waitForQuery(queries.screenSettings.USER_PROFILE.visible());
  });

  it('Checking the logout', async () => {
    await vpn.waitForQuery(queries.screenSettings.SIGN_OUT.visible());
    await vpn.scrollToQuery(
        queries.screenSettings.SCREEN,
        queries.screenSettings.SIGN_OUT.visible());

    await vpn.clickOnQuery(queries.screenSettings.SIGN_OUT.visible());
    await vpn.waitForInitialView();
  });
});
