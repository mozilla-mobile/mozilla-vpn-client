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

    it("records events on succesfull speed test", async () => {
      this.ctx.downloadUrlCallback = async () => {};

      // Activate the VPN, otherwise the speed test feature is disabled
      await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
      await vpn.activate(true);

      const [ _, { extra: appStepExtras } ] = await vpn.gleanTestGetValue("sample", "appStep", "main");
      assert.strictEqual("main", appStepExtras.screen);
      assert.strictEqual("impression", appStepExtras.action);

      // Start the connection benchmark.
      await vpn.waitForQueryAndClick(
          queries.screenHome.CONNECTION_INFO_TOGGLE.visible());

      const triggeredEventsList = await vpn.gleanTestGetValue("interaction", "speedTestTriggered", "main")
      assert.strictEqual(triggeredEventsList.length, 1);
      const triggeredEventExtra = triggeredEventsList[0].extra;
      assert.strictEqual("main", triggeredEventExtra.screen);
      assert.strictEqual("select", triggeredEventExtra.action);
      assert.strictEqual("speed_test", triggeredEventExtra.element_id);

      // Connection benchmark results loaded succesfully
      let completedEventsList;
      await vpn.waitForCondition(async () => {
        completedEventsList = await vpn.gleanTestGetValue("impression", "speedTestResultCompleted", "main")
        return completedEventsList.length == 1;
      });

      let completedEventExtra = completedEventsList[0].extra;
      assert.strictEqual("speed_test_result", completedEventExtra.screen);
      assert.strictEqual("impression", completedEventExtra.action);

      const outcomeEventsList = await vpn.gleanTestGetValue("outcome", "speedTestCompleted", "main")
      assert.strictEqual(outcomeEventsList.length, 1);
      const outcomeEventExtra = outcomeEventsList[0].extra;
      assert.strictEqual("speed_test_completed", outcomeEventExtra.outcome);
      assert.strictEqual(await vpn.getMozillaProperty('Mozilla.VPN', 'VPNConnectionBenchmark', 'speed'), outcomeEventExtra.speed);

      // Refresh the test
      await vpn.waitForQueryAndClick(
        queries.screenHome.CONNECTION_INFO_RESTART.visible());

      const refreshEventsList = await vpn.gleanTestGetValue("interaction", "speedTestRefresh", "main")
      assert.strictEqual(refreshEventsList.length, 1);
      const refreshEventExtra = refreshEventsList[0].extra;
      assert.strictEqual("speed_test_result", refreshEventExtra.screen);
      assert.strictEqual("select", refreshEventExtra.action);
      assert.strictEqual("refresh", refreshEventExtra.element_id);

      // Connection benchmark results loaded succesfully again
      await vpn.waitForCondition(async () => {
        completedEventsList = await vpn.gleanTestGetValue("impression", "speedTestResultCompleted", "main")
        // Now the list has two events, the first one and the new one
        return completedEventsList.length == 2;
      });

      completedEventExtra = completedEventsList[1].extra;
      assert.strictEqual("speed_test_result", completedEventExtra.screen);
      assert.strictEqual("impression", completedEventExtra.action);

      // Close the connection benchmark
      await vpn.waitForQueryAndClick(
        queries.screenHome.CONNECTION_INFO_TOGGLE.visible());

      const closedEventsList = await vpn.gleanTestGetValue("interaction", "speedTestClosed", "main")
      assert.strictEqual(closedEventsList.length, 1);
      const closedEventExtra = closedEventsList[0].extra;
      assert.strictEqual("speed_test_result", closedEventExtra.screen);
      assert.strictEqual("select", closedEventExtra.action);
      assert.strictEqual("close", closedEventExtra.element_id);
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
        loadingEventsList = await vpn.gleanTestGetValue("impression", "speedTestResultsLoading", "main")
        return loadingEventsList.length == 1;
      });

      let loadingEventExtra = loadingEventsList[0].extra;
      assert.strictEqual("speed_test_loading", loadingEventExtra.screen);
      assert.strictEqual("impression", loadingEventExtra.action);

      // TODO (VPN-2859): Enable this part of the test.
      // Here we are checking if the close event has the expected screen extra key.
      // But that bug flashes the results screen when closed, so the screen extra key
      // on the close event always has the value speed_test_result regardless which
      // screen it was clicked from.
      //
      // Close the connection benchmark while loading
      // await vpn.waitForQueryAndClick(
      //   queries.screenHome.CONNECTION_INFO_TOGGLE.visible());

      // const closedEventsList = await vpn.gleanTestGetValue("interaction", "speedTestClosed", "main")
      // assert.strictEqual(closedEventsList.length, 1);
      // const closedEventExtra = closedEventsList[0].extra;
      // assert.strictEqual("speed_test_loading", closedEventExtra.screen);
      // assert.strictEqual("select", closedEventExtra.action);
      // assert.strictEqual("close", closedEventExtra.element_id);

      // Resolve the loading speed test, just in case.
      resolveSpeedTestResults();
    });

    it("records events on errored speed test", async () => {
      this.ctx.networkBenchmarkOverrideEndpoints.GETs['/'].status = 400;
      this.ctx.downloadUrlCallback = async () => {};

      // Activate the VPN, otherwise the speed test feature is disabled
      await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
      await vpn.activate(true);

      const [ _, { extra: appStepExtras } ] = await vpn.gleanTestGetValue("sample", "appStep", "main")
      assert.strictEqual("main", appStepExtras.screen);
      assert.strictEqual("impression", appStepExtras.action);

      // Start the connection benchmark.
      await vpn.waitForQueryAndClick(
          queries.screenHome.CONNECTION_INFO_TOGGLE.visible());

      const triggeredEventsList = await vpn.gleanTestGetValue("interaction", "speedTestTriggered", "main")
      assert.strictEqual(triggeredEventsList.length, 1);
      const triggeredEventExtra = triggeredEventsList[0].extra;
      assert.strictEqual("main", triggeredEventExtra.screen);
      assert.strictEqual("select", triggeredEventExtra.action);
      assert.strictEqual("speed_test", triggeredEventExtra.element_id);

      // Connection benchmark results errorred
      let erroredEventsList;
      await vpn.waitForCondition(async () => {
        erroredEventsList = await vpn.gleanTestGetValue("impression", "speedTestResultError", "main")
        return erroredEventsList.length == 1;
      });

      let completedEventExtra = erroredEventsList[0].extra;
      assert.strictEqual("speed_test_error", completedEventExtra.screen);
      assert.strictEqual("impression", completedEventExtra.action);

      // Refresh the test
      await vpn.waitForQueryAndClick(
        queries.screenHome.CONNECTION_INFO_RESTART.visible());

      const refreshEventsList = await vpn.gleanTestGetValue("interaction", "speedTestRefresh", "main")
      assert.strictEqual(refreshEventsList.length, 1);
      const refreshEventExtra = refreshEventsList[0].extra;
      assert.strictEqual("speed_test_error", refreshEventExtra.screen);
      assert.strictEqual("select", refreshEventExtra.action);
      assert.strictEqual("refresh", refreshEventExtra.element_id);

      // Connection benchmark results errored again
      await vpn.waitForCondition(async () => {
        erroredEventsList = await vpn.gleanTestGetValue("impression", "speedTestResultError", "main")
        // Now the list has two events, the first one and the new one
        return erroredEventsList.length == 2;
      });

      completedEventExtra = erroredEventsList[1].extra;
      assert.strictEqual("speed_test_error", completedEventExtra.screen);
      assert.strictEqual("impression", completedEventExtra.action);

      // Refresh the test, but this time through the "Try Again" button
      await vpn.waitForQueryAndClick(
        queries.screenHome.CONNECTION_INFO_RETRY.visible());

      const tryAgainEventsList = await vpn.gleanTestGetValue("interaction", "speedTestRefresh", "main")
      // Execting two events, since this is the same as the refreshEvent
      assert.strictEqual(tryAgainEventsList.length, 2);
      const tryAgainEventExtra = tryAgainEventsList[1].extra;
      assert.strictEqual("speed_test_error", tryAgainEventExtra.screen);
      assert.strictEqual("select", tryAgainEventExtra.action);
      assert.strictEqual("try_again", tryAgainEventExtra.element_id);

      // Connection benchmark results errored again
      await vpn.waitForCondition(async () => {
        erroredEventsList = await vpn.gleanTestGetValue("impression", "speedTestResultError", "main")
        // Now the list has three events, the first one and the new one
        return erroredEventsList.length == 3;
      });

      completedEventExtra = erroredEventsList[2].extra;
      assert.strictEqual("speed_test_error", completedEventExtra.screen);
      assert.strictEqual("impression", completedEventExtra.action);

      // TODO (VPN-2859): Enable this part of the test.
      // Here we are checking if the close event has the expected screen extra key.
      // But that bug flashes the results screen when closed, so the screen extra key
      // on the close event always has the value speed_test_result regardless which
      // screen it was clicked from.
      //
      // Close the connection benchmark
      // await vpn.waitForQueryAndClick(
      //   queries.screenHome.CONNECTION_INFO_TOGGLE.visible());

      // const closedEventsList = await vpn.gleanTestGetValue("interaction", "speedTestClosed", "main")
      // assert.strictEqual(closedEventsList.length, 1);
      // const closedEventExtra = closedEventsList[0].extra;
      // assert.strictEqual("speed_test_error", closedEventExtra.screen);
      // assert.strictEqual("select", closedEventExtra.action);
      // assert.strictEqual("close", closedEventExtra.element_id);
    });
  });
});
