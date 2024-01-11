/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { equal } from 'assert';
import { screenPostAuthentication, screenTelemetry } from './queries.js';
import { waitForInitialView, authenticateInApp, clickOnQuery, waitForQuery, getSetting, waitForCondition } from './helper.js';

describe('Telemetry view', function() {
  beforeEach(async () => {
    await waitForInitialView();
  });

  async function _getToTelemetryPage() {
    await authenticateInApp();
    await clickOnQuery(screenPostAuthentication.BUTTON.visible());
    await waitForQuery(screenTelemetry.BUTTON.visible());
    await waitForQuery(screenTelemetry.DECLINE_LINK.visible());
  }

  it('Accept telemetry', async () => {
    equal(await getSetting('telemetryPolicyShown'), false);
    equal(await getSetting('gleanEnabled'), true);
    await _getToTelemetryPage();
    await clickOnQuery(screenTelemetry.BUTTON.visible());

    await waitForCondition(
        async () => await getSetting('telemetryPolicyShown') === true);
    equal(await getSetting('telemetryPolicyShown'), true);
    equal(await getSetting('gleanEnabled'), true);
  });

  it('Deny telemetry', async () => {
    equal(await getSetting('telemetryPolicyShown'), false);
    equal(await getSetting('gleanEnabled'), true);
    await _getToTelemetryPage();
    await clickOnQuery(screenTelemetry.DECLINE_LINK.visible());

    await waitForCondition(
        async () => await getSetting('telemetryPolicyShown') === true);
    equal(await getSetting('telemetryPolicyShown'), true);
    equal(await getSetting('gleanEnabled'), false);
  });
});
