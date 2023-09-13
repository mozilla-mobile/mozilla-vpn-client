/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import chalk from 'chalk';
import {program} from 'commander';
import fs from 'fs';
import { fileURLToPath } from 'node:url';
import path from 'path'; 
import inquirer from 'inquirer';
import fetch from 'node-fetch';
import WBK from 'wikibase-sdk';

const I18N_SUBMODULE_PATH = '../../src/translations/i18n';
const LANGUAGES_OUTPUT_FILE =
    '../../src/translations/extras/languages.json';

const LanguageLocalizer = {

  quit() {

    process.exit();
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
    program.option("-c, --check", `
      Check only, will fail if user input would be required to generate 
      the file.
    `);

    program.action(async () => await this.run(program.opts()));

    program.parseAsync(process.argv);
  },

  async run(options) {
    this.options = options;
    if(options.check){
      console.log("Running Checks Only");
    }
    this.wbk = WBK({
      instance: 'https://www.wikidata.org',
      sparqlEndpoint: 'https://query.wikidata.org/sparql',
    })
    const __filename = fileURLToPath(import.meta.url);
    const __dirname = path.dirname(__filename);
    const submodule_path = path.resolve(__dirname, I18N_SUBMODULE_PATH)
    const dir_entries = fs.readdirSync(submodule_path, {withFileTypes: true});
    const language_folders = dir_entries
                              .filter(f => f.isDirectory())
                              .map(f=>f.name)
                              .filter(f=>!f.startsWith("."));

    this.languages = language_folders;


    this.oldData =
        JSON.parse(fs.readFileSync(LANGUAGES_OUTPUT_FILE).toString()) || [];

    for (const language of this.languages) {
      await this.localizeLanguage(language);
    }

    // `localizeLanguage` creates a `translations` object with all the language
    // translations in language X using Wikidata as a data source (via SPARQL
    // queries).  Now we use CLDR which will give us X translated in all the
    // languages in an object called `languages`. We need to normalize these 2
    // objects and then drop the `translations`. Wikidata is used fallback.
    for (let language of this.newData) {
      await this.cldrLanguage(language);
    }

    this.writeData();
    console.log(chalk.green('Languages known file written.'));

    this.quit();
  },

  writeData() {
    if(this.options.check){
      console.log(chalk.green('In check mode, skipping write'));
      return;
    }
    console.log(chalk.yellow('Sorting data...'));

    for (let language of this.newData) {
      delete language.translations;
      const old_entry = this.oldData.find(e => e.languageCode === language.languageCode)

      if ('currencies' in language) {
        language.currencies =
            Object.keys(language.currencies).sort().reduce((obj, key) => {
              obj[key] = language.currencies[key];
              return obj;
            }, {});
      }

      if ('languages' in language) {
        if( old_entry && 'languages' in old_entry){
          // Check if we the new languages object lost language data, if so
          // put it back. 
          const old_languages = Object.keys(old_entry.languages);
          const new_languages = Object.keys(language.languages);
          // List of keys we no longer have in the new lang obj i.e ["bg","cy"] 
          const lost_languages = old_languages.filter( l => !new_languages.includes(l));
          lost_languages.forEach(lost_language_code =>{
            language.languages[lost_language_code] = old_entry.languages[lost_language_code];
          });
        }
        language.languages =
            Object.keys(language.languages).sort().reduce((obj, key) => {
              obj[key] = language.languages[key];
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
    const language_old_Data = this.oldData.find(s => s.languageCode === language);
    // Create a copy, otherwise we modify our old state
    let languageData = structuredClone(language_old_Data);
    if (!languageData) {
      if(this.options.check){
        console.error(`Unknown language ${language} - cannot continue`);
        console.error(`Check out this branch and run "npm run languages:update" to update languages.json`);
        process.exit(1);
      }
      console.log('    Unknown language!');
      languageData = {
        wikiDataID: null,
        languageCode: language,
        currencies: {},
        languages: {},
      }
    } else if (!languageData.wikiDataID) {
      console.log(
          '    Language found but we do not have a valid wikiDataID. Please update the file!');
      if(this.options.check){
            console.log(JSON.stringify(languageData))
            console.error(`${ languageData.languageCode } - No Wikidata ID found.`)
            console.error(`This is likely a new language. Check this branch out and run this`)
            console.error(`tool locally again!`)
            process.exit(1);
      }
      languageData.wikiDataID = null;
      languageData.currencies = {};
      languageData.languages = {};
    }

    // This is a temporary object to unify Wikidata and CLDR values
    languageData.translations = {};

    if (!languageData.wikiDataID) {
      while (true) {
        let answer = await inquirer.prompt([{
          type: 'confirm',
          name: 'value',
          message: `${ languageData.languageCode } - No Wikidata ID found. Do you know it?`,
        }]);

        if (answer.value) {
          answer = await inquirer.prompt([{
            name: 'value',
            message: 'Wikidata ID:',
          }]);

          if (answer.value != '') {
            languageData.wikiDataID = answer.value;
            break;
          }

          continue;
        }

        answer = await inquirer.prompt([{
          name: 'value',
          message: `Alternative Wikidata ID: ${
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

    if (!languageData.IETFcode && !this.options?.check) {
      while (true) {
        let answer = await inquirer.prompt([{
          type: 'confirm',
          name: 'value',
          message: `${ languageData.languageCode } - No IETF ID found. Do you know it?`,
        }]);

        if (!answer.value) {
          break;
        }

        answer = await inquirer.prompt([{
          name: 'value',
          message: 'IETF language code:',
        }]);

        if (answer.value != '') {
          languageData.IETFcode = answer.value;
          break;
        }

        continue;
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

    const currencies = {};
    if (languageData.IETFcode) {
      const currencyData = await this.retrieveJson(
          `https://raw.githubusercontent.com/unicode-org/cldr-json/main/cldr-json/cldr-numbers-full/main/${
              languageData.IETFcode}/currencies.json`);
      Object.keys(currencyData.main[languageData.IETFcode].numbers.currencies)
          .map(c => ({
                 c,
                 s: currencyData.main[languageData.IETFcode]
                        .numbers.currencies[c]
                        .symbol
               }))
          .filter(a => a.c != a.s)
          .forEach(a => currencies[a.c] = a.s);
    }

    languageData.translations = translations;
    languageData.currencies = currencies;
    this.newData.push(languageData);
  },

  async cldrLanguage(languageData) {
    console.log(`  - Fetching CLDR language values for language ${
        chalk.yellow(languageData.languageCode)}...`);
    const languages = {};

    if (languageData.IETFcode) {
      const ietfToCode =
          this.newData.filter(a => 'IETFcode' in a)
              .map(a => ({languageCode: a.languageCode, IETFcode: a.IETFcode}));

      const cldrLanguages =
          (await this.retrieveJson(
               `https://raw.githubusercontent.com/unicode-org/cldr-json/main/cldr-json/cldr-localenames-full/main/${
                   languageData.IETFcode}/languages.json`))
              .main[languageData.IETFcode]
              .localeDisplayNames.languages;

      Object.keys(cldrLanguages)
          .filter(l => !!ietfToCode.find(ietf => ietf.IETFcode === l))
          .forEach(
              code => languages[ietfToCode.find(ietf => ietf.IETFcode === code)
                                    .languageCode] = cldrLanguages[code]);
    }

    // Let's add languages from Wikidata if CLDR does not have them yet.
    this.newData.filter(l => languageData.languageCode in l.translations)
        .filter(l => !(languageData.languageCode in languages))
        .forEach(
            l => languages[l.languageCode] =
                l.translations[languageData.languageCode]);

    languageData.languages = languages;
  },

  languages: new Map(),
  newData: [],
  oldData: [],
};

LanguageLocalizer.init();
