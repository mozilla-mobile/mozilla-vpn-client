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
      let state = await vpn.getConnectionBenchmarkProperty('state');
      return state == 'StateRunning';
    });
    await vpn.waitForCondition(async () => {
      let state = await vpn.getConnectionBenchmarkProperty('state');
      return state != 'StateRunning';
    });

    // We expect the benchmark to succeed.
    await vpn.wait(3000);
    let state = await vpn.getConnectionBenchmarkProperty('state');
    let speed = await vpn.getConnectionBenchmarkProperty('speed');
    let bps = parseInt(await vpn.getConnectionBenchmarkProperty('downloadBps'));
    assert.strictEqual(state, 'StateReady');

    assert.strictEqual(
        speed,
        (bps >= 25000000) ? 'SpeedFast' :
                            ((bps >= 10000000) ? 'SpeedMedium' : 'SpeedSlow'));

    // Exit the benchmark
    await vpn.waitForElement(homeScreen.CONNECTION_INFO_TOGGLE);
    await vpn.clickOnElement(homeScreen.CONNECTION_INFO_TOGGLE);
  });

  it('Failed benchmark on HTTP error', async () => {
    await vpn.waitForElement(generalElements.CONTROLLER_TITLE);
    await vpn.activate(true);

    // Re-Configure the benchmark to use a URL that generates an HTTP error.
    await vpn.setConnectionBenchmarkUrl('http://httpstat.us/500?sleep=2000');

    // Start the connection benchmark and wait for it to finish.
    await vpn.waitForElementAndClick(homeScreen.CONNECTION_INFO_TOGGLE);
    await vpn.waitForCondition(async () => {
      let state = await vpn.getConnectionBenchmarkProperty('state');
      return state == 'StateRunning';
    });
    await vpn.waitForCondition(async () => {
      let state = await vpn.getConnectionBenchmarkProperty('state');
      return state != 'StateRunning';
    });

    // We expect the benchmark to fail.
    await vpn.wait(3000);
    assert.strictEqual(
        await vpn.getConnectionBenchmarkProperty('state'), 'StateError');
    assert.strictEqual(
        await vpn.getElementProperty('connectionInfoError', 'visible'), 'true');

    // Exit the benchmark
    await vpn.waitForElementAndClick(homeScreen.CONNECTION_INFO_TOGGLE);
  });

  it('Retry failed benchmark', async () => {
    await vpn.waitForElement(generalElements.CONTROLLER_TITLE);
    await vpn.activate(true);

    let downloadUrl = await vpn.getConnectionBenchmarkProperty('downloadUrl');

    // Re-Configure the benchmark to use a URL that generates an HTTP error.
    await vpn.setConnectionBenchmarkUrl('http://httpstat.us/404?sleep=2000');

    // Start the connection benchmark and wait for it to finish.    
    await vpn.waitForElementAndClick(homeScreen.CONNECTION_INFO_TOGGLE);
    await vpn.waitForCondition(async () => {
      let state = await vpn.getConnectionBenchmarkProperty('state');
      return state == 'StateRunning';
    });
    await vpn.waitForCondition(async () => {
      let state = await vpn.getConnectionBenchmarkProperty('state');
      return state != 'StateRunning';
    });

    // We expect the benchmark to fail.
    await vpn.wait(3000);
    assert.strictEqual(
        await vpn.getConnectionBenchmarkProperty('state'), 'StateError');
    assert.strictEqual(
        await vpn.getElementProperty('connectionInfoError', 'visible'), 'true');

    // Restore the original download URL and retry the benchmark.
    await vpn.setConnectionBenchmarkUrl(downloadUrl);
    await vpn.waitForElementAndClick(homeScreen.CONNECTION_INFO_RETRY);
    await vpn.waitForCondition(async () => {
      let state = await vpn.getConnectionBenchmarkProperty('state');
      return state == 'StateRunning';
    });
    await vpn.waitForCondition(async () => {
      let state = await vpn.getConnectionBenchmarkProperty('state');
      return state != 'StateRunning';
    });

    // This time we expect the benchmark to succeed.
    await vpn.wait();
    let speed = await vpn.getConnectionBenchmarkProperty('speed');
    let bps = parseInt(await vpn.getConnectionBenchmarkProperty('downloadBps'));
    assert.strictEqual(
        speed,
        (bps >= 25000000) ? 'SpeedFast' :
                            ((bps >= 10000000) ? 'SpeedMedium' : 'SpeedSlow'));

    // Exit the benchmark    
    await vpn.waitForElementAndClick(homeScreen.CONNECTION_INFO_TOGGLE);
  });

  it('Error on unexpected disconnect', async () => {
    await vpn.waitForElement(generalElements.CONTROLLER_TITLE);
    await vpn.activate(true);

    // Re-Configure the benchmark to use a URL that will hang for a while.
    await vpn.setConnectionBenchmarkUrl('http://httpstat.us/204?sleep=10000');

    // Start the connection benchmark.    
    await vpn.waitForElementAndClick(homeScreen.CONNECTION_INFO_TOGGLE);
    await vpn.waitForCondition(async () => {
      let state = await vpn.getConnectionBenchmarkProperty('state');
      return state == 'StateRunning';
    });

    // Disconnect the VPN, this should trigger an error.
    await vpn.wait(1000);
    await vpn.deactivate();
    await vpn.waitForCondition(async () => {
      let state = await vpn.getConnectionBenchmarkProperty('state');
      return state != 'StateRunning';
    });

    // We expect the benchmark to fail.
    await vpn.wait(5000);
    assert.strictEqual(
        await vpn.getConnectionBenchmarkProperty('state'), 'StateError');
    assert.strictEqual(
        await vpn.getElementProperty('connectionInfoError', 'visible'), 'true');

    // Exit the benchmark    
    await vpn.waitForElementAndClick(homeScreen.CONNECTION_INFO_TOGGLE);
  });
});
