import chalk from 'chalk';
import child_process from 'child_process';
import fs from 'fs';
import program from 'commander';
import readline from 'readline';
import websocket from 'websocket';

const DEFAULT_MOZILLAVPN = '../../src/mozillavpn';

const TEMPLATE = `
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// NOTE: this test has been generated!

const assert = require('assert');
const util = require('util');
const vpn = require('./helper.js');

const exec = util.promisify(require('child_process').exec);

describe('Automated test', function() {
  this.timeout(300000);

  before(async () => {
    await vpn.connect();
  });

  beforeEach(() => {});

  afterEach(vpn.dumpFailure);

  after(async () => {
    vpn.disconnect();
  });

  it('Test', async () => {
    TEST_TEST_TEST
  });

  it('quit the app', async () => await vpn.quit());
});
`;

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
  run(config, parts) {}
}

class Quit extends Command {
  get cmd() {
    return 'quit';
  }

  get desc() {
    return 'Quit the app';
  }

  run(config, parts) {
    Testify.quit();
  }
}

class Help extends Command {
  get cmd() {
    return 'help';
  }

  get desc() {
    return 'This help menu';
  }

  run(config, parts) {
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

class Record extends Command {
  get cmd() {
    return 'record';
  }

  get desc() {
    return 'Record something';
  }

  get params() {
    return [{
      name: 'allEvents',
      params: ['--all', '-a'],
      desc: 'Retrieve all the events (and not just the main ones.'
    }];
  }

  async run(config, parts) {
    if (parts.length > 2) {
      console.log(chalk.red('Max 1 param supported.'));
      return;
    }

    let allEvents = false;
    if (parts.length > 1) {
      if (parts[1] !== '-a' && parts[1] !== '--all') {
        console.log(chalk.red(`Unknown param ${parts[1]}.`));
        return;
      }

      allEvents = true;
    }

    await Testify.sendMessageToClient(
        'start_recording', `/tmp/foo.txt ${allEvents}`);

    console.log(`Recording. Press <${chalk.bold('enter')}> to stop...`);
    await new Promise(r => Testify.rl.question('recording> ', a => r(a)));

    await Testify.sendMessageToClient('stop_recording');

    if (!fs.existsSync('/tmp/foo.txt')) {
      console.log(chalk.red('Nothing has been recorded.'));
      return;
    }

    const events = fs.readFileSync('/tmp/foo.txt', 'utf8').trim().split('\n');
    console.log(chalk.green(`Added ${chalk.bold(events.length)} events.`));

    steps.push({
      type: 'replay',
      events,
    });
  }
}

class Pick extends Command {
  get cmd() {
    return 'pick';
  }

  get desc() {
    return 'Pick an element to explore';
  }

  async run(config, parts) {
    await Testify.sendMessageToClient('pick');

    console.log('Pick an element, please.');
    const elements = await new Promise(async r => {
      while (true) {
        const answer = await Testify.sendMessageToClient('picked');
        if (answer.clicked) {
          r(answer.value);
          break;
        }

        await new Promise(r => setTimeout(r, 200));
      }
    });

    if (elements.length === 0) {
      console.log(chalk.red(
          'You have not selected an object with a name. Please, add names in the QML world.'));
      return;
    }

    if (elements.length > 1) {
      console.log(
          `You have selected more than 1 elements: ${elements.join(', ')}`);
      console.log('Let\'s continue with the first one only.');
    }

    console.log(`Tell me something about element "${elements[0]}"`);

    const step = {
      type: 'elm',
      name: elements[0],
      properties: {},
    };

    const answer = await Testify.sendMessageToClient('list_json', elements[0]);
    const maxLength =
        Math.max(...Object.keys(answer.value).map(prop => prop.length));
    Object.keys(answer.value).forEach(prop => {
      if (answer.value[prop] !== null) {
        process.stdout.write(chalk.blue(prop));
        for (let i = prop.length; i < maxLength; ++i) process.stdout.write(' ');
        process.stdout.write(`\t${answer.value[prop]}\n`);
      }
    });

    while (true) {
      const prop = await new Promise(
          r => Testify.rl.question(
              `Property to check (${chalk.bold('empty to stop')})> `,
              a => r(a)));
      if (prop === '') break;

      if (!(prop in answer.value)) {
        console.log(chalk.red('This property doesn\'t exist.'));
        continue;
      }

      const value = await new Promise(
          r => Testify.rl.question(
              `What is the value? (${chalk.bold(answer.value[prop])})> `,
              a => r(a)));

      step.properties[prop] = value === '' ? answer.value[prop] : value;
    }

    console.log(chalk.green('Added a picked-element.'));

    steps.push(step);
  }
}

class Finalize extends Command {
  get cmd() {
    return 'done';
  }

  get desc() {
    return 'Finalize the test';
  }

  async run(config, parts) {
    console.log(`Your test is composed by ${chalk.bold(steps.length)} steps`);

    const content = TEMPLATE.replace(
        'TEST_TEST_TEST', await this.serializeSteps(Testify.rl));
    fs.writeFileSync(config.testFile, content);

    console.log(chalk.bold.green('All done!'));

    Testify.quit();
  }

  async serializeSteps(rl) {
    const loginNeeded = await new Promise(async r => {
      while (true) {
        const answer = await new Promise(
            r => rl.question(
                `Do you need to run behind authentication? (${
                    chalk.bold('Y/n')})> `,
                a => r(a)));
        if (answer === 'n' || answer === 'N') {
          r(false);
          break;
        }

        if (answer === '' || answer === 'y' || answer === 'Y') {
          r(true);
          break;
        }

        console.log('Sorry, can you repeat?');
      }
    });

    let content = [];

    if (loginNeeded) {
      content.push('// Authenticate');
      content.push('await vpn.authenticate();');
    }

    steps.forEach((step, pos) => {
      content.push();
      content.push(`// Step ${pos}`);
      content.push();

      switch (step.type) {
        case 'elm':
          content = content.concat(this.serializeStepElm(step));
          break;
        case 'replay':
          content = content.concat(this.serializeStepReplay(step));
          break;
        default:
          console.log('Unsupported step!');
          break;
      }
    });

    if (loginNeeded) {
      content.push('// Logout');
      content.push('await vpn.logout();');
      content.push('await vpn.wait();');
    }

    return content.join('\n    ');
    ;
  }

  serializeStepElm(step) {
    const content = [`await vpn.waitForElement('${step.name}');`];
    Object.keys(step.properties).forEach(prop => {
      content.push(
          `await vpn.waitForElementProperty('${this.escape(step.name)}', '${
              this.escape(prop)}', '${this.escape(step.properties[prop])}');`);
    });

    return content;
  }

  serializeStepReplay(step) {
    const content = ['await vpn.sendEvents(['];
    step.events.forEach(line => {
      if (line.length) content.push(`  ${line},`);
    });
    content.push(']);');
    return content;
  }

  escape(str) {
    // This is horrible.
    return str.replace('\'', '\\\'');
  }
}

const Commands = [
  new Quit(),
  new Help(),
  new Record(),
  new Pick(),
  new Finalize(),
];

const Testify = {
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
        console.log('The client has closed the webscoke connection.');
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
    program.argument('<testFile>', 'The test file');
    program.action(async testFile => await this.run(program.opts(), testFile));

    program.parseAsync(process.argv);
  },

  async run(options, testFile) {
    this.execClient(options.path);
    await this.connectToClient();

    this._rl = readline.createInterface({
      input: process.stdin,
      output: process.stdout,
      prompt: '> ',
      completer: line => this.completer(line),
    });

    this.rl.prompt();

    this.rl.on('close', () => {
      console.log('Bye!');
      this.vpn.kill();
      process.exit();
    });

    const processLine = async line => {
      const parts = line.trim().split(' ');
      if (parts.length === 0 || parts[0] === '') return;

      const cmd = Commands.find(c => c.cmd === parts[0]);
      if (!cmd) {
        console.log(`What do you mean with "${parts[0]}"`);
        return;
      }

      const config = {testFile};
      await cmd.run(config, parts);
    };

    this.rl.on('line', async line => {
      await processLine(line);
      this.rl.prompt();
    });
  },

  completer(line) {
    const trimmedLine = line.trimStart();
    if (trimmedLine.indexOf(' ') === -1) {
      const options = [];
      Commands.forEach(c => {
        if (c.cmd.startsWith(trimmedLine)) options.push(c.cmd);
      });
      return [options, line];
    }

    const parts = trimmedLine.split(' ');
    const cmd = Commands.find(c => c.cmd === parts[0]);
    if (!cmd) {
      return [[], line];
    }

    const part = parts[parts.length - 1];
    const options = [];
    cmd.params.forEach(param => {
      param.params.forEach(p => {
        if (p.startsWith(part)) options.push(p);
      });
    });

    return [options, line];
  }
};

Testify.init();
