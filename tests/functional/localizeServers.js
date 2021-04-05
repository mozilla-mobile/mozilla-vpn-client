/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const util = require('util');
const vpn = require('./helper.js');
const FirefoxHelper = require('./firefox.js');
const fetch = require('node-fetch');
const WBK = require('wikibase-sdk')

const webdriver = require('selenium-webdriver'), By = webdriver.By,
      Keys = webdriver.Key, until = webdriver.until;

const exec = util.promisify(require('child_process').exec);

describe('Server list', function() {
  let driver;
  let servers;
  let currentCountryCode;
  let currentCity;
  let wbk;
  let translations = [];
  let languages = new Map();

  const cityNames = new Map();
  cityNames.set('Dallas, TX', 'Dallas, Tx.');
  cityNames.set('Denver, CO', 'Denver, Colorado');
  cityNames.set('Miami, FL', 'Miami');
  cityNames.set('Phoenix, AZ', 'Phoenix, Arizona');
  cityNames.set('Salt Lake City, UT', 'Salt Lake City, Utah');

  this.timeout(1000000);

  function retrieveJson(url) {
    return fetch(url, {
             headers: {
               'User-Agent':
                   'Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:86.0) Gecko/20100101 Firefox/86.0',
               'Accept':
                   'text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8',
             }
           })
        .then(r => r.json());
  }

  before(async () => {
    await vpn.connect();
    driver = await FirefoxHelper.createDriver();
    wbk = WBK({
      instance: 'https://www.wikidata.org',
      sparqlEndpoint: 'https://query.wikidata.org/sparql',
    })
  });

  beforeEach(() => {});

  afterEach(() => {});

  after(async () => {
    await driver.quit();
    vpn.disconnect();
  });

  it('authenticate', async () => await vpn.authenticate(driver));

  it('Post authentication view', async () => {
    await vpn.waitForElement('postAuthenticationButton');
    await vpn.clickOnElement('postAuthenticationButton');
    await vpn.wait();
  });

  it('Retrieve the list of languages', async () => {
    for (let language of await vpn.languages()) {
      languages.set(language.toLowerCase().replace('_', '-'), language);
    }
  });

  it('retrieve list of servers and the current one', async () => {
    servers = await vpn.servers();
    currentCountryCode = await vpn.getSetting('current-server-country-code');
    currentCity = await vpn.getSetting('current-server-city');
  });

  it('localize countries and cities', async () => {
    for (let server of servers) {
      const translation = await localizeCountry(server);
      if (translation) translations.push(translation);
    }

    process.stderr.write(JSON.stringify(translations));
  });

  async function localizeCountry(server) {
    console.log('Localize country', server.name);

    const sparql = `SELECT ?country ?countryName (lang(?countryName) as ?lang) 
                    WHERE
                    {
                      # wdt:P31 is "instance-of"
                      # wd:Q6256 is "country"
                      { ?country wdt:P31 wd:Q6256 ;
                                 rdfs:label ?countryName .
                                { ?country rdfs:label "${
        server.name}"@en . } UNION { ?country skos:altLabel "${
        server.name}"@en . } }
                      UNION
                      { ?country wdt:P31 wd:Q3624078 ;
                                 rdfs:label ?countryName ;
                                { ?country rdfs:label "${
        server.name}"@en . } UNION { ?country skos:altLabel "${
        server.name}"@en . } }
                      UNION
                      # wd:Q515 is "city"
                      { ?country wdt:P31 wd:Q515 ;
                                 rdfs:label ?countryName ;
                                { ?country rdfs:label "${
        server.name}"@en . } UNION { ?country skos:altLabel "${
        server.name}"@en . } }
                    }`;

    const url = wbk.sparqlQuery(sparql)
    const result = await retrieveJson(url);

    if (!('head' in result)) throw new Error('Invalid SPARQL result (no head)');
    if (!('results' in result))
      throw new Error('Invalid SPARQL result (no results)');
    if (!('bindings' in result.results))
      throw new Error('Invalid SPARQL result (no results/bindings)');

    if (result.results.bindings.length === 0) {
      throw new Error('No results');
    }

    const countryUrl = result.results.bindings[0].country.value;

    const translation = {countryCode: server.code, languages: {}, cities: []};

    for (let lang of result.results.bindings) {
      const langCode = lang.countryName['xml:lang'];
      const value = lang.countryName['value'];
      if (languages.has(langCode) && value != server.name) {
        translation.languages[languages.get(langCode)] = value;
      }
    }

    for (let city of server.cities) {
      if (city.name === server.name) {
        continue;
      }

      const cityData = await localizeCity(countryUrl, city);
      if (cityData) translation.cities.push(cityData);
    }

    return translation;
  }

  async function localizeCity(countryUrl, city) {
    console.log(' - Localize city', city.name);

    const translation = {city: city.name, languages: {}};

    let cityName = city.name;
    if (cityNames.has(cityName)) {
      cityName = cityNames.get(cityName);
    }

    const sparql = `SELECT ?cityName (lang(?cityName) as ?lang) 
                    WHERE
                    {
                      # wdt:P31 is "instance-of"
                      # wd:Q515 is "city"
                      { ?city wdt:P31 wd:Q515 ;
                              wdt:P17 <${countryUrl}> ;
                              rdfs:label ?cityName .
                              { ?city rdfs:label "${
        cityName}"@en . } UNION { ?city skos:altLabel "${cityName}"@en . } }
                      UNION
                      { ?city wdt:P31 ?bigcity ;
                              wdt:P17 <${countryUrl}> ;
                              rdfs:label ?cityName ;
                              { ?city rdfs:label "${
        cityName}"@en . } UNION { ?city skos:altLabel "${cityName}"@en . }
                        ?bigcity wdt:P279 wd:Q515 . }
                      UNION
                      { ?city wdt:P31 ?metropolis ;
                              wdt:P17 <${countryUrl}> ;
                              rdfs:label ?cityName ;
                              { ?city rdfs:label "${
        cityName}"@en . } UNION { ?city skos:altLabel "${cityName}"@en . }
                        ?metropolis wdt:P279 ?bigcity .
                        ?bigcity wdt:P279 wd:Q515 . }
    }`;

    const url = wbk.sparqlQuery(sparql)
    const result = await retrieveJson(url);

    if (!('head' in result)) throw new Error('Invalid SPARQL result (no head)');
    if (!('results' in result))
      throw new Error('Invalid SPARQL result (no results)');
    if (!('bindings' in result.results))
      throw new Error('Invalid SPARQL result (no results/bindings)');

    if (result.results.bindings.length === 0) {
      throw new Error('No results');
    }

    for (let lang of result.results.bindings) {
      const langCode = lang.cityName['xml:lang'];
      const value = lang.cityName['value'];
      if (languages.has(langCode) && value != city.name) {
        translation.languages[languages.get(langCode)] = lang.cityName['value'];
      }
    }

    return translation;
  }

  it('Logout', async () => {
    await vpn.logout();
    await vpn.wait();
  });

  it('quit the app', async () => await vpn.quit());
});
