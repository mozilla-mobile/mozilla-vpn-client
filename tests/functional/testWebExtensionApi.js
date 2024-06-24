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
} = require('./utils/webextension.js')

if(!vpn.runningOnWasm()) {

describe('WebExtension API', function() {
  this.timeout(60000);
  this.ctx.authenticationNeeded = true;

  it('A Webextension can query the Status of the VPN', async () => {
    const sock = await connectExtension();
    const messagePipe = getMessageStream(sock);
    sentToClient(new ExtensionMessage('status'), sock);
    await readResponseOfType('status', messagePipe);
    sock.destroy();
  });
  it('A Webextension can activate the VPN', async () => {
    const sock = await connectExtension();
    const messagePipe = getMessageStream(sock);
    sentToClient(new ExtensionMessage('activate'), sock);

    await vpn.waitForCondition(async () => {
      sentToClient(new ExtensionMessage('status'), sock);
      const msg = await readResponseOfType('status', messagePipe);
      return msg.status.vpn === 'StateOn';
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
      return msg.status.vpn === 'StateOn';
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
});

}
