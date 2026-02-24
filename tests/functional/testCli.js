/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const {spawn} = require('child_process');
const guardian = require('./servers/guardian.js');
const fxaServer = require('./servers/fxa.js');
const addonServer = require('./servers/addon.js');
const helper = require('./helper.js');
const captivePortalServer = require('./servers/captivePortalServer.js');
const crypto = require('crypto');

let stdout = '';
let stderr = '';

describe('CLI Tests', function() {
  let daemonProcess = null;
  let testEnv = {};

  before(async function() {
    if (helper.runningOnWasm()) {
      // CLI tests are not expected to run in wasm
      this.skip();
    }
    await guardian.start();
    await fxaServer.start(guardian.url);

    // Setup environment variables
    testEnv = {
      ...process.env,
      MVPN_API_BASE_URL: guardian.url,
      MZ_FXA_API_BASE_URL: fxaServer.url,
      MVPN_CONTROL_SOCKET:
          `mozillavpn-mock-${crypto.randomBytes(8).toString('hex')}`
    };

    // Start mock daemon
    daemonProcess = spawn(process.env.MVPN_BIN, ['mockdaemon'], {env: testEnv});

    // Wait for daemon to be ready
    await new Promise(resolve => setTimeout(resolve, 1000));

    await execCli(['logout', '-t']);
  });

  after(async function() {
    if (helper.runningOnWasm()) {
      // CLI tests are not expected to run in wasm
      return;
    }
    // Kill mockdaemon
    if (daemonProcess && !daemonProcess.killed) {
      daemonProcess.kill();
    }
    daemonProcess = null;

    // Stop servers and throw if they had any exceptions during the tests
    guardian.stop();
    fxaServer.stop();

    guardian.throwExceptionsIfAny();
    fxaServer.throwExceptionsIfAny();
  });

  afterEach(async function() {
    // Logout after each test to reset vpn
    await execCli(['logout', '-t']);
    // If a test failed dump stdout and stderr for debugging
    if (!this.currentTest.state || this.currentTest.state === 'failed') {
      console.log('::group::Stdout Logs');
      console.log(stdout);
      console.log('::endgroup');
      console.log('::group::Stderr Logs');
      console.log(stderr);
      console.log('::endgroup');
    }
  });

  function execCli(args, options = {}) {
    return new Promise((resolve, reject) => {
      const child =
          spawn(process.env.MVPN_BIN, args, {env: testEnv, ...options});

      stdout = '';
      stderr = '';

      child.stdout.on('data', (data) => {
        const output = data.toString();
        stdout += output;
        if (options.onStdout) {
          options.onStdout(output, child);
        }
      });

      child.stderr.on('data', (data) => {
        const output = data.toString();
        stderr += output;
        if (options.onStderr) {
          options.onStderr(output, child);
        }
      });

      child.on('close', (code) => {
        resolve({code, stdout, stderr});
      });

      child.on('error', (error) => {
        reject(error);
      });
    });
  }

  async function cliLogin() {
    const result = await execCli(['login', '-d', '-t'], {
      onStdout: (output, child) => {
        if (output.includes('Enter the code:')) {
          child.stdin.write('the_code\n');
        }
      }
    });
    return result;
  }

  describe('Login and Logout Commands', () => {
    it('Login should succeed with valid credentials', async () => {
      const result = await cliLogin();
      assert(result.code === 0, 'Login command should exit with code 0');

      const statusResult = await execCli(['status', '-t', '-c'])
      assert(
          statusResult.stdout.includes('User status: authenticated'),
          'Should indicate authenticated status');
    });

    it('Should fail when already authenticated', async () => {
      await cliLogin();

      // Attempt to login again
      const result = await execCli(['login', '-d', '-t']);
      assert(result.code !== 0, 'Should fail when already authenticated');
      assert(
          result.stdout.includes('User status: already authenticated'),
          'Should indicate already authenticated');
    });

    it('Should logout successfully when authenticated', async () => {
      const resultNotLoggedIn = await execCli(['logout', '-t']);
      assert(
          resultNotLoggedIn.code === 1,
          'Logout should fail when not authenticated');
      assert(
          resultNotLoggedIn.stdout.includes('User status: not authenticated'),
          'Should indicate not authenticated');

      await cliLogin();

      const result = await execCli(['logout', '-t']);
      assert(result.code === 0, 'Should exit with code 0');
      assert(result.stdout === '', 'Should have no stdout output');

      const statusResult = await execCli(['status', '-t']);
      assert(
          statusResult.stdout.includes('User status: not authenticated'),
          'Should indicate not authenticated after logout');
    });
  });

  describe('Status Command', () => {
    it('Output not authenticated when not logged in', async () => {
      const result = await execCli(['status', '-t']);
      assert(result.code === 0, 'Status command should exit with code 0');
      assert(
          result.stdout.includes('User status: not authenticated'),
          'Should indicate not authenticated');
    });

    it('Output VPN connection when logged in', async () => {
      await cliLogin();

      const result = await execCli(['status', '-t', '-c']);

      // User should be authenticated, have a valid subscription, 1 devices and
      // VPN should be off
      assert(result.code === 0, 'Status command should exit with code 0');
      assert(
          result.stdout.includes('User status: authenticated'),
          'Should show authenticated status');
      assert(
          result.stdout.includes('User subscription needed: false'),
          'Should have a valid subscription');
      assert(
          result.stdout.includes('Active devices: 1'),
          'Should show 1 active device');
      assert(
          result.stdout.includes('VPN state: off'),
          'VPN should be off by default');
    });
  });

  describe('Servers command', () => {
    it('Should list servers when authenticated', async () => {
      await cliLogin();

      // List servers
      const result = await execCli(['servers', '-t', '-c']);
      assert(result.code === 0, 'Should exit successfully');
      assert(
          result.stdout.includes('City: Vienna'),
          'Should include Vienna server');
    });

    it('Should support JSON format with -j flag', async () => {
      await cliLogin();

      // List servers in JSON format
      const result = await execCli(['servers', '-t', '-j']);
      assert(result.code === 0, 'Should exit successfully');
      assert(
          JSON.parse(result.stdout).length > 0, 'Should contain server data');
    });
  });

  describe('Activate/Deactivate Commands', () => {
    it('should activate and deactivate VPN when authenticated', async () => {
      await cliLogin();

      // Activate VPN
      const activateResult = await execCli(['activate', '-t']);
      assert(activateResult.code === 0, 'Should exit with code 0');
      assert(
          activateResult.stdout.includes('The VPN tunnel is now active'),
          'Should activate successfully');

      // Check status to confirm VPN is on
      const statusResult = await execCli(['status', '-t', '-c']);
      assert(statusResult.code === 0, 'Should exit with code 0');
      assert(
          statusResult.stdout.includes('VPN state: on'),
          'VPN should be on after activation');

      // Then deactivate
      const deactivateResult = await execCli(['deactivate', '-t']);
      assert(deactivateResult.code === 0, 'Should deactivate successfully');
      assert(
          deactivateResult.stdout.includes('The VPN tunnel is now inactive'),
          'Should deactivate successfully');

      // Check status again to confirm VPN is off
      const statusAfterDeactivate = await execCli(['status', '-t', '-c']);
      assert(statusAfterDeactivate.code === 0, 'Should exit with code 0');
      assert(
          statusAfterDeactivate.stdout.includes('VPN state: off'),
          'VPN should be off after deactivation');
    });
  });

  describe('Select command', () => {
    it('Select a single server', async () => {
      await cliLogin();
      // Check initial server
      const result = await execCli(['status', '-t']);
      assert(result.code === 0, 'Should exit successfully');
      assert(
          result.stdout.includes('Server city: Vienna'),
          'Should show current server as Vienna');

      // Select a different server (Brussels)
      const selectResult = await execCli(['select', '-t', 'host-be-bru']);
      assert(selectResult.code === 0, 'Should exit successfully');
      assert(
          selectResult.stdout == '',
          'Stdout should be empty on successful select');

      // Check that the server was updated
      const statusAfterSelect = await execCli(['status', '-t']);
      assert(statusAfterSelect.code === 0, 'Should exit successfully');
      assert(
          statusAfterSelect.stdout.includes('Server city: Brussels'),
          'Should show Brussels as current server');
    });

    it('Select multiple servers', async () => {
      await cliLogin();
      // Check initial server
      const result = await execCli(['status', '-t']);
      assert(result.code === 0, 'Should exit successfully');
      assert(
          result.stdout.includes('Server city: Vienna'),
          'Should show current server as Vienna');

      // Select entry and exit servers for multihop
      const selectResult =
          await execCli(['select', '-t', 'host-at-vie', 'host-be-bru']);
      assert(selectResult.code === 0, 'Should exit successfully');
      assert(
          selectResult.stdout == '',
          'Stdout should be empty on successful select');

      // Check that both entry and exit servers were selected
      const statusAfterSelect = await execCli(['status', '-t']);
      assert(statusAfterSelect.code === 0, 'Should exit successfully');
      assert(
          statusAfterSelect.stdout.includes('Entry server city: Brussels'),
          'Should show Brussels as current entry server');
      assert(
          statusAfterSelect.stdout.includes('Exit server city: Vienna'),
          'Should show Vienna as current exit server');
    });
  });

  describe('Wgconf command', () => {
    it('Single server scenario', async () => {
      await cliLogin();
      // Select a server
      const selectResult = await execCli(['select', '-t', 'host-be-bru']);
      assert(selectResult.code === 0, 'Should exit successfully');

      // Get wgconf output and check the selected server is included in the
      // config by checking the comment
      const wgconf = await execCli(['wgconf', '-t']);
      assert(wgconf.code === 0, 'Should exit successfully');
      assert(wgconf.stdout.includes('[Peer]'), 'Should contain a Peer section');
      assert(
          wgconf.stdout.includes('# Exit Server: host-be-bru - Brussels (be)'),
          'Should show Brussels as current server');
    });

    it('Multihop scenario', async () => {
      await cliLogin();
      // Select entry and exit servers for multihop
      const selectResult =
          await execCli(['select', '-t', 'host-at-vie', 'host-be-bru']);
      assert(selectResult.code === 0, 'Should exit successfully');

      // Get wgconf output and check that both entry and exit servers are
      // included in the config by checking the comments Check only one Peer is
      // present for the mullvad multihop config
      const wgconfMullvadMultihop =
          await execCli(['wgconf', '-t', '--mullvad-multihop']);
      assert(wgconfMullvadMultihop.code === 0, 'Should exit successfully');
      assert(
          (wgconfMullvadMultihop.stdout.match(/\[Peer\]/g) || []).length == 1,
          'Should contain 1 peer for mullvad multihop');
      assert(
          wgconfMullvadMultihop.stdout.includes(
              '# Entry Server: host-be-bru - Brussels (be)'),
          'Should show Brussels as entry server');
      assert(
          wgconfMullvadMultihop.stdout.includes(
              '# Exit Server: host-at-vie - Vienna (at)'),
          'Should show Vienna as exit server');

      // Check two Peers are present for the wireguard multihop config (one for
      // entry and one for exit server)
      const wgconfWireguardMultihop =
          await execCli(['wgconf', '-t', '--wireguard-multihop']);
      assert(wgconfWireguardMultihop.code === 0, 'Should exit successfully');
      assert(
          (wgconfWireguardMultihop.stdout.match(/\[Peer\]/g) || []).length == 2,
          'Should contain 2 peers for multihop');
      assert(
          wgconfWireguardMultihop.stdout.includes(
              '# Entry Server: host-be-bru - Brussels (be)'),
          'Should show Brussels as entry server');
      assert(
          wgconfWireguardMultihop.stdout.includes(
              '# Exit Server: host-at-vie - Vienna (at)'),
          'Should show Vienna as exit server');
    });
  });

  describe('Device command', () => {
    it('Error on attempt to remove current device', async () => {
      await cliLogin();
      const result = await execCli(['device', '-t', '1']);
      assert(result.code === 1, 'Should exit with error code');
      assert(
          result.stdout.includes(
              'Removing the current device is not allowed. Use \'logout\' instead.'),
          'Should show error message');
    });

    it('Error on invalid inputs', async () => {
      await cliLogin();

      // Attempt to remove a device with a non number id
      const deviceNotANumber = await execCli(['device', '-t', 'invalid']);
      assert(deviceNotANumber.code === 1, 'Should exit with error code');
      assert(
          deviceNotANumber.stdout.includes('invalid is not a valid number.'),
          'Should show error message');

      // Attempt to remove a device with an invalid ID
      const deviceInvalidId = await execCli(['device', '-t', '999']);
      assert(deviceInvalidId.code === 1, 'Should exit with error code');
      assert(
          deviceInvalidId.stdout.includes('999 is not a valid ID.'),
          'Should show error message');
    });
  });

  describe('Exclude-ip command', () => {
    it('Should add, list and remove excluded IPs', async () => {
      await cliLogin();

      // Add an IP to the exclude list
      const addResult = await execCli(['excludeip', '-t', 'add', '1.2.3.4']);
      assert(addResult.code === 0, 'Should exit successfully');
      assert(
          addResult.stdout == '', 'Stdout should be empty on successful add');

      // Check that the IP is in the exclude list
      const listResult = await execCli(['excludeip', '-t', 'list']);
      assert(listResult.code === 0, 'Should exit successfully');
      assert(
          listResult.stdout.includes('- 1.2.3.4/32'),
          'Should list excluded IP');

      // Remove the IP from the exclude list
      const removeResult =
          await execCli(['excludeip', '-t', 'remove', '1.2.3.4']);
      assert(removeResult.code === 0, 'Should exit successfully');
      assert(
          !removeResult.stdout.includes('- 1.2.3.4/32'),
          'List should be empty on successful remove');
    });
  });
});
