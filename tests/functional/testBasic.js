/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const util = require('util');
const vpn = require('./helper.js');

const exec = util.promisify(require('child_process').exec);

describe('Run and quit', function() {
  this.timeout(100000);

  before(async () => await vpn.connect());

  beforeEach(() => {});

  afterEach(() => {});

  after(() => vpn.disconnect());

  it('reset the app', () => vpn.reset());

  it('wait for the main view', async () => {
    // TODO: we should check more elements.
    await vpn.waitForElement('getHelpLink');
    await vpn.waitForElementProperty('getHelpLink', 'visible', 'true');
  });

  it('Open the help menu', async () => {
    await vpn.clickOnElement('getHelpLink');
    await vpn.waitForElementProperty('getHelpLink', 'visible', 'false');

    await vpn.waitForElement('getHelpBack');
    await vpn.waitForElementProperty('getHelpBack', 'visible', 'true');
  });

  it('Go back to the main view', async() => {
    await vpn.clickOnElement('getHelpBack');
    await vpn.waitForElementProperty('getHelpBack', 'visible', 'true');

    await vpn.waitForElement('getHelpLink');
    await vpn.waitForElementProperty('getHelpLink', 'visible', 'true');

    // This is needed just for humans. The UI is already in the other state
    // before completing the animation.
    await vpn.wait();
  });

  it('quit the app', () => vpn.quit());
});
