/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const vpn = require('./helper.js');
const guardian = require('./servers/guardian.js');
const assert = require('node:assert/strict');
const queries = require('./queries.js');

describe('PushMessage.handlers', function() {
  describe('DEVICE_DELETED', function() {
    const messageType = 'DEVICE_DELETED';

    const UserData = {
      avatar: '',
      display_name: 'Test',
      email: 'test@mozilla.com',
      max_devices: 5,
      subscriptions: {vpn: {active: true}},
      devices: [
        {
          name: 'device_1',
          unique_id: 'device_1',
          pubkey: 'a',
          ipv4_address: '127.0.0.1',
          ipv6_address: '::1',
          created_at: new Date().toISOString()
        },
        {
          name: 'device_2',
          unique_id: 'device_2',
          pubkey: 'b',
          ipv4_address: '127.0.0.1',
          ipv6_address: '::1',
          created_at: new Date().toISOString()
        },
        {
          name: 'device_3',
          unique_id: 'device_3',
          pubkey: 'c',
          ipv4_address: '127.0.0.1',
          ipv6_address: '::1',
          created_at: new Date().toISOString()
        },
        {
          name: 'device_4',
          unique_id: 'device_4',
          pubkey: 'd',
          ipv4_address: '127.0.0.1',
          ipv6_address: '::1',
          created_at: new Date().toISOString()
        },
      ],
    };

    this.ctx.guardianOverrideEndpoints = {
      GETs: {
        '/api/v1/vpn/account':
            {status: 200, requiredHeaders: ['Authorization'], body: UserData},
      },
      POSTs: {
        '/api/v1/vpn/device': {
          status: 201,
          requiredHeaders: ['Authorization'],
          callback: (req) => {
            UserData.devices[0].name = req.body.name;
            UserData.devices[0].pubkey = req.body.pubkey;
            UserData.devices[0].unique_id = req.body.unique_id;
          },
          body: {}
        },
      }
    };

    beforeEach(async () => {
      await vpn.overrideWebSocketServerUrl(guardian.url.replace("http", "ws"));
    });

    it('attempting to delete the current device leads to a logout',
       async () => {
         await vpn.authenticateInApp(true, true);
         await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());

         const devices = await vpn.getDevices();
         guardian.broadcastMessage(JSON.stringify({
           type: messageType,
           payload: {
             publicKey: devices.find(device => device.currentDevice).publicKey,
           }
         }));

         await vpn.waitForVPNProperty(
             'VPN', 'userState', 'UserNotAuthenticated');
       });

    it('attempting to delete a third device updates the device list',
       async () => {
         await vpn.authenticateInApp(true, true);
         await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());

         let devices = await vpn.getDevices();
         // Get a device from the list of devices that is not the current one.
         let otherDevice = devices.find(device => !device.currentDevice);
         // Delete each one of the devices that are not the current one,
         // until only the current device is left.
         while (otherDevice) {
           // Broadcast a message to delete the device.
           guardian.broadcastMessage(JSON.stringify({
             type: messageType,
             payload: {
               publicKey: otherDevice.publicKey,
             }
           }));

           // Wait for devices list to be updated.
           await vpn.waitForCondition(async () => {
             const updatedDevices = await vpn.getDevices();
             return updatedDevices.length === devices.length - 1;
           });

           devices = await vpn.getDevices();
           // Get another device that is not the current one.
           otherDevice = devices.find(device => !device.currentDevice);
         }
       });

    it('invalid device deletion messages are ignored', async () => {
      await vpn.authenticateInApp(true, true);
      await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());

      const devices = await vpn.getDevices();

      const invalidMessages = [
        // no payload
        { type: messageType, },
        // missing expected key
        { type: messageType, payload: { oops: "WRONG" } },
        // wrong value type
        { type: messageType, payload: { publicKey: 42 } },
        // unknown public key
        { type: messageType, payload: { publicKey: "unknown" } },
      ];

      for (const invalidMessage of invalidMessages) {
        // Broadcast a message to delete an unknown device.
        guardian.broadcastMessage(JSON.stringify(invalidMessage));

        // Give the app some time to process that message.
        await vpn.wait();

        // Check the devices list has not been changed.
        assert.equal(devices.length, (await vpn.getDevices()).length);
      }
    });
  });
});
