/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import chalk from 'chalk';
import child_process from 'child_process';
import fs from 'fs';
import fuzzy from 'fuzzy';
import program from 'commander';
import inquirer from 'inquirer';
import inquirerAutocompletePrompt from 'inquirer-autocomplete-prompt';
import websocket from 'websocket';
import YAML from 'yaml'
import pascalize from 'pascalize';

const DEFAULT_MOZILLAVPN = '../../src/mozillavpn';

const steps = [];

class Command {
  get cmd() {
    return '';
  }
  get desc() {
    return '';
  }
  get params() {
    return []
  }
  run(config) {}
}

class Quit extends Command {
  get cmd() {
    return 'quit';
  }

  get desc() {
    return 'Quit the app';
  }

  run(config) {
    TutorialGen.quit();
  }
}

class Help extends Command {
  get cmd() {
    return 'help';
  }

  get desc() {
    return 'This help menu';
  }

  run(config) {
    Commands.forEach(c => {
      process.stdout.write(`${chalk.blue(c.cmd)}\t${c.desc}\n`);

      c.params.forEach(param => {
        process.stdout.write(
            `\t${param.params.map(a => chalk.green(a)).join(' | ')}\t${
                param.desc}\n`);
      });
    });
  }
}

class Tooltip extends Command {
  get cmd() {
    return 'tooltip';
  }

  get desc() {
    return 'Show a tooltip next to an element';
  }

  async run(config) {
    const answer = await inquirer.prompt([{
      type: 'autocomplete',
      name: 'value',
      message: 'String ID:',
      source: (answers, input) =>
          fuzzy.filter(input || '', config.stringIds, {})
              .map(el => el.original),
    }]);
    const stringId = answer.value;
    if (!stringId) {
      console.log(chalk.red('A string ID is needed.'));
      return;
    }

    await TutorialGen.sendMessageToClient('pick');

    console.log('Pick an element, please.');
    let elements = await new Promise(async r => {
      while (true) {
        const answer = await TutorialGen.sendMessageToClient('picked');
        if (answer.clicked) {
          r(answer.value);
          break;
        }

        await new Promise(r => setTimeout(r, 200));
      }
    });

    elements = [...new Set(elements)];

    if (elements.length === 0) {
      console.log(chalk.red(
          'You have not selected an object with a name. Please, add names in the QML world.'));
      return;
    }

    if (elements.length > 1) {
      console.log(`You have selected more than 1 elements: ${
          elements.join(', ')}. Pick one.`);

      const elementAnswer = await inquirer.prompt([{
        type: 'autocomplete',
        name: 'value',
        message: '>',
        source: (elementAnswers, input) =>
            fuzzy.filter(input || '', elements, {}).map(el => el.original),
      }]);
      elements = [elementAnswer.value];
    }

    console.log(chalk.green('Added a tooltip.'));

    const step = {
      type: 'tooltip',
      name: elements[0],
      id: stringId,
    };

    steps.push(step);
  }
}

class Finalize extends Command {
  get cmd() {
    return 'done';
  }

  get desc() {
    return 'Finalize the tutorial file';
  }

  async run(config) {
    console.log(
        `Your tutorial file is composed by ${chalk.bold(steps.length)} steps`);

    const content = await this.serializeSteps(TutorialGen.rl);
    fs.writeFileSync(config.tutorialFile, content);

    console.log(chalk.bold.green('All done!'));

    TutorialGen.quit();
  }

  async serializeSteps(rl) {
    let content = [
      '# This Source Code Form is subject to the terms of the Mozilla Public',
      '# License, v. 2.0. If a copy of the MPL was not distributed with this',
      '# file, You can obtain one at http://mozilla.org/MPL/2.0/. */',
      '',
    ];

    steps.forEach((step, pos) => {
      switch (step.type) {
        case 'tooltip':
          content = content.concat(this.serializeStepTooltip(step));
          break;
        default:
          console.log('Unsupported step!');
          break;
      }
    });

    return content.join('\n') + '\n';
    ;
  }

  serializeStepTooltip(step) {
    return [`TOOLTIP ${step.id} ${step.name}`];
  }
}

const Commands = [
  new Quit(),
  new Help(),
  new Tooltip(),
  new Finalize(),
];

const TutorialGen = {
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

  async run(options, tutorialFile) {
    this.execClient(options.path);
    await this.connectToClient();

    const yaml = YAML.parse(
        fs.readFileSync('../../translations/strings.yaml').toString());
    const stringIds = [];
    Object.keys(yaml).forEach(categories => {
      Object.keys(yaml[categories]).forEach(key => {
        stringIds.push(pascalize(categories + '_' + key));
      });
    });

    inquirer.registerPrompt('autocomplete', inquirerAutocompletePrompt);

    while (true) {
      const answer = await inquirer.prompt([{
        type: 'autocomplete',
        name: 'value',
        message: '>',
        source: (answers, input) =>
            fuzzy
                .filter(
                    input || '', Commands.map(c => ({name: c.cmd, value: c})),
                    {extract: el => el.name})
                .map(el => el.original),
      }]);

      const config = {tutorialFile, stringIds};
      await answer.value.run(config);
    };
  },
};

TutorialGen.init();
