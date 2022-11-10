/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const { generalElements } = require('./elements.js');
const vpn = require('./helper.js');

describe('Connectivity', function() {
  this.ctx.authenticationNeeded = true;

  it('check the ui', async () => {
    await vpn.waitForElement(generalElements.CONTROLLER_TITLE);
    await vpn.waitForElementProperty(generalElements.CONTROLLER_TITLE, 'visible', 'true');
    assert(
        await vpn.getElementProperty(generalElements.CONTROLLER_TITLE, 'text') ===
        'VPN is off');

    await vpn.waitForElementProperty(generalElements.CONTROLLER_SUBTITLE, 'visible', 'true');
    assert(
        await vpn.getElementProperty(generalElements.CONTROLLER_SUBTITLE, 'text') ===
        'Turn on to protect your privacy');

    await vpn.waitForElementProperty(generalElements.CONTROLLER_TOGGLE, 'visible', 'true');
  });

  it('Connect to VPN', async () => {
    await vpn.waitForElement(generalElements.CONTROLLER_TITLE);

    await vpn.setSetting('connection-change-notification', 'true');
    // TODO: investigate why the click doesn't work on github.
    // await vpn.clickOnElement('controllerToggle');
    await vpn.activate();

    await vpn.waitForCondition(async () => {
      let connectingMsg =
          await vpn.getElementProperty(generalElements.CONTROLLER_TITLE, 'text');
      return connectingMsg === 'Connecting…';
    });

    assert(
        await vpn.getElementProperty(generalElements.CONTROLLER_SUBTITLE, 'text') ===
        'Masking connection and location');

    await vpn.waitForCondition(async () => {
      return await vpn.getElementProperty(generalElements.CONTROLLER_TITLE, 'text') ==
          'VPN is on';
    });

    assert((await vpn.getElementProperty('secureAndPrivateSubtitle', 'text'))
               .startsWith('Secure and private '));

    await vpn.waitForCondition(() => {
      return vpn.lastNotification().title === 'VPN Connected';
    });

    assert(vpn.lastNotification().title === 'VPN Connected');
    assert(vpn.lastNotification().message.startsWith('Connected to '));
  });

  it('Disconnecting and disconnected', async () => {
    await vpn.waitForElement(generalElements.CONTROLLER_TITLE);

    await vpn.setSetting('connection-change-notification', 'true');
    await vpn.activate();
    await vpn.waitForCondition(async () => {
      return await vpn.getElementProperty(generalElements.CONTROLLER_TITLE, 'text') ==
          'VPN is on';
    });
    await vpn.deactivate();

    // No test for disconnecting because often it's too fast to be tracked.

    await vpn.waitForCondition(async () => {
      return await vpn.getElementProperty(generalElements.CONTROLLER_TITLE, 'text') ===
          'VPN is off';
    });

    assert(
        await vpn.getElementProperty(generalElements.CONTROLLER_SUBTITLE, 'text') ===
        'Turn on to protect your privacy');

    await vpn.waitForCondition(() => {
      return vpn.lastNotification().title === 'VPN Disconnected';
    });

    assert(vpn.lastNotification().title === 'VPN Disconnected');
    assert(vpn.lastNotification().message.startsWith('Disconnected from '));
  });
});
