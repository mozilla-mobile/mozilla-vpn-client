/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const queries = require('./queries.js');
const vpn = require('./helper.js');
const { startAndConnect } = require('./setupVpn.js');


describe('Initialize', function() {
  it('Navigating to and from the help menu is possible', async () => {
    await vpn.waitForQueryAndClick(
        queries.screenInitialize.GET_HELP_LINK.visible());
    await vpn.waitForQueryAndClick(queries.screenGetHelp.BACK_BUTTON);

    await vpn.waitForQuery(queries.screenInitialize.SWIPE_VIEW.visible());
  });

  it('SwipeView is visible', async () => {
    await vpn.waitForQuery(queries.screenInitialize.SWIPE_VIEW.visible());
  });

  it('Sign up button is visible', async () => {
    await vpn.waitForQuery(queries.screenInitialize.SIGN_UP_BUTTON.visible());
  });

  it('Already a subscriber button is visible', async () => {
    await vpn.waitForQuery(
        queries.screenInitialize.ALREADY_A_SUBSCRIBER_LINK.visible());
  });

  it('Panel title is set correctly based on StackView currentIndex',
     async () => {
       await vpn.waitForQuery(queries.screenInitialize.SWIPE_VIEW.visible());
       await vpn.setQueryProperty(
           queries.screenInitialize.SWIPE_VIEW, 'currentIndex', 0);
       await vpn.wait();
       await vpn.waitForQuery(queries.screenInitialize.PANEL_TITLE.visible());
       assert.equal(
           await vpn.getQueryProperty(
               queries.screenInitialize.PANEL_TITLE, 'text'), 'Mozilla VPN');
     });

  it('Panel description is set correctly based on StackView currentIndex',
     async () => {
       await vpn.waitForQuery(queries.screenInitialize.SWIPE_VIEW.visible());
       await vpn.setQueryProperty(
           queries.screenInitialize.SWIPE_VIEW, 'currentIndex', 0);
       await vpn.wait();
       const descriptionText = await vpn.getQueryProperty(
           queries.screenInitialize.PANEL_DESCRIPTION, 'text');
       assert(descriptionText.includes('Firefox'));
     });

  it('Panel title and description are updated when SwipeView currentIndex changes',
     async () => {
       await vpn.waitForQuery(queries.screenInitialize.SWIPE_VIEW.visible());
       await vpn.setQueryProperty(
           queries.screenInitialize.SWIPE_VIEW, 'currentIndex', 2);
       await vpn.wait();
       assert.equal(
           await vpn.getQueryProperty(
               queries.screenInitialize.PANEL_TITLE.visible(), 'text'),
           'One tap to safety');
       const descriptionText = await vpn.getQueryProperty(
           queries.screenInitialize.PANEL_DESCRIPTION, 'text');
       assert(descriptionText.includes('Protecting yourself is simple'));
     });

  it('Sign up button opens auth flow', async () => {
    await vpn.setMozillaProperty(
        'Mozilla.Shared', 'MZUrlOpener', 'lastUrl', '');
    await vpn.waitForQueryAndClick(
        queries.screenInitialize.SIGN_UP_BUTTON.visible());

    // What happens next depends on feature flagging.
    if (await vpn.isFeatureEnabled('inAppAccountCreate')) {
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible());
    } else {
      await vpn.waitForCondition(async () => {
        const url = await vpn.getLastUrl();
        return url.includes('/api/v2/vpn/login');
      });
    }
  });

  it('Already a subscriber? opens auth flow', async () => {
    await vpn.setMozillaProperty(
        'Mozilla.Shared', 'MZUrlOpener', 'lastUrl', '');
    await vpn.waitForQueryAndClick(
        queries.screenInitialize.ALREADY_A_SUBSCRIBER_LINK.visible());

    // What happens next depends on feature flagging.
    if (await vpn.isFeatureEnabled('inAppAuthentication')) {
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible());
    } else {
      await vpn.waitForCondition(async () => {
        const url = await vpn.getLastUrl();
        return url.includes('/api/v2/vpn/login');
      });
    }
  });

});
