/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import Server from './server.js';
import { existsSync, readFileSync, readdirSync, statSync } from 'fs';
import { join, dirname } from 'path';
import {env} from 'process';
import fs from 'node:fs';
import path from 'node:path';

// If not specified, assume the test addons can be found in the 'addons' subdir
// under the MVPN_BIN binary
const TEST_ADDONS_PATH = ('MVPN_ADDONS_PATH' in env) ?
                         env["MVPN_ADDONS_PATH"] : 
                         join(dirname(env["MVPN_BIN"]), 'addons');

// This function exposes all the files for a particular addon scenario through
// the addon server.
function createScenario(scenario, addonPath) {
  const manifestPath = join(addonPath, 'manifest.json');
  if (!existsSync(manifestPath)) {
    throw new Error(`No manifest file! ${manifestPath} should exist!?`);
  }

  const obj = {};

  obj[`/${scenario}/manifest.json`] = {
    status: 200,
    bodyRaw: readFileSync(manifestPath),
  };
  obj[`/${scenario}/manifest.json.sig`] = {
    status: 404,
    bodyRaw: '',
  };

  const files = readdirSync(addonPath);
  for (const file of files) {
    const filePath = join(addonPath, file);
    const stat = statSync(filePath);
    if (!stat.isFile()) {
      throw new Error(`Unexpected object: ${filePath}`);
    }

    obj[`/${scenario}/${file}`] = {
      status: 200,
      bodyRaw: readFileSync(filePath),
    };
  }

  obj[`/${scenario}/manifest.json.sig`] = {
    status: 404,
    bodyRaw: '',
  };

  return obj;
}

let server = null;
export const addonServer =  {
  async start(headerCheck = true) {
    let scenarios = {};

    if (!fs.existsSync(TEST_ADDONS_PATH)) {
      throw new Error(`Addon path not found!`);
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
      throw new Error(`No addons found!?`);
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
