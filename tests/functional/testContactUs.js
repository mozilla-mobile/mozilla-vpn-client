/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
const vpn = require('./helper.js');
const assert = require('assert');


describe('Contact us view', function() {
  async function openContactUs() {
    await vpn.wait();
    await vpn.openContactUs();
    await vpn.wait();
    await vpn.waitForElement('contactUs');
    return await vpn.waitForElementProperty('contactUs', 'visible', 'true');
  }

  it('VPNUserInfo is not visible to unathenticated users', async () => {
    await openContactUs();

    await vpn.waitForElement('contactUs-userInfo');
    await vpn.waitForElementProperty('contactUs-userInfo', 'visible', 'false');
    assert(
        await vpn.getElementProperty('contactUs-userInfo', 'visible') ===
        'false');
  });

  it('Email inputs are visible to unauthenticated user', async () => {
    await openContactUs();

    await vpn.waitForElement('contactUs-unauthedUserInputs');
    await vpn.waitForElementProperty(
        'contactUs-unauthedUserInputs', 'visible', 'true');
    assert(
        await vpn.getElementProperty(
            'contactUs-unauthedUserInputs', 'visible') === 'true');
  });

  describe('Contact us view - authenticated user', function() {
    this.ctx.authenticationNeeded = true;

    it('VPNUserInfo visible to authenticated users', async () => {
      await openContactUs();

      await vpn.waitForElement('contactUs-userInfo');
      await vpn.waitForElementProperty('contactUs-userInfo', 'visible', 'true');
      assert(
          await vpn.getElementProperty('contactUs-userInfo', 'visible') ===
          'true');
    });

    it('VPNUserInfo is disabled', async () => {
      await openContactUs();

      await vpn.waitForElement('contactUs-userInfo');
      await vpn.waitForElementProperty(
          'contactUs-userInfo', 'enabled', 'false');
      assert(
          await vpn.getElementProperty('contactUs-userInfo', 'enabled') ===
          'false');
    });

    it('Email inputs are not visible to authenticated user', async () => {
      it('Email inputs are visible to unauthenticated user', async () => {
        await openContactUs();

        await vpn.waitForElement('contactUs-unauthedUserInputs');
        await vpn.waitForElementProperty(
            'contactUs-unauthedUserInputs', 'visible', 'false');
        assert(
            await vpn.getElementProperty(
                'contactUs-unauthedUserInputs', 'visible') === 'false');
      });
    });
  });
});
