/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { waitForQuery, activate, waitForQueryAndClick, waitForCondition, getMozillaProperty, deactivate, forceConnectionStabilityStatus, runningOnWasm, gleanTestGetValue } from './helper.js';
import { strictEqual } from 'assert';
import { screenHome } from './queries.js';

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

    await waitForQuery(screenHome.CONTROLLER_TITLE.visible());
    await activate(true);

    // Start the connection benchmark.
    await waitForQueryAndClick(
        screenHome.CONNECTION_INFO_TOGGLE.visible());
    await waitForCondition(async () => {
      let state = await getMozillaProperty(
          'Mozilla.VPN', 'VPNConnectionBenchmark', 'state');
      return state == 'StateRunning';
    });
    await waitForCondition(async () => {
      let state = await getMozillaProperty(
          'Mozilla.VPN', 'VPNConnectionBenchmark', 'state');
      return state != 'StateRunning';
    });

    // We expect the benchmark to succeed.
    let speed = await getMozillaProperty(
        'Mozilla.VPN', 'VPNConnectionBenchmark', 'speed');
    let downloadBps = parseInt(await getMozillaProperty(
        'Mozilla.VPN', 'VPNConnectionBenchmark', 'downloadBps'));
    let uploadBps = parseInt(await getMozillaProperty(
        'Mozilla.VPN', 'VPNConnectionBenchmark', 'uploadBps'));
    let state = await getMozillaProperty(
        'Mozilla.VPN', 'VPNConnectionBenchmark', 'state');
    strictEqual(state, 'StateReady');

    strictEqual(
        speed,
        (downloadBps >= 25000000 && uploadBps > 0) ?
            'SpeedFast' :
            ((downloadBps >= 10000000 && uploadBps > 0) ? 'SpeedMedium' :
                                                          'SpeedSlow'));

    // Exit the benchmark
    await waitForQueryAndClick(screenHome.CONNECTION_INFO_TOGGLE);
  });

  it('Failed benchmark on HTTP error', async () => {
    await waitForQuery(screenHome.CONTROLLER_TITLE.visible());
    await activate(true);

    // Re-Configure the benchmark to use a URL that generates an HTTP error.
    this.ctx.downloadUrlCallback = async req => {
      this.ctx.networkBenchmarkOverrideEndpoints.GETs['/'].status = 500;
      await new Promise(r => setTimeout(r, 2000));
    };

    // Start the connection benchmark and wait for it to finish.
    await waitForQueryAndClick(screenHome.CONNECTION_INFO_TOGGLE);
    await waitForCondition(async () => {
      let state = await getMozillaProperty(
          'Mozilla.VPN', 'VPNConnectionBenchmark', 'state');
      return state == 'StateRunning';
    });
    await waitForCondition(async () => {
      let state = await getMozillaProperty(
          'Mozilla.VPN', 'VPNConnectionBenchmark', 'state');
      return state != 'StateRunning';
    });

    // We expect the benchmark to fail.
    strictEqual(
        await getMozillaProperty(
            'Mozilla.VPN', 'VPNConnectionBenchmark', 'state'),
        'StateError');
    await waitForQuery(screenHome.CONNECTION_INFO_ERROR.visible());

    // Exit the benchmark
    await waitForQueryAndClick(screenHome.CONNECTION_INFO_TOGGLE);
  });

  it('Retry failed benchmark', async () => {
    await waitForQuery(screenHome.CONTROLLER_TITLE.visible());
    await activate(true);

    // Re-Configure the benchmark to use a URL that generates an HTTP error.
    this.ctx.downloadUrlCallback = async req => {
      this.ctx.networkBenchmarkOverrideEndpoints.GETs['/'].status = 400;
      await new Promise(r => setTimeout(r, 2000));
    };

    // Start the connection benchmark and wait for it to finish.
    await waitForQueryAndClick(screenHome.CONNECTION_INFO_TOGGLE);
    await waitForCondition(async () => {
      let state = await getMozillaProperty(
          'Mozilla.VPN', 'VPNConnectionBenchmark', 'state');
      return state == 'StateRunning';
    });
    await waitForCondition(async () => {
      let state = await getMozillaProperty(
          'Mozilla.VPN', 'VPNConnectionBenchmark', 'state');
      return state != 'StateRunning';
    });

    // We expect the benchmark to fail.
    strictEqual(
        await getMozillaProperty(
            'Mozilla.VPN', 'VPNConnectionBenchmark', 'state'),
        'StateError');
    await waitForQuery(screenHome.CONNECTION_INFO_ERROR.visible());

    // Let's set 200 as status code.
    this.ctx.downloadUrlCallback = async req => {
      this.ctx.networkBenchmarkOverrideEndpoints.GETs['/'].status = 200;
    };

    await waitForQueryAndClick(screenHome.CONNECTION_INFO_RETRY);
    await waitForCondition(async () => {
      let state = await getMozillaProperty(
          'Mozilla.VPN', 'VPNConnectionBenchmark', 'state');
      return state == 'StateRunning';
    });
    await waitForCondition(async () => {
      let state = await getMozillaProperty(
          'Mozilla.VPN', 'VPNConnectionBenchmark', 'state');
      return state != 'StateRunning';
    });

    // This time we expect the benchmark to succeed.
    let speed = await getMozillaProperty(
        'Mozilla.VPN', 'VPNConnectionBenchmark', 'speed');
    let downloadBps = parseInt(await getMozillaProperty(
        'Mozilla.VPN', 'VPNConnectionBenchmark', 'downloadBps'));
    let uploadBps = parseInt(await getMozillaProperty(
        'Mozilla.VPN', 'VPNConnectionBenchmark', 'uploadBps'));
    let state = await getMozillaProperty(
        'Mozilla.VPN', 'VPNConnectionBenchmark', 'state');
    strictEqual(state, 'StateReady');

    strictEqual(
        speed,
        (downloadBps >= 25000000 && uploadBps > 0) ?
            'SpeedFast' :
            ((downloadBps >= 10000000 && uploadBps > 0) ? 'SpeedMedium' :
                                                          'SpeedSlow'));

    // Exit the benchmark
    await waitForQueryAndClick(screenHome.CONNECTION_INFO_TOGGLE);
  });

  it('Error on unexpected disconnect', async () => {
    await waitForQuery(screenHome.CONTROLLER_TITLE.visible());
    await activate(true);

    // Re-Configure the benchmark to use a URL that will hang for a while.
    this.ctx.downloadUrlCallback = async req => {
      this.ctx.networkBenchmarkOverrideEndpoints.GETs['/'].status = 230;
      await new Promise(r => setTimeout(r, 10000));
    };

    // Start the connection benchmark.
    await waitForQueryAndClick(screenHome.CONNECTION_INFO_TOGGLE);
    await waitForCondition(async () => {
      let state = await getMozillaProperty(
          'Mozilla.VPN', 'VPNConnectionBenchmark', 'state');
      return state == 'StateRunning';
    });

    // Disconnect the VPN, this should trigger an error.
    await deactivate();
    await waitForCondition(async () => {
      let state = await getMozillaProperty(
          'Mozilla.VPN', 'VPNConnectionBenchmark', 'state');
      return state != 'StateRunning';
    });

    // We expect the benchmark to fail.
    strictEqual(
        await getMozillaProperty(
            'Mozilla.VPN', 'VPNConnectionBenchmark', 'state'),
        'StateError');
    await waitForQuery(screenHome.CONNECTION_INFO_ERROR.visible());

    // Exit the benchmark
    await waitForQueryAndClick(screenHome.CONNECTION_INFO_TOGGLE);
  });

  it('Error when starting with no connection', async () => {
    await waitForQuery(screenHome.CONTROLLER_TITLE.visible());
    await activate(true);

    // Force a No Signal situation
    await forceConnectionStabilityStatus('nosignal');

    // Start the connection benchmark; it goes into Running state before Error.
    await waitForQueryAndClick(screenHome.CONNECTION_INFO_TOGGLE);

    // We expect the benchmark to fail.
    strictEqual(
        await getMozillaProperty(
            'Mozilla.VPN', 'VPNConnectionBenchmark', 'state'),
        'StateError');
    await waitForQuery(screenHome.CONNECTION_INFO_ERROR.visible());

    // Exit the benchmark
    await waitForQueryAndClick(screenHome.CONNECTION_INFO_TOGGLE);
  });

  describe('speed tests related telemetry tests', () => {
    if(runningOnWasm()) {
      // No Glean on WASM.
      return;
    }

    // Telemetry design is detailed at:
    // https://miro.com/app/board/uXjVM_QZzjA=/?share_link_id=616399368132

    it("records events on successful speed test", async () => {
      this.ctx.downloadUrlCallback = async () => {};

      // Activate the VPN, otherwise the speed test feature is disabled
      await waitForQuery(screenHome.CONTROLLER_TITLE.visible());
      await activate(true);

      const [ { extra: appStepExtras } ] = await gleanTestGetValue("impression", "mainScreen", "main");
      strictEqual("main", appStepExtras.screen);

      // Start the connection benchmark.
      await waitForQueryAndClick(
          screenHome.CONNECTION_INFO_TOGGLE.visible());

      const triggeredEventsList = await gleanTestGetValue("interaction", "startSpeedTestSelected", "main")
      strictEqual(triggeredEventsList.length, 1);
      const triggeredEventExtra = triggeredEventsList[0].extra;
      strictEqual("main", triggeredEventExtra.screen);

      // Connection benchmark results loaded succesfully
      let completedEventsList;
      await waitForCondition(async () => {
        completedEventsList = await gleanTestGetValue("impression", "speedTestCompletedScreen", "main")
        return completedEventsList.length == 1;
      });

      let completedEventExtra = completedEventsList[0].extra;
      strictEqual("speed_test_completed", completedEventExtra.screen);

      const outcomeEventsList = await gleanTestGetValue("outcome", "speedTestCompleted", "main")
      strictEqual(outcomeEventsList.length, 1);
      const outcomeEventExtra = outcomeEventsList[0].extra;
      strictEqual("speed_test_completed", outcomeEventExtra.outcome);
      strictEqual(await getMozillaProperty('Mozilla.VPN', 'VPNConnectionBenchmark', 'speed'), outcomeEventExtra.speed);

      // Refresh the test
      await waitForQueryAndClick(
        screenHome.CONNECTION_INFO_RESTART.visible());

      const refreshEventsList = await gleanTestGetValue("interaction", "refreshSelected", "main")
      strictEqual(refreshEventsList.length, 1);
      const refreshEventExtra = refreshEventsList[0].extra;
      strictEqual("speed_test_completed", refreshEventExtra.screen);

      // Connection benchmark results loaded succesfully again
      await waitForCondition(async () => {
        completedEventsList = await gleanTestGetValue("impression", "speedTestCompletedScreen", "main")
        // Now the list has two events, the first one and the new one
        return completedEventsList.length == 2;
      });

      completedEventExtra = completedEventsList[1].extra;
      strictEqual("speed_test_completed", completedEventExtra.screen);

      // Close the connection benchmark
      await waitForQueryAndClick(
        screenHome.CONNECTION_INFO_TOGGLE.visible());

      const closedEventsList = await gleanTestGetValue("interaction", "closeSelected", "main")
      strictEqual(closedEventsList.length, 1);
      const closedEventExtra = closedEventsList[0].extra;
      strictEqual("speed_test_completed", closedEventExtra.screen);
    });

    it("records events while loading speed test", async () => {
      let resolveSpeedTestResults;
      this.ctx.downloadUrlCallback = async () => {
        // Hold the speed test in the loading state until we are done testing the loading state.
        await new Promise(resolve => resolveSpeedTestResults = resolve);
      };

      // Activate the VPN, otherwise the speed test feature is disabled
      await waitForQuery(screenHome.CONTROLLER_TITLE.visible());
      await activate(true);

      // Start the connection benchmark.
      await waitForQueryAndClick(
          screenHome.CONNECTION_INFO_TOGGLE.visible());

      // Connection benchmark results are loading
      let loadingEventsList;
      await waitForCondition(async () => {
        loadingEventsList = await gleanTestGetValue("impression", "speedTestLoadingScreen", "main")
        return loadingEventsList.length == 1;
      });

      let loadingEventExtra = loadingEventsList[0].extra;
      strictEqual("speed_test_loading", loadingEventExtra.screen);

      // Close the connection benchmark while loading
      await waitForQueryAndClick(
        screenHome.CONNECTION_INFO_TOGGLE.visible());

      const closedEventsList = await gleanTestGetValue("interaction", "closeSelected", "main")
      strictEqual(closedEventsList.length, 1);
      const closedEventExtra = closedEventsList[0].extra;
      strictEqual("speed_test_loading", closedEventExtra.screen);

      // Resolve the loading speed test, just in case.
      resolveSpeedTestResults();
    });

    it("records events on errored speed test", async () => {
      this.ctx.networkBenchmarkOverrideEndpoints.GETs['/'].status = 400;
      this.ctx.downloadUrlCallback = async () => {};

      // Activate the VPN, otherwise the speed test feature is disabled
      await waitForQuery(screenHome.CONTROLLER_TITLE.visible());
      await activate(true);

      const [ { extra: appStepExtras } ] = await gleanTestGetValue("impression", "mainScreen", "main")
      strictEqual("main", appStepExtras.screen);

      // Start the connection benchmark.
      await waitForQueryAndClick(
          screenHome.CONNECTION_INFO_TOGGLE.visible());

      const triggeredEventsList = await gleanTestGetValue("interaction", "startSpeedTestSelected", "main")
      strictEqual(triggeredEventsList.length, 1);
      const triggeredEventExtra = triggeredEventsList[0].extra;
      strictEqual("main", triggeredEventExtra.screen);

      // Connection benchmark results errorred
      let erroredEventsList;
      await waitForCondition(async () => {
        erroredEventsList = await gleanTestGetValue("impression", "speedTestErrorScreen", "main")
        return erroredEventsList.length == 1;
      });

      let completedEventExtra = erroredEventsList[0].extra;
      strictEqual("speed_test_error", completedEventExtra.screen);

      // Refresh the test
      await waitForQueryAndClick(
        screenHome.CONNECTION_INFO_RESTART.visible());

      const refreshEventsList = await gleanTestGetValue("interaction", "refreshSelected", "main")
      strictEqual(refreshEventsList.length, 1);
      const refreshEventExtra = refreshEventsList[0].extra;
      strictEqual("speed_test_error", refreshEventExtra.screen);

      // Connection benchmark results errored again
      await waitForCondition(async () => {
        erroredEventsList = await gleanTestGetValue("impression", "speedTestErrorScreen", "main")
        // Now the list has two events, the first one and the new one
        return erroredEventsList.length == 2;
      });

      completedEventExtra = erroredEventsList[1].extra;
      strictEqual("speed_test_error", completedEventExtra.screen);

      // Refresh the test, but this time through the "Try Again" button
      await waitForQueryAndClick(
        screenHome.CONNECTION_INFO_RETRY.visible());

      const tryAgainEventsList = await gleanTestGetValue("interaction", "refreshSelected", "main")
      // Execting two events, since this is the same as the refreshEvent
      strictEqual(tryAgainEventsList.length, 2);
      const tryAgainEventExtra = tryAgainEventsList[1].extra;
      strictEqual("speed_test_error", tryAgainEventExtra.screen);

      // Connection benchmark results errored again
      await waitForCondition(async () => {
        erroredEventsList = await gleanTestGetValue("impression", "speedTestErrorScreen", "main")
        // Now the list has three events, the first one and the new one
        return erroredEventsList.length == 3;
      });

      completedEventExtra = erroredEventsList[2].extra;
      strictEqual("speed_test_error", completedEventExtra.screen);

      // Close the connection benchmark
      await waitForQueryAndClick(
        screenHome.CONNECTION_INFO_TOGGLE.visible());

      const closedEventsList = await gleanTestGetValue("interaction", "closeSelected", "main")
      strictEqual(closedEventsList.length, 1);
      const closedEventExtra = closedEventsList[0].extra;
      strictEqual("speed_test_error", closedEventExtra.screen);
    });
  });
});
