/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const vpn = require('./helper.js');
const queries = require('./queries.js');

const {
  sentToClient,
  connectExtension,
  getMessageStream,
  ExtensionMessage,
  readResponseOfType
} = require('./utils/webextension.js');


if(!vpn.runningOnWasm()) {

describe('WebExtension API', function() {
  this.timeout(60000);
  this.ctx.authenticationNeeded = true;

  beforeEach(async () => {
    await vpn.flipFeatureOn('webExtension');
  });

  it('A Webextension can query the Status of the VPN', async () => {
    const sock = await connectExtension();
    const messagePipe = getMessageStream(sock);
    const statusPromise = readResponseOfType('status', messagePipe);
    sentToClient(new ExtensionMessage('status'), sock);
    const msg = await statusPromise
    assert(msg.status.version, `A Version is sent in msg: ${JSON.stringify(msg)}` )
    assert(msg.status.connectionHealth, `The current Connection Health status is sent in msg: ${JSON.stringify(msg)}` )
    sock.destroy();
  });
  it('A Webextension can activate the VPN', async () => {
    const sock = await connectExtension();
    const messagePipe = getMessageStream(sock);
    sentToClient(new ExtensionMessage('activate'), sock);

    await vpn.waitForCondition(async () => {
      sentToClient(new ExtensionMessage('status'), sock);
      const msg = await readResponseOfType('status', messagePipe);
      return msg.status.vpn === 'StateOnPartial';
    });
    const currentstate =
        await vpn.getMozillaProperty('Mozilla.VPN', 'VPNController', 'state');
    assert(currentstate === 'StateOnPartial')
    sock.destroy();
  });
  it('A Webextension can deactivate the VPN if it Self Activated', async () => {
    const sock = await connectExtension();
    const messagePipe = getMessageStream(sock);
    sentToClient(new ExtensionMessage('activate'), sock);
    await readResponseOfType('activate', messagePipe);
    await vpn.waitForCondition(async () => {
      sentToClient(new ExtensionMessage('status'), sock);
      const msg = await readResponseOfType('status', messagePipe);
      return msg.status.vpn === 'StateOnPartial';
    });
    sentToClient(new ExtensionMessage('deactivate'), sock);
    await vpn.waitForCondition(async () => {
      sentToClient(new ExtensionMessage('status'), sock);
      const msg = await readResponseOfType('status', messagePipe);
      return msg.status.vpn === 'StateOff';
    });
    sock.destroy();
  });

  it('A Webextension can NOT deactivate the VPN if it was activated in Client',
     async () => {
       /**
        * We currently do not want to allow the extension to Disable the
        * Connection unless the extension was the reason for the current
        * connection
        */
       await vpn.activate();
       const sock = await connectExtension();
       const messagePipe = getMessageStream(sock);

       await vpn.waitForCondition(async () => {
         sentToClient(new ExtensionMessage('status'), sock);
         const msg = await readResponseOfType('status', messagePipe);
         return msg.status.vpn === 'StateOn';
       }, 500, 'VPN should report StateOn');
       sentToClient(new ExtensionMessage('deactivate'), sock);
       await vpn.wait(200);

       await vpn.waitForCondition(async () => {
         sentToClient(new ExtensionMessage('status'), sock);
         const msg = await readResponseOfType('status', messagePipe);
         return msg.status.vpn === 'StateOn';
       }, 500, 'VPN should still report StateOn');
       sock.destroy();
     });

  it('A Webextension can get a Subset of the Featurelist', async () => {
    const sock = await connectExtension();
    const messagePipe = getMessageStream(sock);

    const response = readResponseOfType('featurelist', messagePipe);
    sentToClient(new ExtensionMessage('featurelist'), sock);
    const msg = await response;
    assert(msg.featurelist.webExtension === true);
    assert(msg.featurelist.hasOwnProperty('localProxy'));

    sock.destroy();
  });
  it('A Webextension will be notified if the stability becomes instable ', async () => {
    const sock = await connectExtension();
    const messagePipe = getMessageStream(sock);
    const statusPromise = readResponseOfType('status', messagePipe);
    await vpn.forceConnectionStabilityStatus("unstable");
    const msg = await statusPromise
    assert(msg.status.connectionHealth == "Unstable", "The extension was notified of the instability: "+ msg.status.connectionHealth)
    sock.destroy();
  });
});

}
