/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const queries = require('./queries.js');
const vpn = require('./helper.js');
const {validators} = require('./servers/guardian_endpoints.js');

const SUBSCRIPTION_DETAILS = {
  plan: {amount: 123, currency: 'usd', interval: 'year', interval_count: 1},
  payment: {
    payment_provider: 'stripe',
    payment_type: 'credit',
    last4: '1234',
    exp_month: 12,
    exp_year: 2022,
    brand: 'visa',
  },
  subscription: {
    _subscription_type: 'web',
    created: 1,
    current_period_end: 2,
    cancel_at_period_end: true,
    status: 'active'
  },
};

describe('User authentication in browser', function() {
  this.timeout(60000);

  beforeEach(async () => {
    await vpn.flipFeatureOff('inAppAuthentication');
  })

  describe('Expired subscription', function() {
    this.timeout(30000);

    // There are two constants defined below: userDataActive and
    // userDataInactive which are used to override the Guardian endpoint to mock
    // a subscription expiration by changing the VPN subscription from true to
    // false. Because this override should be happening on the same device, the
    // device is defined here and used later in both userDataActive and
    // userDataInactive. Simply copying the same device definition in both cases
    // will actually result in two distinct objects, causing the test to fail.
    const device = {
      name: 'Current device',
      unique_id: '',
      pubkey: '',
      ipv4_address: '127.0.0.1',
      ipv6_address: '::1',
      created_at: new Date().toISOString()
    };

    const userDataActive = {
      avatar: '',
      display_name: 'Test',
      email: 'test@mozilla.com',
      max_devices: 5,
      subscriptions: {vpn: {active: true}},
      devices: [device],
    };
    const userDataInactive = {
      avatar: '',
      display_name: 'Test test',
      email: 'test@mozilla.com',
      max_devices: 5,
      subscriptions: {vpn: {active: false}},
      devices: [device]
    };

    this.ctx.guardianOverrideEndpoints = {
      GETs: {
        '/api/v1/vpn/account': {
          status: 200,
          requiredHeaders: ['Authorization'],
          body: userDataInactive
        }
      },
      POSTs: {
        '/api/v1/vpn/device': {
          status: 201,
          requiredHeaders: ['Authorization'],
          bodyValidator: validators.guardianDevice,
          callback: (req) => {
            device.name = req.body.name;
            device.pubkey = req.body.pubkey;
            device.unique_id = req.body.unique_id;
          },
          body: {}
        },
      }
    };

    it.only('Purchase subscription and authenticate in browser', async () => {
      await vpn.waitForInitialView();
      await vpn.authenticateInBrowser(false, true, this.ctx.wasm);


      // Verify that user gets the "Subscribe to Mozilla VPN" screen.
      await vpn.waitForQuery(queries.screenSettings.subscriptionView
                                 .SUBSCRIPTION_NEEDED_VIEW.visible());

      // Click on the Subscribe Now button.
      await vpn.waitForQueryAndClick(queries.screenSettings.subscriptionView
                                         .SUBSCRIPTION_NEEDED_BUTTON.visible());

      // Mock in browser sub authentication (code from helper.js)
      // We don't really want to go through the authentication flow because we
      // are mocking everything. So this next chunk of code manually
      // makes a call to the DesktopAuthenticationListener to mock
      // a successful authentication in browser.
      const url = await this.getLastUrl();
      const authListenerPort = (new URL(url)).searchParams.get('port');
      const options = {
        // We hardcode 127.0.0.1 to match listening on QHostAddress:LocalHost
        // and hardcoded in guardian's vpnClientPixelImageAuthUrl
        hostname: '127.0.0.1',
        port: parseInt(authListenerPort, 10),
        path: '/?code=the_code',
        method: 'GET',
      };

      await new Promise(resolve => {
        const req = http.request(options, res => {});
        req.on('close', resolve);
        req.on('error', error => {
          throw new Error(
              `Unable to connect to ${urlObj.hostname} to complete the
                  auth. ${error.name}, ${error.message}, ${error.stack}`);
        });
        req.end();
      });

      // Wait for VPN client screen to move from spinning wheel to next screen
      await this.waitForMozillaProperty(
          'Mozilla.VPN', 'VPN', 'userState', 'UserAuthenticated');
      await this.waitForQuery(
          queries.screenPostAuthentication.BUTTON.visible());
    });
  });

  it('returns to main view on canceling authentication', async () => {
    await vpn.waitForInitialView();

    await vpn.clickOnQuery(queries.screenInitialize.SIGN_UP_BUTTON.visible());

    if (!this.ctx.wasm) {
      await vpn.waitForCondition(async () => {
        const url = await vpn.getLastUrl();
        return url.includes('/api/v2/vpn/login');
      });
    }

    await vpn.clickOnQuery(
        queries.screenInitialize.AUTHENTICATE_VIEW.visible());
    await vpn.waitForQueryAndClick(
        queries.screenAuthenticating.CANCEL_FOOTER_LINK.visible());

    await vpn.waitForQuery(queries.screenInitialize.SIGN_UP_BUTTON.visible());
  });

  it('Completes authentication', async () => {
    await vpn.waitForInitialView();
    await vpn.authenticateInBrowser(true, true, this.ctx.wasm);
  });

  it('Completes authentication after logout', async () => {
    await vpn.authenticateInBrowser(true, true, this.ctx.wasm);
    await vpn.waitForQueryAndClick(queries.navBar.SETTINGS);

    await vpn.waitForQuery(queries.screenSettings.SIGN_OUT);
    await vpn.scrollToQuery(
        queries.screenSettings.SCREEN, queries.screenSettings.SIGN_OUT);
    await vpn.waitForQueryAndClick(queries.screenSettings.SIGN_OUT);
    await vpn.waitForInitialView();

    await vpn.authenticateInBrowser(false, false, this.ctx.wasm);
  });
});
