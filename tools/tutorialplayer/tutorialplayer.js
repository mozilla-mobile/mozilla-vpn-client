/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import chalk from 'chalk';
import child_process from 'child_process';
import fs from 'fs';
import program from 'commander';
import websocket from 'websocket';

const DEFAULT_MOZILLAVPN = '../../src/mozillavpn';

const TutorialPlayer = {
  get vpn() {
    return this._vpn;
  },
  get client() {
    return this._client;
  },
  get rl() {
    return this._rl;
  },

  quit() {
    if (this.vpn) {
      this.vpn.kill();
    }

    process.exit();
  },

  async sendMessageToClient(cmd, params = '', reportError = false) {
    let answer = await new Promise(resolve => {
      this._waitReadCallback = resolve;
      this.client.send(`${cmd} ${params}`.trim());
    });

    if (answer.type !== cmd) {
      console.log('Something wrong is happening.');
      this.quit();
    }

    if (!reportError) {
      if (answer.error) {
        console.log(`Something wrong is happening: ${answer.error}`);
        this.quit();
      }
    }

    return answer;
  },

  execClient(path) {
    const vpn = child_process.spawn(path, ['ui', '--testing']);

    console.log('Running the client...');
    vpn.stderr.on('data', data => {});
    vpn.stdout.on('data', data => {});
    vpn.on('close', code => {
      console.log(
          'The client has been closed. I hope this is what you wanted.');
      this.quit();
    });

    this._vpn = vpn;
  },

  async connectToClient() {
    // Let's wait 1 sec for the activation of the websocket.
    await new Promise(r => setTimeout(r, 1000));

    const client = await new Promise(resolve => {
      const client = new websocket.w3cwebsocket('ws://localhost:8765/', '');

      client.onopen = async () => resolve(client);

      client.onclose = () => {
        console.log('The client has closed the websocket connection.');
        this.quit();
      };

      client.onerror = () => resolve(null);

      client.onmessage = data => {
        const json = JSON.parse(data.data);

        if (json.type === 'tutorialCompleted') {
          console.log('Tutorial completed!');
          return;
        }

        if (json.type === 'tutorialStarted') {
          console.log('Tutorial started!');
          return;
        }

        // Ignoring logs.
        if (json.type === 'log') return;

        // Ignore notifications.
        if (json.type === 'notification') return;

        if (!this._waitReadCallback) {
          console.log('Internal error?!?');
          return;
        }

        const wr = this._waitReadCallback;
        this._waitReadCallback = null;
        wr(json);
      };
    });

    if (!client) {
      console.log('Failed to connect.');
      process.exit();
    }

    this._client = client;
  },

  async init() {
    program.version('0.0.1')

    program.option(
        '-p, --path <mozillavpn>',
        `config file. Default: ${DEFAULT_MOZILLAVPN}`, DEFAULT_MOZILLAVPN);
    program.argument('<tutorialFile>', 'The tutorial file');
    program.action(
        async tutorialFile => await this.run(program.opts(), tutorialFile));

    program.parseAsync(process.argv);
  },

  async waitForElement(id) {},

  async run(options, tutorialFile) {
    this.execClient(options.path);
    await this.connectToClient();

    console.log('Wait for the main view to be shown...');

    while (true) {
      const answer = await this.sendMessageToClient('has', 'deviceListButton');
      if (answer.value) break;
      await new Promise(resolve => setTimeout(resolve, 200));
    }

    while (true) {
      const answer = await this.sendMessageToClient(
          'property', 'deviceListButton visible');
      if (answer.value) break;
      await new Promise(resolve => setTimeout(resolve, 200));
    }

    console.log('Ready!');

    await this.sendMessageToClient('tutorial', tutorialFile);
  }
};

TutorialPlayer.init();
