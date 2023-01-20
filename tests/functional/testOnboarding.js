/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const queries = require('./queries.js');
const vpn = require('./helper.js');

describe('Initial view and onboarding', function() {
  this.timeout(240000);

  beforeEach(async () => {
    assert(await vpn.getLastUrl() === '');
  })

  it('Check for links on mainView', async () => {
    await vpn.waitForQuery(queries.screenInitialize.GET_HELP_LINK.visible());
    assert(await vpn.query(queries.screenInitialize.GET_STARTED.visible()));
    assert(await vpn.query(queries.screenInitialize.LEARN_MORE_LINK.visible()));
  });

  it('Open the help menu', async () => {
    await vpn.clickOnQuery(queries.screenInitialize.GET_HELP_LINK.visible());
    await vpn.waitForQuery(queries.screenGetHelp.BACK_BUTTON.visible());
  });

  it('Open help links', async () => {
    await vpn.clickOnQuery(queries.screenInitialize.GET_HELP_LINK.visible());
    await vpn.waitForQuery(queries.screenGetHelp.LINKS.visible());

    await vpn.waitForQuery(queries.screenGetHelp.HELP_CENTER.visible());
    await vpn.waitForQuery(queries.screenGetHelp.SUPPORT.visible());
    await vpn.waitForQuery(queries.screenGetHelp.LOGS.visible());

    await vpn.clickOnQuery(queries.screenGetHelp.LOGS);
    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.startsWith('file://') && url.includes('mozillavpn') &&
          url.endsWith('.txt');
    });

    await vpn.clickOnQuery(queries.screenGetHelp.HELP_CENTER.visible());
    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.endsWith('/firefox-private-network-vpn');
    });

    await vpn.clickOnQuery(queries.screenGetHelp.SUPPORT.visible());
    await vpn.waitForQuery(
        queries.screenGetHelp.contactSupportView.UNAUTH_USER_INPUTS.visible());
  });

  it('Complete the onboarding (aborting in each phase)', async () => {
    let onboardingView = 0;

    while (true) {
      assert(
          await vpn.query(queries.screenInitialize.LEARN_MORE_LINK.visible()));
      await vpn.clickOnQuery(
          queries.screenInitialize.LEARN_MORE_LINK.visible());

      await vpn.waitForQuery(
          queries.screenInitialize.SKIP_ONBOARDING.visible());

      await vpn.waitForQuery(
          queries.screenInitialize.SCREEN.prop('busy', false));

      for (let i = 0; i < onboardingView; ++i) {
        assert(await vpn.query(
            queries.screenInitialize.ONBOARDING_NEXT.visible()));
        await vpn.clickOnQuery(
            queries.screenInitialize.ONBOARDING_NEXT.visible());
        await vpn.waitForQuery(
            queries.screenInitialize.SCREEN.prop('busy', false));
      }

      assert(
          await vpn.query(queries.screenInitialize.ONBOARDING_NEXT.visible()));
      if (await vpn.getQueryProperty(
              queries.screenInitialize.ONBOARDING_NEXT, 'text') !== 'Next') {
        break;
      }

      await vpn.clickOnQuery(
          queries.screenInitialize.SKIP_ONBOARDING.visible());

      await vpn.waitForQuery(queries.screenInitialize.GET_HELP_LINK.visible());

      await vpn.waitForQuery(
          queries.screenInitialize.SCREEN.prop('busy', false));
      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());

      ++onboardingView;
    }

    assert(onboardingView, 4);
  });
});
