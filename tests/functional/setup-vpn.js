/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* This is a GLOBAL setup file.
 * before applies to before running all tests.
 * beforeEach applies to running before every test.
 */
const {config} = require('dotenv');
const fs = require('fs');
const {exec, spawn} = require('child_process');
const vpn = require('./helper.js');
const {assert} = require('console');

// Inject environment variables from .env file.
config();

const app = process.env.MVPN_TEST_BIN;
let vpnProcess = null;
let stdErr = '';

before('Check VPN available', function() {
  exec(`${app} --version`, (error, stdout, stderr) => {
    if (error) {
      console.error(`Could not run "${app}".`);
      console.error('Have you set MVPN_BIN in .env or environment?');
      console.error(`stdout: ${stdout}`);
      console.error(`stderr: ${stderr}`);
      process.exit(1);
    }
    console.log(`VPN Version is: ${stdout}`);
  });
})

beforeEach('Start VPN app', () => {
  vpnProcess = spawn(app, ['ui', '--testing']);
  stdErr += 'VPN Process ID: ' + vpnProcess.pid;
  vpnProcess.stderr.on('data', (data) => {
    stdErr += data;
  });
})

beforeEach('Connect to VPN', async () => {
  await vpn.connect();
});

beforeEach('Reset the VPN', async () => {
  await vpn.reset();
  vpn.resetLastNotification();
})

// Notes:
// - Cannot be an arrow function, as we need `this`.
// - Must be before disconnect.
afterEach('Collect errors on failure', async function() {
  if (this.currentTest.state === 'failed') {
    // Print error logs
    console.log('::group::Error Logs');
    console.log(stdErr);
    console.log('::endgroup');

    // Screenshot of failure state
    const data = await vpn.screenCapture();
    const buffer = Buffer.from(data, 'base64');
    const dir = process.env.ARTIFACT_DIR + '/screencapture';
    const title = this.currentTest.title.toLowerCase();
    const filename = title.replace(/[^a-z0-9]/g, '_');
    if (!fs.existsSync(dir)) {
      fs.mkdirSync(dir);
    }
    fs.writeFileSync(`${dir}/${filename}.png`, buffer);
  }
  // Reset error logs
  stdErr = '';
})

afterEach('Close VPN app', async () => {
  await vpn.quit()
  vpn.disconnect();
  vpnProcess.kill();
})
