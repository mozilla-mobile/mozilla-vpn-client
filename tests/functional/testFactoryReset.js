/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const queries = require('./queries.js');
const vpn = require('./helper.js');
const setup = require('./setupVpn.js');

describe('Factory Reset', function() {
  this.timeout(120000);
  this.ctx.authenticationNeeded = true;

  beforeEach(async () => {
    if (!(await vpn.isFeatureFlippedOn('factoryReset'))) {
      await vpn.flipFeatureOn('factoryReset');
    }

    await vpn.waitForQueryAndClick(queries.navBar.SETTINGS.visible());
  	await vpn.waitForQueryAndClick(queries.screenSettings.GET_HELP.visible());
  	await vpn.waitForQueryAndClick(queries.screenGetHelp.STACKVIEW.ready());
  	await vpn.waitForQueryAndClick(queries.screenGetHelp.RESET.visible());
  	await vpn.waitForQueryAndClick(queries.screenGetHelp.STACKVIEW.ready());
  });


  //Tests all the navigation for the reset settings view and confirmation modal
  it('setting navigation', async () => {
  	await vpn.waitForQueryAndClick(queries.screenGetHelp.STACKVIEW.ready());
  	await vpn.waitForQueryAndClick(queries.screenGetHelp.BACK_BUTTON.visible());

  	await vpn.waitForQueryAndClick(queries.screenGetHelp.STACKVIEW.ready());
  	await vpn.waitForQueryAndClick(queries.screenGetHelp.RESET.visible());

  	await vpn.waitForQueryAndClick(queries.screenGetHelp.STACKVIEW.ready());
	await vpn.scrollToQuery(queries.screenGetHelp.resetView.VIEW,
		queries.screenGetHelp.resetView.BACK_BUTTON);
  	await vpn.waitForQueryAndClick(queries.screenGetHelp.resetView.BACK_BUTTON.visible());

  	await vpn.waitForQueryAndClick(queries.screenGetHelp.STACKVIEW.ready());
  	await vpn.waitForQueryAndClick(queries.screenGetHelp.RESET.visible());

  	await vpn.waitForQueryAndClick(queries.screenGetHelp.STACKVIEW.ready());
  	await vpn.waitForQueryAndClick(queries.screenGetHelp.resetView.RESET_BUTTON.visible());

  	await vpn.waitForQuery(queries.screenGetHelp.resetView.POPUP_LOADER.prop('active', true));
  	await vpn.waitForQueryAndClick(queries.screenGetHelp.resetView.POPUP_CLOSE_BUTTON.visible());

  	await vpn.waitForQuery(queries.screenGetHelp.resetView.POPUP_LOADER.prop('active', false));
  	await vpn.waitForQueryAndClick(queries.screenGetHelp.resetView.RESET_BUTTON.visible());

  	await vpn.waitForQuery(queries.screenGetHelp.resetView.POPUP_LOADER.prop('active', true));
  	await vpn.waitForQueryAndClick(queries.screenGetHelp.resetView.CANCEL_BUTTON.visible());
  });

  it('reset app back to factory settings', async () => {
  	// WASM fails at relaunching the app, so skip this test on WASM
    if (this.ctx.wasm) {
      return;
    }

  	//Set settings that we plan to reset
  	await vpn.setSetting('dnsProviderFlags', 14);
  	assert.equal(await vpn.getSetting('dnsProviderFlags'), 14);

  	await vpn.waitForQueryAndClick(queries.screenGetHelp.resetView.RESET_BUTTON.visible());
  	await vpn.waitForQuery(queries.screenGetHelp.resetView.POPUP_LOADER.prop('active', true));

    // can't use waitForQueryAndClick for final click because it returns an
    // error - as expected, we crashed the app - but that causes test to fail
    await vpn.clickOnQueryAndAcceptAnyResults(queries.screenGetHelp.resetView.CONFIRM_RESET_BUTTON.visible());

    // Confirm the app quit
    assert.equal(setup.vpnIsInactive(), true);

    // relaunch app
	await setup.startAndConnect();

	//Check that we are on viewInitialize and that we are signed out
  	await vpn.waitForQuery(queries.screenInitialize.SCREEN.visible());

  	//Check that the user is logged out
  	await vpn.waitForMozillaProperty('Mozilla.VPN', 'VPN', 'userAuthenticated', 'false');

	//Check that settings were reset to factory
	assert.equal(await vpn.getSetting('dnsProviderFlags'), 0);
  });

});