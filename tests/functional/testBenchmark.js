/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
const vpn = require('./helper.js');
const assert = require('assert');
const { homeScreen, generalElements } = require('./elements.js');

describe('Benchmark', function() {
  this.timeout(120000);
  this.ctx.authenticationNeeded = true;

  it('Successful benchmark', async () => {
    await vpn.waitForElement(generalElements.CONTROLLER_TITLE);
    await vpn.activate(true);

    // Start the connection benchmark.
    await vpn.waitForElementAndClick(homeScreen.CONNECTION_INFO_TOGGLE);
    await vpn.waitForCondition(async () => {
      let state =
          await vpn.getElementProperty(homeScreen.CONNECTION_BENCHMARK, 'state');
      return state == 'StateRunning';
    });
    await vpn.waitForCondition(async () => {
      let state =
          await vpn.getElementProperty(homeScreen.CONNECTION_BENCHMARK, 'state');
      return state != 'StateRunning';
    });

    // We expect the benchmark to succeed.
    let speed =
        await vpn.getElementProperty(homeScreen.CONNECTION_BENCHMARK, 'speed');
    let downloadBps = parseInt(await vpn.getElementProperty(
        homeScreen.CONNECTION_BENCHMARK, 'downloadBps'));
    let uploadBps = parseInt(await vpn.getElementProperty(
        homeScreen.CONNECTION_BENCHMARK, 'uploadBps'));
    let state =
        await vpn.getElementProperty(homeScreen.CONNECTION_BENCHMARK, 'state');
    assert.strictEqual(state, 'StateReady');

    assert.strictEqual(
        speed,
        (downloadBps >= 25000000 && uploadBps > 0) ?
            'SpeedFast' :
            ((downloadBps >= 10000000 && uploadBps > 0) ? 'SpeedMedium' :
                                                          'SpeedSlow'));

    // Exit the benchmark
    await vpn.waitForElement(homeScreen.CONNECTION_INFO_TOGGLE);
    await vpn.clickOnElement(homeScreen.CONNECTION_INFO_TOGGLE);
  });

  it('Failed benchmark on HTTP error', async () => {
    await vpn.waitForElement(generalElements.CONTROLLER_TITLE);
    await vpn.activate(true);

    // Re-Configure the benchmark to use a URL that generates an HTTP error.
    await vpn.setElementProperty(
        homeScreen.CONNECTION_BENCHMARK, 'downloadUrl', 's',
        'http://httpstat.us/500?sleep=2000');

    // Start the connection benchmark and wait for it to finish.
    await vpn.waitForElementAndClick(homeScreen.CONNECTION_INFO_TOGGLE);
    await vpn.waitForCondition(async () => {
      let state =
          await vpn.getElementProperty(homeScreen.CONNECTION_BENCHMARK, 'state');
      return state == 'StateRunning';
    });
    await vpn.waitForCondition(async () => {
      let state =
          await vpn.getElementProperty(homeScreen.CONNECTION_BENCHMARK, 'state');
      return state != 'StateRunning';
    });

    // We expect the benchmark to fail.
    assert.strictEqual(
        await vpn.getElementProperty(homeScreen.CONNECTION_BENCHMARK, 'state'),
        'StateError');
    assert.strictEqual(
        await vpn.getElementProperty('connectionInfoError', 'visible'), 'true');

    // Exit the benchmark
    await vpn.waitForElementAndClick(homeScreen.CONNECTION_INFO_TOGGLE);
  });

  it('Retry failed benchmark', async () => {
    await vpn.waitForElement(generalElements.CONTROLLER_TITLE);
    await vpn.activate(true);
    let downloadUrl = await vpn.getElementProperty(
        homeScreen.CONNECTION_BENCHMARK, 'downloadUrl');

    // Re-Configure the benchmark to use a URL that generates an HTTP error.
    await vpn.setElementProperty(
        homeScreen.CONNECTION_BENCHMARK, 'downloadUrl', 's',
        'http://httpstat.us/404?sleep=2000');

    // Start the connection benchmark and wait for it to finish.    
    await vpn.waitForElementAndClick(homeScreen.CONNECTION_INFO_TOGGLE);
    await vpn.waitForCondition(async () => {
      let state =
          await vpn.getElementProperty(homeScreen.CONNECTION_BENCHMARK, 'state');
      return state == 'StateRunning';
    });
    await vpn.waitForCondition(async () => {
      let state =
          await vpn.getElementProperty(homeScreen.CONNECTION_BENCHMARK, 'state');
      return state != 'StateRunning';
    });

    // We expect the benchmark to fail.
    assert.strictEqual(
        await vpn.getElementProperty(homeScreen.CONNECTION_BENCHMARK, 'state'),
        'StateError');
    assert.strictEqual(
        await vpn.getElementProperty('connectionInfoError', 'visible'), 'true');

    // Restore the original download URL and retry the benchmark.
    await vpn.setElementProperty(
        homeScreen.CONNECTION_BENCHMARK, 'downloadUrl', 's', downloadUrl);
    await vpn.waitForElementAndClick(homeScreen.CONNECTION_INFO_RETRY);
    await vpn.waitForCondition(async () => {
      let state =
          await vpn.getElementProperty(homeScreen.CONNECTION_BENCHMARK, 'state');
      return state == 'StateRunning';
    });
    await vpn.waitForCondition(async () => {
      let state =
          await vpn.getElementProperty(homeScreen.CONNECTION_BENCHMARK, 'state');
      return state != 'StateRunning';
    });

    // This time we expect the benchmark to succeed.
    let speed =
        await vpn.getElementProperty(homeScreen.CONNECTION_BENCHMARK, 'speed');
    let downloadBps = parseInt(await vpn.getElementProperty(
        homeScreen.CONNECTION_BENCHMARK, 'downloadBps'));
    let uploadBps = parseInt(await vpn.getElementProperty(
        homeScreen.CONNECTION_BENCHMARK, 'uploadBps'));
    let state =
        await vpn.getElementProperty(homeScreen.CONNECTION_BENCHMARK, 'state');
    assert.strictEqual(state, 'StateReady');

    assert.strictEqual(
        speed,
        (downloadBps >= 25000000 && uploadBps > 0) ?
            'SpeedFast' :
            ((downloadBps >= 10000000 && uploadBps > 0) ? 'SpeedMedium' :
                                                          'SpeedSlow'));

    // Exit the benchmark
    await vpn.waitForElementAndClick(homeScreen.CONNECTION_INFO_TOGGLE);
  });

  it('Error on unexpected disconnect', async () => {
    await vpn.waitForElement(generalElements.CONTROLLER_TITLE);
    await vpn.activate(true);

    // Re-Configure the benchmark to use a URL that will hang for a while.
    await vpn.setElementProperty(
        homeScreen.CONNECTION_BENCHMARK, 'downloadUrl', 's',
        'http://httpstat.us/204?sleep=10000');

    // Start the connection benchmark.
    await vpn.waitForElementAndClick(homeScreen.CONNECTION_INFO_TOGGLE);
    await vpn.waitForCondition(async () => {
      let state =
          await vpn.getElementProperty(homeScreen.CONNECTION_BENCHMARK, 'state');
      return state == 'StateRunning';
    });

    // Disconnect the VPN, this should trigger an error.
    await vpn.deactivate();
    await vpn.waitForCondition(async () => {
      let state =
          await vpn.getElementProperty(homeScreen.CONNECTION_BENCHMARK, 'state');
      return state != 'StateRunning';
    });

    // We expect the benchmark to fail.
    assert.strictEqual(
        await vpn.getElementProperty(homeScreen.CONNECTION_BENCHMARK, 'state'),
        'StateError');
    await vpn.waitForCondition(async () => {
      return await vpn.getElementProperty('connectionInfoError', 'visible');
    });

    // Exit the benchmark    
    await vpn.waitForElementAndClick(homeScreen.CONNECTION_INFO_TOGGLE);
  });
});
