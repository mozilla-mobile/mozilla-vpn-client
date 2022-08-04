/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const vpn = require('./helper.js');
const guardian = require('./guardian.js');
const assert = require('node:assert/strict');

describe('PushMessage.handlers', function() {
  describe('DEVICE_DELETED', function() {
    const messageType = 'DEVICE_DELETED';
    this.ctx.authenticationNeeded = true;

    this.beforeEach(async () => {
      // Override the guardian endpoints to return more than one device.
      const UserData = {
        avatar: '',
        display_name: 'Test test',
        email: 'test@mozilla.com',
        max_devices: 5,
        subscriptions: {vpn: {active: true}},
        devices: [
          {
            name: 'currentDevice',
            unique_id: 'currentDevice',
            pubkey: await vpn.getPublicKey(),
            ipv4_address: '127.0.0.1',
            ipv6_address: '::1',
            created_at: new Date().toISOString()
          },
          {
            name: 'device_2',
            unique_id: 'device_2',
            pubkey: 'publicKey_2',
            ipv4_address: '127.0.0.1',
            ipv6_address: '::1',
            created_at: new Date().toISOString()
          },
          {
            name: 'device_3',
            unique_id: 'device_3',
            pubkey: 'publicKey_3',
            ipv4_address: '127.0.0.1',
            ipv6_address: '::1',
            created_at: new Date().toISOString()
          },
          {
            name: 'device_4',
            unique_id: 'device_4',
            pubkey: 'publicKey_4',
            ipv4_address: '127.0.0.1',
            ipv6_address: '::1',
            created_at: new Date().toISOString()
          },
          {
            name: 'device_5',
            unique_id: 'device_5',
            pubkey: 'publicKey_5',
            ipv4_address: '127.0.0.1',
            ipv6_address: '::1',
            created_at: new Date().toISOString()
          }
        ],
      };

      this.ctx.guardianOverrideEndpoints = {
        GETs: {
          '/api/v1/vpn/account': {status: 200, body: UserData},
        },
      };
    });

    it('no-op', () => {
      // We need this test case here for due to the following situation:
      //
      // We need the guardian overrides to be inside a `beforeEach` block,
      // otherwise we cannot use `await` to wait for the publicKey function to complete.
      //
      // That means it runs _after_ the VPN is initialized for the first time
      // which means the overrides are only effective after a test case has already run.
    })

    it('attempting to delete the current device leads to a logout', async () => {
      // Wait for viewMain to load, then wait a bit more for good measure.
      await vpn.waitForElement("viewMainFlickable");
      await vpn.wait();

      const devices = await vpn.getDevices();
      guardian.broadcastMessage(JSON.stringify({
        type: messageType,
        payload: {
          publicKey: devices.find(device => device.currentDevice).publicKey,
        }
      }));

      await vpn.waitForElementProperty('VPN', 'userState', 'UserNotAuthenticated');
    });

    it('attempting to delete a third device updates the device list', async () => {
      // Wait for viewMain to load, then wait a bit more for good measure.
      await vpn.waitForElement("viewMainFlickable");
      await vpn.wait();

      let devices = await vpn.getDevices();
      // Get a device from the list of devices that is not the current one.
      let otherDevice = devices.find(device => !device.currentDevice);
      // Delete each one of the devices that are not the current one,
      // until only the current device is left.
      while(otherDevice) {
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
      // Wait for viewMain to load, then wait a bit more for good measure.
      await vpn.waitForElement("viewMainFlickable");
      await vpn.wait();

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
