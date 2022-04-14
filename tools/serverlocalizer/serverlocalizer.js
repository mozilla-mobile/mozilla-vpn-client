/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import chalk from 'chalk';
import child_process from 'child_process';
import fetch from 'node-fetch';
import fs from 'fs';
import inquirer from 'inquirer';
import program from 'commander';
import websocket from 'websocket';
import WBK from 'wikibase-sdk';

const DEFAULT_MOZILLAVPN = '../../src/mozillavpn';
const SERVERS_OUTPUT_FILE = '../../translations/servers.json';

const ServerLocalizer = {
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
    program.action(
        async (emailAddress, password) => await this.run(program.opts()));

    program.parseAsync(process.argv);
  },

  async run(option) {
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

    for (let language of languages.value) {
      this.languages.set(language.toLowerCase().replace('_', '-'), language);
    }

    const servers = await this.sendMessageToClient('servers');
    if (!Array.isArray(servers.value) || servers.value.length === 0) {
      console.log('No servers! Are you authenticated already?');
      this.quit();
    }

    this.oldData =
        JSON.parse(fs.readFileSync(SERVERS_OUTPUT_FILE).toString()) || [];

    for (const server of servers.value) {
      await this.localizeServer(server);
    }

    this.writeData();
    console.log(chalk.green('Servers known file written.'));

    this.quit();
  },

  writeData() {
    console.log(chalk.yellow('Sorting data...'));

    for (let country of this.newData) {
      if ('languages' in country) {
        country.languages =
            Object.keys(country.languages).sort().reduce((obj, key) => {
              obj[key] = country.languages[key];
              return obj;
            }, {});
      }

      if (Array.isArray(country.cities)) {
        for (let city of country.cities) {
          if ('languages' in city) {
            city.languages =
                Object.keys(city.languages).sort().reduce((obj, key) => {
                  obj[key] = city.languages[key];
                  return obj;
                }, {});
          }
        }
      }
    }

    this.newData.sort((a, b) => {return a.code < b.code});

    fs.writeFileSync(
        SERVERS_OUTPUT_FILE, JSON.stringify(this.newData, null, ' '));
  },

  async localizeServer(server) {
    console.log(`  - Localizing country ${chalk.yellow(server.name)}...`);
    let serverData = this.oldData.find(s => s.countryCode === server.code);
    if (!serverData) {
      console.log('    Unknown country!');
      serverData = {
        wikiDataID: null,
        countryCode: server.code,
        languages: {},
      }
    } else if (!serverData.wikiDataID) {
      console.log(
          '    Server found but we do not have a valid wikiDataID. Please update the file!');
      serverData.wikiDataID = null;
      serverData.languages = {};
    }

    if (!serverData.wikiDataID) {
      while (true) {
        const answer = await inquirer.prompt([{
          name: 'value',
          message: 'WikiData ID:',
        }]);

        if (answer.value != '') {
          serverData.wikiDataID = answer.value;
          break;
        }
      }
    }

    const sparql = `SELECT ?countryName (lang(?countryName) as ?lang)
                    WHERE { <http://www.wikidata.org/entity/${
        serverData.wikiDataID}> rdfs:label ?countryName . }`;
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
      const langCode = lang.countryName['xml:lang'];
      const value = lang.countryName['value'];

      if (value === server.name) continue;

      if (this.languages.has(langCode)) {
        translations[this.languages.get(langCode)] = value;
      }

      for (let language of this.languages) {
        if (langCode === language[0]) continue;
        if (translations[language[1]]) continue;
        if (!language[0].startsWith(langCode + '-')) continue;
        translations[language[1]] = value;
      }
    }

    serverData.languages = translations;

    serverData.cities = await this.localizeCities(server, serverData);
    this.newData.push(serverData);
  },

  async localizeCities(server, serverData) {
    const cities = [];
    for (const city of server.cities) {
      cities.push(await this.localizeCity(city.name, serverData));
    }
    return cities;
  },

  async localizeCity(name, serverData) {
    console.log(`    - Localizing city ${chalk.yellow(name)}...`);
    let cityData = serverData.cities.find(c => c.city === name);
    if (!cityData) {
      console.log('      Unknown country!');
      cityData = {
        wikiDataID: null,
        city: name,
        languages: {},
      }
    } else if (!cityData.wikiDataID) {
      console.log(
          '      City found but we do not have a valid wikiDataID. Please update the file!');
      cityData.wikiDataID = null;
      cityData.languages = {};
    }

    if (!cityData.wikiDataID) {
      while (true) {
        const answer = await inquirer.prompt([{
          name: 'value',
          message: 'WikiData ID:',
        }]);

        if (answer.value != '') {
          cityData.wikiDataID = answer.value;
          break;
        }
      }
    }

    const translations = {};

    const sparql = `SELECT ?cityName (lang(?cityName) as ?lang)
                    WHERE { <http://www.wikidata.org/entity/${
        cityData.wikiDataID}> rdfs:label ?cityName . }`;

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

    for (let lang of result.results.bindings) {
      const langCode = lang.cityName['xml:lang'];
      const value = lang.cityName['value'];

      if (name === value) continue;

      if (this.languages.has(langCode)) {
        translations[this.languages.get(langCode)] = value;
      }

      for (let language of this.languages) {
        if (langCode === language[0]) continue;
        if (translations[language[1]]) continue;
        if (!language[0].startsWith(langCode + '-')) continue;
        translations[language[1]] = value;
      }
    }

    cityData.languages = translations;
    return cityData;
  },

  languages: new Map(),
  newData: [],
  oldData: [],
};

ServerLocalizer.init();
