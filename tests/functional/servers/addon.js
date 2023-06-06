/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const Server = require('./server.js');
const fs = require('fs');
const path = require('path');

const TEST_ADDONS_PATH = ('MVPN_ADDONS_PATH' in process.env) ?
                         process.env.MVPN_ADDONS_PATH : 
                         './tests/functional/addons/generated';

// This function exposes all the files for a particular addon scenario through
// the addon server.
function createScenario(scenario, addonPath) {
  const manifestPath = path.join(addonPath, 'manifest.json');
  if (!fs.existsSync(manifestPath)) {
    throw new Error(`No manifest file! ${manifestPath} should exist! Have you executed \`./scripts/addon/generate_all_tests.py'?`);
  }

  const obj = {};

  obj[`/${scenario}/manifest.json`] = {
    status: 200,
    bodyRaw: fs.readFileSync(manifestPath),
  };
  obj[`/${scenario}/manifest.json.sig`] = {
    status: 404,
    bodyRaw: '',
  };

  const files = fs.readdirSync(addonPath);
  for (const file of files) {
    const filePath = path.join(addonPath, file);
    const stat = fs.statSync(filePath);
    if (!stat.isFile()) {
      throw new Error(`Unexpected object: ${filePath}`);
    }

    obj[`/${scenario}/${file}`] = {
      status: 200,
      bodyRaw: fs.readFileSync(filePath),
    };
  }

  obj[`/${scenario}/manifest.json.sig`] = {
    status: 404,
    bodyRaw: '',
  };

  return obj;
}

let server = null;
module.exports = {
  async start(headerCheck = true) {
    let scenarios = {};

    if (!fs.existsSync(TEST_ADDONS_PATH)) {
      throw new Error(`Addon path not found! Have you executed \`./scripts/addon/generate_all_tests.py'?`);
    }

    // Generate test addon scenarios
    const dirs = fs.readdirSync(TEST_ADDONS_PATH);
    for (const dir of dirs) {
      const addonPath = path.join(TEST_ADDONS_PATH, dir);
      const stat = fs.statSync(addonPath);
      if (!stat.isDirectory()) {
        continue;
      }
      if (!fs.existsSync(path.join(addonPath, 'manifest.json'))) {
        continue;
      }

      scenarios = { ...scenarios, ...createScenario(dir, addonPath) };
    }

    if (Object.keys(scenarios).length == 0) {
      throw new Error(`No addons found! Have you executed \`./scripts/addon/generate_all_tests.py'?`);
    }

    const endpoints = {
      GETs: scenarios,
      POSTs: {},
      DELETEs: {},
    };

    server = new Server('Addon', endpoints, headerCheck);
    await server.start();
  },

  stop() {
    server.stop();
  },

  get port() {
    return server.port;
  },

  get url() {
    return server.url;
  },

  get overrideEndpoints() {
    return server.overrideEndpoints;
  },

  set overrideEndpoints(value) {
    server.overrideEndpoints = value;
  },

  throwExceptionsIfAny() {
    server.throwExceptionsIfAny();
  },

};
