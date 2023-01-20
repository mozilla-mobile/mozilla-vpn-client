/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const vpn = require('./helper.js');
const assert = require('assert');
const queries = require('./queries.js');

describe('Benchmark', function() {
  this.timeout(120000);
  this.ctx.authenticationNeeded = true;

  this.ctx.networkBenchmarkOverrideEndpoints = {
    GETs: {
      '/': {
        status: 200,
        bodyRaw: new Array(1024).join('a'),
        callback: async req => await this.ctx.downloadUrlCallback(req)
      },
    },
    POSTs: {},
    DELETEs: {},
  };

  it('Successful benchmark', async () => {
    // For the first tests, we don't need any special responses.
    this.ctx.downloadUrlCallback = req => {};

    await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
    await vpn.activate(true);

    // Start the connection benchmark.
    await vpn.waitForQueryAndClick(
        queries.screenHome.CONNECTION_INFO_TOGGLE.visible());
    await vpn.waitForCondition(async () => {
      let state = await vpn.getVPNProperty('VPNConnectionBenchmark', 'state');
      return state == 'StateRunning';
    });
    await vpn.waitForCondition(async () => {
      let state = await vpn.getVPNProperty('VPNConnectionBenchmark', 'state');
      return state != 'StateRunning';
    });

    // We expect the benchmark to succeed.
    let speed = await vpn.getVPNProperty('VPNConnectionBenchmark', 'speed');
    let downloadBps = parseInt(
        await vpn.getVPNProperty('VPNConnectionBenchmark', 'downloadBps'));
    let uploadBps = parseInt(
        await vpn.getVPNProperty('VPNConnectionBenchmark', 'uploadBps'));
    let state = await vpn.getVPNProperty('VPNConnectionBenchmark', 'state');
    assert.strictEqual(state, 'StateReady');

    assert.strictEqual(
        speed,
        (downloadBps >= 25000000 && uploadBps > 0) ?
            'SpeedFast' :
            ((downloadBps >= 10000000 && uploadBps > 0) ? 'SpeedMedium' :
                                                          'SpeedSlow'));

    // Exit the benchmark
    await vpn.waitForQueryAndClick(queries.screenHome.CONNECTION_INFO_TOGGLE);
  });

  it('Failed benchmark on HTTP error', async () => {
    await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
    await vpn.activate(true);

    // Re-Configure the benchmark to use a URL that generates an HTTP error.
    this.ctx.downloadUrlCallback = async req => {
      this.ctx.networkBenchmarkOverrideEndpoints.GETs['/'].status = 500;
      await new Promise(r => setTimeout(r, 2000));
    };

    // Start the connection benchmark and wait for it to finish.
    await vpn.waitForQueryAndClick(queries.screenHome.CONNECTION_INFO_TOGGLE);
    await vpn.waitForCondition(async () => {
      let state = await vpn.getVPNProperty('VPNConnectionBenchmark', 'state');
      return state == 'StateRunning';
    });
    await vpn.waitForCondition(async () => {
      let state = await vpn.getVPNProperty('VPNConnectionBenchmark', 'state');
      return state != 'StateRunning';
    });

    // We expect the benchmark to fail.
    assert.strictEqual(
        await vpn.getVPNProperty('VPNConnectionBenchmark', 'state'),
        'StateError');
    await vpn.waitForQuery(queries.screenHome.CONNECTION_INFO_ERROR.visible());

    // Exit the benchmark
    await vpn.waitForQueryAndClick(queries.screenHome.CONNECTION_INFO_TOGGLE);
  });

  it('Retry failed benchmark', async () => {
    await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
    await vpn.activate(true);

    // Re-Configure the benchmark to use a URL that generates an HTTP error.
    this.ctx.downloadUrlCallback = async req => {
      this.ctx.networkBenchmarkOverrideEndpoints.GETs['/'].status = 400;
      await new Promise(r => setTimeout(r, 2000));
    };

    // Start the connection benchmark and wait for it to finish.
    await vpn.waitForQueryAndClick(queries.screenHome.CONNECTION_INFO_TOGGLE);
    await vpn.waitForCondition(async () => {
      let state = await vpn.getVPNProperty('VPNConnectionBenchmark', 'state');
      return state == 'StateRunning';
    });
    await vpn.waitForCondition(async () => {
      let state = await vpn.getVPNProperty('VPNConnectionBenchmark', 'state');
      return state != 'StateRunning';
    });

    // We expect the benchmark to fail.
    assert.strictEqual(
        await vpn.getVPNProperty('VPNConnectionBenchmark', 'state'),
        'StateError');
    await vpn.waitForQuery(queries.screenHome.CONNECTION_INFO_ERROR.visible());

    // Let's set 200 as status code.
    this.ctx.downloadUrlCallback = async req => {
      this.ctx.networkBenchmarkOverrideEndpoints.GETs['/'].status = 200;
    };

    await vpn.waitForQueryAndClick(queries.screenHome.CONNECTION_INFO_RETRY);
    await vpn.waitForCondition(async () => {
      let state = await vpn.getVPNProperty('VPNConnectionBenchmark', 'state');
      return state == 'StateRunning';
    });
    await vpn.waitForCondition(async () => {
      let state = await vpn.getVPNProperty('VPNConnectionBenchmark', 'state');
      return state != 'StateRunning';
    });

    // This time we expect the benchmark to succeed.
    let speed = await vpn.getVPNProperty('VPNConnectionBenchmark', 'speed');
    let downloadBps = parseInt(
        await vpn.getVPNProperty('VPNConnectionBenchmark', 'downloadBps'));
    let uploadBps = parseInt(
        await vpn.getVPNProperty('VPNConnectionBenchmark', 'uploadBps'));
    let state = await vpn.getVPNProperty('VPNConnectionBenchmark', 'state');
    assert.strictEqual(state, 'StateReady');

    assert.strictEqual(
        speed,
        (downloadBps >= 25000000 && uploadBps > 0) ?
            'SpeedFast' :
            ((downloadBps >= 10000000 && uploadBps > 0) ? 'SpeedMedium' :
                                                          'SpeedSlow'));

    // Exit the benchmark
    await vpn.waitForQueryAndClick(queries.screenHome.CONNECTION_INFO_TOGGLE);
  });

  it('Error on unexpected disconnect', async () => {
    await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
    await vpn.activate(true);

    // Re-Configure the benchmark to use a URL that will hang for a while.
    this.ctx.downloadUrlCallback = async req => {
      this.ctx.networkBenchmarkOverrideEndpoints.GETs['/'].status = 230;
      await new Promise(r => setTimeout(r, 10000));
    };

    // Start the connection benchmark.
    await vpn.waitForQueryAndClick(queries.screenHome.CONNECTION_INFO_TOGGLE);
    await vpn.waitForCondition(async () => {
      let state = await vpn.getVPNProperty('VPNConnectionBenchmark', 'state');
      return state == 'StateRunning';
    });

    // Disconnect the VPN, this should trigger an error.
    await vpn.deactivate();
    await vpn.waitForCondition(async () => {
      let state = await vpn.getVPNProperty('VPNConnectionBenchmark', 'state');
      return state != 'StateRunning';
    });

    // We expect the benchmark to fail.
    assert.strictEqual(
        await vpn.getVPNProperty('VPNConnectionBenchmark', 'state'),
        'StateError');
    await vpn.waitForQuery(queries.screenHome.CONNECTION_INFO_ERROR.visible());

    // Exit the benchmark
    await vpn.waitForQueryAndClick(queries.screenHome.CONNECTION_INFO_TOGGLE);
  });
});
