/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import assert from 'assert';
import { w3cwebsocket as websocket } from 'websocket';
import { URL } from 'node:url';
import http from 'http';

let driver;
let intervalId;

async function waitForURL(driver, url) {
  await driver.setContext('content');

  // I'm sure there is something better than this, but this is the only
  // solution to monitor the tab loading so far.
  return await new Promise(resolve => {
    const check = async () => {
      const handles = await driver.getAllWindowHandles();
      for (let handle of handles) {
        await driver.switchTo().window(handle);
        const t = await driver.getCurrentUrl();
        if (t.includes(url)) {
          resolve(handle);
          return;
        }
      }

      setTimeout(check, 500);
    };

    check();
  });
}

export async function connect(options, onopen, onclose, onmessage) {
  driver = options.driver;

  await driver.setContext('content');
  const handle = await waitForURL(driver, options.url);

  const objs = await driver.executeScript('return flushMessages()');
  if (!Array.isArray(objs) || objs.length === 0) return false;

  intervalId = setInterval(async () => {
    const objs = await driver.executeScript('return flushMessages()');
    if (Array.isArray(objs)) objs.forEach(obj => onmessage(obj));
  }, 200);

  await onopen();
  setTimeout(() => objs.forEach(obj => onmessage(obj)), 0);
  return true;
}
export function close() {
  clearInterval(intervalId);
  driver.get('about:blank');
}
export async function send(msg) {
  await driver.executeScript(`Module.inspectorCommand("${msg.replace(/[\\"']/g, '\\$&').replace(/\u0000/g, '\\0')}")`);
}
