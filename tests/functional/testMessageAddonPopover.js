/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const queries = require('./queries.js');
const vpn = require('./helper.js');

// Wait for the home screen and addons, then navigate away and back so that
// onCurrentComponentChanged fires with VPN.state === StateMain and
// screen === ScreenHome simultaneously — the condition maybeShowMessagePopover
// requires. Without this, all three QML triggers fire during startup before
// both conditions are true and the popover never appears.
async function waitForHomeReady() {
  await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
  await vpn.waitForMozillaProperty(
      'Mozilla.Shared', 'MZAddonManager', 'loadCompleted', 'true');
  await vpn.waitForQueryAndClick(queries.navBar.MESSAGES.visible());
  await vpn.waitForQuery(queries.screenMessaging.SCREEN.visible());
  await vpn.waitForQueryAndClick(queries.navBar.HOME.visible());
  await vpn.waitForQuery(queries.screenHome.SCREEN.visible());
}

async function loadAdditionalAddonsAndShowFirstPopover() {
  await vpn.fetchAddons('07_two_promo_messages');
  await vpn.waitForMozillaProperty(
      'Mozilla.Shared', 'MZAddonManager', 'loadCompleted', 'true');

  await vpn.waitForQuery(queries.screenMessageAddonPopover.POPOVER.visible());
  const raw = await vpn.getSetting('addonPromoLastShown');
  const lastShown = new Date(raw);
  const ageMs = Date.now() - lastShown.getTime();
  assert(
      ageMs >= 0 && ageMs < 3000,
      `addonPromoLastShown should be within last 3s, but was ${ageMs}ms ago`);
}

async function setLastAddonPopoverToFourDaysAgo() {
  // update the last time to one that allows it to be shown, as it runs once
  // every 72 hours
  const fourDaysAgo = Date.now() - (1000 * 60 * 60 * 24 * 4);
  await vpn.setLastAddonPopover(fourDaysAgo);
}

beforeEach(async () => {
  await setLastAddonPopoverToFourDaysAgo();
});

describe('Message addon popover conditions:', function() {
  this.timeout(3000000);
  this.ctx.authenticationNeeded = true;

  describe('with a Notified message that has promo_text', function() {
    beforeEach(async () => {
      await vpn.resetAddons('04_promo_message');
      await waitForHomeReady();
    })

    it('unread message w/ promo text is shown and displays correct text (happy path)',
       async () => {
         await vpn.waitForQuery(
             queries.screenMessageAddonPopover.POPOVER.visible());
         const text = await vpn.getQueryProperty(
             queries.screenMessageAddonPopover.LABEL, 'text');
         assert.equal(text, 'Tap to see your new message!');
       });

    it('does not show popover again after status is PromoShown', async () => {
      // show it
      await vpn.waitForQuery(
          queries.screenMessageAddonPopover.POPOVER.visible());

      // Dismiss and leave home.
      await vpn.waitForQueryAndClick(
          queries.screenMessageAddonPopover.CLOSE_BUTTON.visible());
      await vpn.waitForQueryAndClick(queries.navBar.MESSAGES.visible());
      await vpn.waitForQuery(queries.screenMessaging.SCREEN.visible());

      // update the last time to one that allows it to be shown
      await setLastAddonPopoverToFourDaysAgo();

      // Return home — onCurrentComponentChanged fires maybeShowMessagePopover,
      // but the message is PromoShown so addonsToPromote.count === 0.
      await vpn.waitForQueryAndClick(queries.navBar.HOME.visible());
      await vpn.waitForQuery(queries.screenHome.SCREEN.visible());
      assert.equal(
          await vpn.getQueryProperty(
              queries.screenMessageAddonPopover.POPOVER, 'visible'),
          'false');
    });

    it('does not show 2nd popover within the normal cooldown period',
       async () => {
         await loadAdditionalAddonsAndShowFirstPopover();

         // go to messages screen (dismisses the addon), go back to home screen
         await vpn.waitForQueryAndClick(queries.navBar.MESSAGES.visible());
         await vpn.waitForQuery(queries.screenMessaging.SCREEN.visible());
         await vpn.waitForQueryAndClick(queries.navBar.HOME.visible());
         await vpn.waitForQuery(queries.screenHome.SCREEN.visible());
         assert.equal(
             await vpn.getQueryProperty(
                 queries.screenMessageAddonPopover.POPOVER, 'visible'),
             'false');
       });

    it('does not show 2nd popover within the short cooldown period',
       async () => {
         await vpn.setSetting('useShortAddonPromoTime', 'true');

         await loadAdditionalAddonsAndShowFirstPopover();

         // go to messages screen (dismisses the addon), go back to home screen
         await vpn.waitForQueryAndClick(queries.navBar.MESSAGES.visible());
         await vpn.waitForQuery(queries.screenMessaging.SCREEN.visible());
         await vpn.waitForQueryAndClick(queries.navBar.HOME.visible());
         await vpn.waitForQuery(queries.screenHome.SCREEN.visible());
         assert.equal(
             await vpn.getQueryProperty(
                 queries.screenMessageAddonPopover.POPOVER, 'visible'),
             'false');
       });

    it('shows 2nd popover after the normal cooldown period', async () => {
      await loadAdditionalAddonsAndShowFirstPopover();

      await setLastAddonPopoverToFourDaysAgo();

      // go to messages screen (dismisses the addon), go back to home screen
      await vpn.waitForQueryAndClick(queries.navBar.MESSAGES.visible());
      await vpn.waitForQuery(queries.screenMessaging.SCREEN.visible());
      await vpn.waitForQueryAndClick(queries.navBar.HOME.visible());
      await vpn.waitForQuery(queries.screenHome.SCREEN.visible());
      assert.equal(
          await vpn.getQueryProperty(
              queries.screenMessageAddonPopover.POPOVER, 'visible'),
          'true');
    });

    it('shows 2nd popover after the short cooldown period', async () => {
      await vpn.setSetting('useShortAddonPromoTime', 'true');

      await loadAdditionalAddonsAndShowFirstPopover();

      // update the last time to one that allows it to be shown, as it runs once
      // every 30 seconds
      const fortySecondsAgo = Date.now() - (1000 * 40);
      await vpn.setLastAddonPopover(fortySecondsAgo);

      // go to messages screen (dismisses the addon), go back to home screen
      await vpn.waitForQueryAndClick(queries.navBar.MESSAGES.visible());
      await vpn.waitForQuery(queries.screenMessaging.SCREEN.visible());
      await vpn.waitForQueryAndClick(queries.navBar.HOME.visible());
      await vpn.waitForQuery(queries.screenHome.SCREEN.visible());
      assert.equal(
          await vpn.getQueryProperty(
              queries.screenMessageAddonPopover.POPOVER, 'visible'),
          'true');
    });
  });

  describe('with a Received message', function() {
    beforeEach(async () => {
      await vpn.resetAddons('09_promo_no_notify');
      await waitForHomeReady();
    })

    it('does not show popover when message status is Received', async () => {
      assert.equal(
          await vpn.getQueryProperty(
              queries.screenMessageAddonPopover.POPOVER, 'visible'),
          'false');
    });
  });

  describe('with a message that has no promo_text', function() {
    beforeEach(async () => {
      await vpn.resetAddons('05_promo_no_promo_text');
      await waitForHomeReady();
    })

    it('does not show popover when promo_text is absent', async () => {
      assert.equal(
          await vpn.getQueryProperty(
              queries.screenMessageAddonPopover.POPOVER, 'visible'),
          'false');
    });
  });

  describe('with a message that has empty string promo_text', function() {
    beforeEach(async () => {
      await vpn.resetAddons('06_promo_empty_text');
      await waitForHomeReady();
    })

    it('does not show popover when promo_text is empty string', async () => {
      assert.equal(
          await vpn.getQueryProperty(
              queries.screenMessageAddonPopover.POPOVER, 'visible'),
          'false');
    });
  });

  describe.only('UI works as expected', function() {
    beforeEach(async () => {
      await vpn.resetAddons('04_promo_message');
      await waitForHomeReady();
    })


    it('close button dismisses the popover', async () => {
      await vpn.waitForQuery(
          queries.screenMessageAddonPopover.POPOVER.visible());
      await vpn.waitForQueryAndClick(
          queries.screenMessageAddonPopover.CLOSE_BUTTON.visible());
      await vpn.waitForQuery(
          queries.screenMessageAddonPopover.POPOVER.hidden());
    });

    it('navigating to the location screen dismisses the popover', async () => {
      await vpn.waitForQuery(
          queries.screenMessageAddonPopover.POPOVER.visible());
      await vpn.waitForQueryAndClick(
          queries.screenHome.SERVER_LIST_BUTTON.visible());
      await vpn.waitForQuery(
          queries.screenMessageAddonPopover.POPOVER.hidden());
    });

    it('navigating to the message screen dismisses the popover', async () => {
      await vpn.waitForQuery(
          queries.screenMessageAddonPopover.POPOVER.visible());
      await vpn.waitForQueryAndClick(queries.navBar.MESSAGES.visible());
      await vpn.waitForQuery(queries.screenMessaging.SCREEN.visible());
      await vpn.waitForQuery(
          queries.screenMessageAddonPopover.POPOVER.hidden());
    });

    it('navigating to the settings screen dismisses the popover', async () => {
      await vpn.waitForQuery(
          queries.screenMessageAddonPopover.POPOVER.visible());
      await vpn.waitForQueryAndClick(queries.navBar.SETTINGS.visible());
      await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());
      await vpn.waitForQuery(
          queries.screenMessageAddonPopover.POPOVER.hidden());
    });

    it('tapping the popover navigates to the messaging screen and highlights the message',
       async () => {
         await vpn.waitForQueryAndClick(
             queries.screenMessageAddonPopover.POPOVER.visible());
         await vpn.waitForQuery(queries.screenMessaging.SCREEN.visible());
         await vpn.waitForQuery(
             queries.screenMessaging.messageItem('message_promo').visible());
         const highlight = await vpn.getQueryProperty(
             queries.screenMessaging.messageItem('message_promo'), 'highlight');
         assert.equal(highlight, 'true');
         const pauseHover = await vpn.getQueryProperty(
             queries.screenMessaging.messageItem('message_promo'),
             'pauseHover');
         assert.equal(pauseHover, 'false');
       });

    it.only(
        'when multiple messages, tapping the popover navigates to the messaging screen and highlights correct message',
        async () => {
          // change the normal setup so we show the 2 promo messages
          await vpn.resetAddons('07_two_promo_messages');
          await setLastAddonPopoverToFourDaysAgo();
          await waitForHomeReady();

          await vpn.waitForQueryAndClick(
              queries.screenMessageAddonPopover.POPOVER.visible());
          await vpn.waitForQuery(queries.screenMessaging.SCREEN.visible());
          await vpn.waitForQuery(
              queries.screenMessaging.messageItem('message_promo_a').visible());
          await vpn.waitForQuery(
              queries.screenMessaging.messageItem('message_promo_b').visible());
          const highlightA = await vpn.getQueryProperty(
              queries.screenMessaging.messageItem('message_promo_a'),
              'highlight');
          assert.equal(highlightA, 'true');
          const pauseHoverA = await vpn.getQueryProperty(
              queries.screenMessaging.messageItem('message_promo_a'),
              'pauseHover');
          assert.equal(pauseHoverA, 'false');
          const highlightB = await vpn.getQueryProperty(
              queries.screenMessaging.messageItem('message_promo_b'),
              'highlight');
          assert.equal(highlightB, 'false');
          const pauseHoverB = await vpn.getQueryProperty(
              queries.screenMessaging.messageItem('message_promo_b'),
              'pauseHover');
          assert.equal(pauseHoverB, 'true');
        });

    it('tapping the message nav bar buton does not cause message to be highlighted',
       async () => {
         await vpn.waitForQuery(
             queries.screenMessageAddonPopover.POPOVER.visible());
         await vpn.waitForQueryAndClick(queries.navBar.MESSAGES.visible());
         await vpn.waitForQuery(queries.screenMessaging.SCREEN.visible());
         await vpn.waitForQuery(
             queries.screenMessaging.messageItem('message_promo').visible());
         const highlight = await vpn.getQueryProperty(
             queries.screenMessaging.messageItem('message_promo'), 'highlight');
         assert.equal(highlight, 'false');
         const pauseHover = await vpn.getQueryProperty(
             queries.screenMessaging.messageItem('message_promo'),
             'pauseHover');
         assert.equal(pauseHover, 'false');
       });
  });
});
