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
      let state = await vpn.getMozillaProperty(
          'Mozilla.VPN', 'VPNConnectionBenchmark', 'state');
      return state == 'StateRunning';
    });
    await vpn.waitForCondition(async () => {
      let state = await vpn.getMozillaProperty(
          'Mozilla.VPN', 'VPNConnectionBenchmark', 'state');
      return state != 'StateRunning';
    });

    // We expect the benchmark to succeed.
    let speed = await vpn.getMozillaProperty(
        'Mozilla.VPN', 'VPNConnectionBenchmark', 'speed');
    let downloadBps = parseInt(await vpn.getMozillaProperty(
        'Mozilla.VPN', 'VPNConnectionBenchmark', 'downloadBps'));
    let uploadBps = parseInt(await vpn.getMozillaProperty(
        'Mozilla.VPN', 'VPNConnectionBenchmark', 'uploadBps'));
    let state = await vpn.getMozillaProperty(
        'Mozilla.VPN', 'VPNConnectionBenchmark', 'state');
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
      let state = await vpn.getMozillaProperty(
          'Mozilla.VPN', 'VPNConnectionBenchmark', 'state');
      return state == 'StateRunning';
    });
    await vpn.waitForCondition(async () => {
      let state = await vpn.getMozillaProperty(
          'Mozilla.VPN', 'VPNConnectionBenchmark', 'state');
      return state != 'StateRunning';
    });

    // We expect the benchmark to fail.
    assert.strictEqual(
        await vpn.getMozillaProperty(
            'Mozilla.VPN', 'VPNConnectionBenchmark', 'state'),
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
      let state = await vpn.getMozillaProperty(
          'Mozilla.VPN', 'VPNConnectionBenchmark', 'state');
      return state == 'StateRunning';
    });
    await vpn.waitForCondition(async () => {
      let state = await vpn.getMozillaProperty(
          'Mozilla.VPN', 'VPNConnectionBenchmark', 'state');
      return state != 'StateRunning';
    });

    // We expect the benchmark to fail.
    assert.strictEqual(
        await vpn.getMozillaProperty(
            'Mozilla.VPN', 'VPNConnectionBenchmark', 'state'),
        'StateError');
    await vpn.waitForQuery(queries.screenHome.CONNECTION_INFO_ERROR.visible());

    // Let's set 200 as status code.
    this.ctx.downloadUrlCallback = async req => {
      this.ctx.networkBenchmarkOverrideEndpoints.GETs['/'].status = 200;
    };

    await vpn.waitForQueryAndClick(queries.screenHome.CONNECTION_INFO_RETRY);
    await vpn.waitForCondition(async () => {
      let state = await vpn.getMozillaProperty(
          'Mozilla.VPN', 'VPNConnectionBenchmark', 'state');
      return state == 'StateRunning';
    });
    await vpn.waitForCondition(async () => {
      let state = await vpn.getMozillaProperty(
          'Mozilla.VPN', 'VPNConnectionBenchmark', 'state');
      return state != 'StateRunning';
    });

    // This time we expect the benchmark to succeed.
    let speed = await vpn.getMozillaProperty(
        'Mozilla.VPN', 'VPNConnectionBenchmark', 'speed');
    let downloadBps = parseInt(await vpn.getMozillaProperty(
        'Mozilla.VPN', 'VPNConnectionBenchmark', 'downloadBps'));
    let uploadBps = parseInt(await vpn.getMozillaProperty(
        'Mozilla.VPN', 'VPNConnectionBenchmark', 'uploadBps'));
    let state = await vpn.getMozillaProperty(
        'Mozilla.VPN', 'VPNConnectionBenchmark', 'state');
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
      let state = await vpn.getMozillaProperty(
          'Mozilla.VPN', 'VPNConnectionBenchmark', 'state');
      return state == 'StateRunning';
    });

    // Disconnect the VPN, this should trigger an error.
    await vpn.deactivate();
    await vpn.waitForCondition(async () => {
      let state = await vpn.getMozillaProperty(
          'Mozilla.VPN', 'VPNConnectionBenchmark', 'state');
      return state != 'StateRunning';
    });

    // We expect the benchmark to fail.
    assert.strictEqual(
        await vpn.getMozillaProperty(
            'Mozilla.VPN', 'VPNConnectionBenchmark', 'state'),
        'StateError');
    await vpn.waitForQuery(queries.screenHome.CONNECTION_INFO_ERROR.visible());

    // Exit the benchmark
    await vpn.waitForQueryAndClick(queries.screenHome.CONNECTION_INFO_TOGGLE);
  });

  it('Error when starting with no connection', async () => {
    await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
    await vpn.activate(true);

    // Force a No Signal situation
    await vpn.forceConnectionStabilityStatus('nosignal');

    // Start the connection benchmark; it goes into Running state before Error.
    await vpn.waitForQueryAndClick(queries.screenHome.CONNECTION_INFO_TOGGLE);

    // We expect the benchmark to fail.
    assert.strictEqual(
        await vpn.getMozillaProperty(
            'Mozilla.VPN', 'VPNConnectionBenchmark', 'state'),
        'StateError');
    await vpn.waitForQuery(queries.screenHome.CONNECTION_INFO_ERROR.visible());

    // Exit the benchmark
    await vpn.waitForQueryAndClick(queries.screenHome.CONNECTION_INFO_TOGGLE);
  });

  describe('speed tests related telemetry tests', () => {
    if(vpn.runningOnWasm()) {
      // No Glean on WASM.
      return;
    }

    // Telemetry design is detailed at:
    // https://miro.com/app/board/uXjVM_QZzjA=/?share_link_id=616399368132

    it("records events on successful speed test", async () => {
      this.ctx.downloadUrlCallback = async () => {};

      // Activate the VPN, otherwise the speed test feature is disabled
      await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
      await vpn.activate(true);

      const [ { extra: appStepExtras } ] = await vpn.gleanTestGetValue("impression", "mainScreen", "main");
      assert.strictEqual("main", appStepExtras.screen);

      // Start the connection benchmark.
      await vpn.waitForQueryAndClick(
          queries.screenHome.CONNECTION_INFO_TOGGLE.visible());

      const triggeredEventsList = await vpn.gleanTestGetValue("interaction", "startSpeedTestSelected", "main")
      assert.strictEqual(triggeredEventsList.length, 1);
      const triggeredEventExtra = triggeredEventsList[0].extra;
      assert.strictEqual("main", triggeredEventExtra.screen);

      // Connection benchmark results loaded succesfully
      let completedEventsList;
      await vpn.waitForCondition(async () => {
        completedEventsList = await vpn.gleanTestGetValue("impression", "speedTestCompletedScreen", "main")
        return completedEventsList.length == 1;
      });

      let completedEventExtra = completedEventsList[0].extra;
      assert.strictEqual("speed_test_completed", completedEventExtra.screen);

      const outcomeEventsList = await vpn.gleanTestGetValue("outcome", "speedTestCompleted", "main")
      assert.strictEqual(outcomeEventsList.length, 1);
      const outcomeEventExtra = outcomeEventsList[0].extra;
      assert.strictEqual("speed_test_completed", outcomeEventExtra.outcome);
      assert.strictEqual(await vpn.getMozillaProperty('Mozilla.VPN', 'VPNConnectionBenchmark', 'speed'), outcomeEventExtra.speed);

      // Refresh the test
      await vpn.waitForQueryAndClick(
        queries.screenHome.CONNECTION_INFO_RESTART.visible());

      const refreshEventsList = await vpn.gleanTestGetValue("interaction", "refreshSelected", "main")
      assert.strictEqual(refreshEventsList.length, 1);
      const refreshEventExtra = refreshEventsList[0].extra;
      assert.strictEqual("speed_test_completed", refreshEventExtra.screen);

      // Connection benchmark results loaded succesfully again
      await vpn.waitForCondition(async () => {
        completedEventsList = await vpn.gleanTestGetValue("impression", "speedTestCompletedScreen", "main")
        // Now the list has two events, the first one and the new one
        return completedEventsList.length == 2;
      });

      completedEventExtra = completedEventsList[1].extra;
      assert.strictEqual("speed_test_completed", completedEventExtra.screen);

      // Close the connection benchmark
      await vpn.waitForQueryAndClick(
        queries.screenHome.CONNECTION_INFO_TOGGLE.visible());

      const closedEventsList = await vpn.gleanTestGetValue("interaction", "closeSelected", "main")
      assert.strictEqual(closedEventsList.length, 1);
      const closedEventExtra = closedEventsList[0].extra;
      assert.strictEqual("speed_test_completed", closedEventExtra.screen);
    });

    it("records events while loading speed test", async () => {
      let resolveSpeedTestResults;
      this.ctx.downloadUrlCallback = async () => {
        // Hold the speed test in the loading state until we are done testing the loading state.
        await new Promise(resolve => resolveSpeedTestResults = resolve);
      };

      // Activate the VPN, otherwise the speed test feature is disabled
      await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
      await vpn.activate(true);

      // Start the connection benchmark.
      await vpn.waitForQueryAndClick(
          queries.screenHome.CONNECTION_INFO_TOGGLE.visible());

      // Connection benchmark results are loading
      let loadingEventsList;
      await vpn.waitForCondition(async () => {
        loadingEventsList = await vpn.gleanTestGetValue("impression", "speedTestLoadingScreen", "main")
        return loadingEventsList.length == 1;
      });

      let loadingEventExtra = loadingEventsList[0].extra;
      assert.strictEqual("speed_test_loading", loadingEventExtra.screen);

      // Close the connection benchmark while loading
      await vpn.waitForQueryAndClick(
        queries.screenHome.CONNECTION_INFO_TOGGLE.visible());

      const closedEventsList = await vpn.gleanTestGetValue("interaction", "closeSelected", "main")
      assert.strictEqual(closedEventsList.length, 1);
      const closedEventExtra = closedEventsList[0].extra;
      assert.strictEqual("speed_test_loading", closedEventExtra.screen);

      // Resolve the loading speed test, just in case.
      resolveSpeedTestResults();
    });

    it("records events on errored speed test", async () => {
      this.ctx.networkBenchmarkOverrideEndpoints.GETs['/'].status = 400;
      this.ctx.downloadUrlCallback = async () => {};

      // Activate the VPN, otherwise the speed test feature is disabled
      await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
      await vpn.activate(true);

      const [ { extra: appStepExtras } ] = await vpn.gleanTestGetValue("impression", "mainScreen", "main")
      assert.strictEqual("main", appStepExtras.screen);

      // Start the connection benchmark.
      await vpn.waitForQueryAndClick(
          queries.screenHome.CONNECTION_INFO_TOGGLE.visible());

      const triggeredEventsList = await vpn.gleanTestGetValue("interaction", "startSpeedTestSelected", "main")
      assert.strictEqual(triggeredEventsList.length, 1);
      const triggeredEventExtra = triggeredEventsList[0].extra;
      assert.strictEqual("main", triggeredEventExtra.screen);

      // Connection benchmark results errorred
      let erroredEventsList;
      await vpn.waitForCondition(async () => {
        erroredEventsList = await vpn.gleanTestGetValue("impression", "speedTestErrorScreen", "main")
        return erroredEventsList.length == 1;
      });

      let completedEventExtra = erroredEventsList[0].extra;
      assert.strictEqual("speed_test_error", completedEventExtra.screen);

      // Refresh the test
      await vpn.waitForQueryAndClick(
        queries.screenHome.CONNECTION_INFO_RESTART.visible());

      const refreshEventsList = await vpn.gleanTestGetValue("interaction", "refreshSelected", "main")
      assert.strictEqual(refreshEventsList.length, 1);
      const refreshEventExtra = refreshEventsList[0].extra;
      assert.strictEqual("speed_test_error", refreshEventExtra.screen);

      // Connection benchmark results errored again
      await vpn.waitForCondition(async () => {
        erroredEventsList = await vpn.gleanTestGetValue("impression", "speedTestErrorScreen", "main")
        // Now the list has two events, the first one and the new one
        return erroredEventsList.length == 2;
      });

      completedEventExtra = erroredEventsList[1].extra;
      assert.strictEqual("speed_test_error", completedEventExtra.screen);

      // Refresh the test, but this time through the "Try Again" button
      await vpn.waitForQueryAndClick(
        queries.screenHome.CONNECTION_INFO_RETRY.visible());

      const tryAgainEventsList = await vpn.gleanTestGetValue("interaction", "refreshSelected", "main")
      // Execting two events, since this is the same as the refreshEvent
      assert.strictEqual(tryAgainEventsList.length, 2);
      const tryAgainEventExtra = tryAgainEventsList[1].extra;
      assert.strictEqual("speed_test_error", tryAgainEventExtra.screen);

      // Connection benchmark results errored again
      await vpn.waitForCondition(async () => {
        erroredEventsList = await vpn.gleanTestGetValue("impression", "speedTestErrorScreen", "main")
        // Now the list has three events, the first one and the new one
        return erroredEventsList.length == 3;
      });

      completedEventExtra = erroredEventsList[2].extra;
      assert.strictEqual("speed_test_error", completedEventExtra.screen);

      // Close the connection benchmark
      await vpn.waitForQueryAndClick(
        queries.screenHome.CONNECTION_INFO_TOGGLE.visible());

      const closedEventsList = await vpn.gleanTestGetValue("interaction", "closeSelected", "main")
      assert.strictEqual(closedEventsList.length, 1);
      const closedEventExtra = closedEventsList[0].extra;
      assert.strictEqual("speed_test_error", closedEventExtra.screen);
    });
  });
});
