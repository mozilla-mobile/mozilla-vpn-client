/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const queries = require('./queries.js');
const vpn = require('./helper.js');

// Navigate away from home screen to prevent accidental promo message,
// load new addons, then go back to main screen.
async function loadAddonsAndNavigateHome(testAddonName) {
  await vpn.waitForQueryAndClick(queries.navBar.MESSAGES.visible());
  await vpn.waitForQuery(queries.screenMessaging.SCREEN.visible());

  await vpn.resetAddons(testAddonName);
  await vpn.waitForMozillaProperty(
      'Mozilla.Shared', 'MZAddonManager', 'loadCompleted', 'true');

  await vpn.waitForQueryAndClick(queries.navBar.HOME.visible());
  await vpn.waitForQuery(queries.screenHome.SCREEN.visible());
}

async function setLastAddonPopoverToFourDaysAgo() {
  // update the last time to one that allows it to be shown, as it runs once
  // every 72 hours
  const fourDaysAgo = Date.now() - (1000 * 60 * 60 * 24 * 4);
  await vpn.setLastAddonPopover(fourDaysAgo);
}

beforeEach(async () => {
  await setLastAddonPopoverToFourDaysAgo();
  // Without this, some tests fail on CI, because the highlight fades before
  // all the tests are finished. This is especially true for test starting
  // with "when multiple messages...".
  await vpn.setSetting('useLengthyAddonMessageHighlight', 'true');
});

describe('Message addon popover conditions:', function() {
  this.timeout(120000);
  this.ctx.authenticationNeeded = true;

  describe('with a Notified message that has promo_text', function() {
    beforeEach(async () => {
      await loadAddonsAndNavigateHome('04_promo_message');
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
  });

  describe('with two messages', function() {
    beforeEach(async () => {
      await loadAddonsAndNavigateHome('07_two_promo_messages');

      await vpn.waitForQuery(
          queries.screenMessageAddonPopover.POPOVER.visible());
      const raw = await vpn.getSetting('addonPromoLastShown');
      const lastShown = new Date(raw);
      const ageMs = Date.now() - lastShown.getTime();
      assert(
          ageMs >= 0 && ageMs < 3000,
          `addonPromoLastShown should be within last 3s, but was ${
              ageMs}ms ago`);
    })

    it('does not show 2nd popover within the normal cooldown period',
       async () => {
         // go to messages screen (dismisses the addon), go back to home screen
         await vpn.waitForQueryAndClick(queries.navBar.MESSAGES.visible());
         await vpn.waitForQuery(queries.screenMessaging.SCREEN.visible());
         await vpn.waitForQueryAndClick(queries.navBar.HOME.visible());
         await vpn.waitForQuery(queries.screenHome.SCREEN.visible());
         await vpn.waitForQuery(
             queries.screenMessageAddonPopover.POPOVER.hidden());
       });

    it('does not show 2nd popover within the short cooldown period',
       async () => {
         await vpn.setSetting('useShortAddonPromoTime', 'true');

         // go to messages screen (dismisses the addon), go back to home screen
         await vpn.waitForQueryAndClick(queries.navBar.MESSAGES.visible());
         await vpn.waitForQuery(queries.screenMessaging.SCREEN.visible());
         await vpn.waitForQueryAndClick(queries.navBar.HOME.visible());
         await vpn.waitForQuery(queries.screenHome.SCREEN.visible());
         await vpn.waitForQuery(
             queries.screenMessageAddonPopover.POPOVER.hidden());
       });

    it('shows 2nd popover after the normal cooldown period', async () => {
      // go to messages screen (dismisses the addon), reset timer, go back to
      // home screen
      await vpn.waitForQueryAndClick(queries.navBar.MESSAGES.visible());
      await vpn.waitForQuery(queries.screenMessaging.SCREEN.visible());
      await setLastAddonPopoverToFourDaysAgo();
      await vpn.waitForQueryAndClick(queries.navBar.HOME.visible());
      await vpn.waitForQuery(queries.screenHome.SCREEN.visible());
      assert.equal(
          await vpn.getQueryProperty(
              queries.screenMessageAddonPopover.POPOVER, 'visible'),
          'true');
    });

    it('shows 2nd popover after the short cooldown period', async () => {
      await vpn.setSetting('useShortAddonPromoTime', 'true');

      // go to messages screen (dismisses the addon), go back to home screen
      await vpn.waitForQueryAndClick(queries.navBar.MESSAGES.visible());
      await vpn.waitForQuery(queries.screenMessaging.SCREEN.visible());

      // update the last time to one that allows it to be shown, as it runs once
      // every 30 seconds
      const fortySecondsAgo = Date.now() - (1000 * 40);
      await vpn.setLastAddonPopover(fortySecondsAgo);

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
      await loadAddonsAndNavigateHome('09_promo_no_notify');
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
      await loadAddonsAndNavigateHome('05_promo_no_promo_text');
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
      await loadAddonsAndNavigateHome('06_promo_empty_text');
    })

    it('does not show popover when promo_text is empty string', async () => {
      assert.equal(
          await vpn.getQueryProperty(
              queries.screenMessageAddonPopover.POPOVER, 'visible'),
          'false');
    });
  });

  describe('UI works as expected', function() {
    beforeEach(async () => {
      await loadAddonsAndNavigateHome('04_promo_message');
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

  describe('UI works as expected with multiple messages', function() {
    beforeEach(async () => {
      await loadAddonsAndNavigateHome('07_two_promo_messages');
    })
    it('when multiple messages, tapping the popover navigates to the messaging screen and highlights correct message',
       async () => {
         await vpn.waitForQueryAndClick(
             queries.screenMessageAddonPopover.POPOVER.visible());
         await vpn.waitForQuery(queries.screenMessaging.SCREEN.visible());
         await vpn.waitForQuery(
             queries.screenMessaging.messageItem('message_promo_a').visible());
         const highlightA = await vpn.getQueryProperty(
             queries.screenMessaging.messageItem('message_promo_a'),
             'highlight');
         const pauseHoverA = await vpn.getQueryProperty(
             queries.screenMessaging.messageItem('message_promo_a'),
             'pauseHover');
         await vpn.waitForQuery(
             queries.screenMessaging.messageItem('message_promo_b').visible());
         const highlightB = await vpn.getQueryProperty(
             queries.screenMessaging.messageItem('message_promo_b'),
             'highlight');
         const pauseHoverB = await vpn.getQueryProperty(
             queries.screenMessaging.messageItem('message_promo_b'),
             'pauseHover');
         assert.equal(highlightA, 'true');
         assert.equal(pauseHoverA, 'false');
         assert.equal(highlightB, 'false');
         assert.equal(pauseHoverB, 'true');
       });
  });
});
