/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import chalk from 'chalk';
import child_process from 'child_process';
import program from 'commander';
import fs from 'fs';
import inquirer from 'inquirer';
import fetch from 'node-fetch';
import websocket from 'websocket';
import WBK from 'wikibase-sdk';

const DEFAULT_MOZILLAVPN = '../../build/src/mozillavpn';
const LANGUAGES_OUTPUT_FILE = '../../translations/languages.json';

const LanguageLocalizer = {
  get vpn() {
    return this._vpn;
  },
  get client() {
    return this._client;
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
      console.log(chalk.red('Something wrong is happening.'));
      this.quit();
    }

    if (!reportError) {
      if (answer.error) {
        console.log(chalk.red(`Something wrong is happening: ${answer.error}`));
        this.quit();
      }
    }

    return answer;
  },

  execClient(path) {
    const vpn = child_process.spawn(path, ['ui', '--testing']);

    console.log(chalk.yellow('Running the client...'));
    vpn.stderr.on('data', data => {});
    vpn.stdout.on('data', data => {});
    vpn.on('close', code => {
      console.log(chalk.red(
          'The client has been closed. I hope this is what you wanted.'));
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
        console.log(
            chalk.yellow('The client has closed the websocket connection.'));
        this.quit();
      };

      client.onerror = () => resolve(null);

      client.onmessage = data => {
        const json = JSON.parse(data.data);

        // Ignoring logs.
        if (json.type === 'log') return;

        // Ignore notifications.
        if (json.type === 'notification' ||
            json.type === 'addon_load_completed')
          return;

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

  retrieveJson(url) {
    return fetch(url, {
             headers: {
               'User-Agent':
                   'Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:86.0) Gecko/20100101 Firefox/86.0',
               'Accept':
                   'text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8',
             }
           })
        .then(r => r.json());
  },

  async init() {
    program.version('0.0.1')

    program.option(
        '-p, --path <mozillavpn>',
        `config file. Default: ${DEFAULT_MOZILLAVPN}`, DEFAULT_MOZILLAVPN);
    program.action(async () => await this.run(program.opts()));

    program.parseAsync(process.argv);
  },

  async run(options) {
    this.execClient(options.path);
    await this.connectToClient();

    this.wbk = WBK({
      instance: 'https://www.wikidata.org',
      sparqlEndpoint: 'https://query.wikidata.org/sparql',
    })

    const languages = await this.sendMessageToClient('languages');
    if (!Array.isArray(languages.value) || languages.value.length === 0) {
      console.log('No languages! Are you authenticated already?');
      this.quit();
    }

    this.languages = languages.value;

    this.oldData =
        JSON.parse(fs.readFileSync(LANGUAGES_OUTPUT_FILE).toString()) || [];

    for (const language of languages.value) {
      await this.localizeLanguage(language);
    }

    this.writeData();
    console.log(chalk.green('Languages known file written.'));

    this.quit();
  },

  writeData() {
    console.log(chalk.yellow('Sorting data...'));

    for (let language of this.newData) {
      if ('translations' in language) {
        language.translations =
            Object.keys(language.translations).sort().reduce((obj, key) => {
              obj[key] = language.translations[key];
              return obj;
            }, {});
      }
    }

    this.newData.sort((a, b) => a.languageCode < b.languageCode ? -1 : 1);

    fs.writeFileSync(
        LANGUAGES_OUTPUT_FILE, JSON.stringify(this.newData, null, '  '));
  },

  async localizeLanguage(language) {
    console.log(`  - Localizing language ${chalk.yellow(language)}...`);
    let languageData = this.oldData.find(s => s.languageCode === language);
    if (!languageData) {
      console.log('    Unknown language!');
      languageData = {
        wikiDataID: null,
        languageCode: language,
        translations: {},
      }
    } else if (!languageData.wikiDataID) {
      console.log(
          '    Language found but we do not have a valid wikiDataID. Please update the file!');
      languageData.wikiDataID = null;
      languageData.translations = {};
    }

    if (!languageData.wikiDataID) {
      while (true) {
        let answer = await inquirer.prompt([{
          type: 'confirm',
          name: 'value',
          message: 'No WikiData ID found. Do you know it?',
        }]);

        if (answer.value) {
          answer = await inquirer.prompt([{
            name: 'value',
            message: 'WikiData ID:',
          }]);

          if (answer.value != '') {
            languageData.wikiDataID = answer.value;
            break;
          }

          continue;
        }

        answer = await inquirer.prompt([{
          name: 'value',
          message: `Alternative WikiData ID: ${
              languageData.alternativeWikiDataID ?
                  '(' + languageData.alternativeWikiDataID + ')' :
                  ''}:`
        }]);

        if (answer.value != '') {
          languageData.alternativeWikiDataID = answer.value;
          break;
        }
      }
    }

    const sparql = `SELECT ?languageName (lang(?languageName) as ?lang)
                    WHERE { <http://www.wikidata.org/entity/${
        languageData.wikiDataID ||
        languageData.alternativeWikiDataID}> rdfs:label ?languageName . }`;
    const url = this.wbk.sparqlQuery(sparql)
    const result = await this.retrieveJson(url);

    if (!('head' in result)) throw new Error('Invalid SPARQL result (no head)');
    if (!('results' in result))
      throw new Error('Invalid SPARQL result (no results)');
    if (!('bindings' in result.results))
      throw new Error('Invalid SPARQL result (no results/bindings)');

    if (result.results.bindings.length === 0) {
      console.log(sparql);
      throw new Error('No results');
    }

    const translations = {};
    for (let lang of result.results.bindings) {
      const langCode = lang.languageName['xml:lang'];
      let value = lang.languageName['value'];
      value = value.charAt(0).toUpperCase() + value.slice(1);

      if (this.languages.includes(langCode)) {
        translations[langCode] = value;
      }

      for (let lang of this.languages) {
        if (lang.toLowerCase().replace('_', '-') === langCode) {
          translations[lang] = value;
        }
        if (lang.toLowerCase().split('_')[0] === langCode &&
            !translations[langCode]) {
          translations[langCode] = value;
        }
      }
    }

    languageData.translations = translations;
    this.newData.push(languageData);
  },

  languages: new Map(),
  newData: [],
  oldData: [],
};

LanguageLocalizer.init();
