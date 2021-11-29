/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const util = require('util');
const vpn = require('./helper.js');
const FirefoxHelper = require('./firefox.js');
const fetch = require('node-fetch');
const WBK = require('wikibase-sdk');
const fs = require('fs');

const exec = util.promisify(require('child_process').exec);

describe('Server list', function() {
  let servers;
  let currentCountryCode;
  let currentCity;
  let wbk;
  let translations = [];
  let languages = new Map();

  let serverOutputFile;
  let serverApiFile;
  let serverTemplateFile;

  const countryIDs = new Map();
  countryIDs.set('Netherlands', 'http://www.wikidata.org/entity/Q55');
  countryIDs.set('Luxembourg', 'http://www.wikidata.org/entity/Q32');
  countryIDs.set('Germany', 'http://www.wikidata.org/entity/Q183');

  const cityIDs = new Map();
  cityIDs.set('Frankfurt', 'http://www.wikidata.org/entity/Q1794');
  cityIDs.set('Melbourne', 'http://www.wikidata.org/entity/Q3141');
  cityIDs.set('Dallas, TX', 'http://www.wikidata.org/entity/Q16557');
  cityIDs.set('Denver, CO', 'http://www.wikidata.org/entity/Q16554');
  cityIDs.set('Miami, FL', 'http://www.wikidata.org/entity/Q8652');
  cityIDs.set('Phoenix, AZ', 'http://www.wikidata.org/entity/Q16556');
  cityIDs.set('Salt Lake City, UT', 'http://www.wikidata.org/entity/Q23337');
  cityIDs.set('Secaucus, NJ', 'http://www.wikidata.org/entity/Q1013249');
  cityIDs.set('Sydney', 'http://www.wikidata.org/entity/Q3130');
  cityIDs.set('Vancouver', 'http://www.wikidata.org/entity/Q24639');
  cityIDs.set('Copenhagen', 'http://www.wikidata.org/entity/Q1748');
  cityIDs.set('Dublin', 'http://www.wikidata.org/entity/Q1761');
  cityIDs.set('Amsterdam', 'http://www.wikidata.org/entity/Q727');
  cityIDs.set('Oslo', 'http://www.wikidata.org/entity/Q585');
  cityIDs.set('Stockholm', 'http://www.wikidata.org/entity/Q1754');
  cityIDs.set('Luxembourg', 'http://www.wikidata.org/entity/Q1842');
  cityIDs.set('Paris', 'http://www.wikidata.org/entity/Q90');
  cityIDs.set('Dusseldorf', 'http://www.wikidata.org/entity/Q1718');
  cityIDs.set('Malmö', 'http://www.wikidata.org/entity/Q2211');

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
    wbk = WBK({
      instance: 'https://www.wikidata.org',
      sparqlEndpoint: 'https://query.wikidata.org/sparql',
    })
  });

  beforeEach(() => {});

  afterEach(() => {});

  after(async () => {
    vpn.disconnect();
  });

  it('validate env', async () => {
    serverOutputFile = process.env.SERVER_OUTPUT;
    assert(!!serverOutputFile);

    serverApiFile = process.env.SERVER_API;
    assert(!!serverApiFile);

    serverTemplateFile = process.env.SERVER_TEMPLATE;
    assert(!!serverTemplateFile);
  });

  it('authenticate', async () => await vpn.authenticate());

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
  });

  it('write files', async () => {
    console.log('Write the API localized file');
    fs.writeFileSync(serverApiFile, JSON.stringify(translations, null, ' '));

    console.log('Merge the template file');
    const template = JSON.parse(fs.readFileSync(serverTemplateFile));
    assert(Array.isArray(template));

    for (let country of template) {
      mergeCountry(country);
    }

    console.log('Sorting data');
    for (let country of translations) {
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

    console.log('Merge the final file');
    fs.writeFileSync(serverOutputFile, JSON.stringify(translations, null, ' '));
  });

  function mergeCountry(country) {
    assert(country.countryCode);

    for (let c of translations) {
      if (c.countryCode === country.countryCode) {
        return mergeCountryReal(c, country);
      }
    }

    console.log('Adding country', country.countryCode);
    translations.push(country);
  }

  function mergeCountryReal(country, template) {
    console.log('Merge country', template.countryCode);

    if ('languages' in template) {
      for (let language of Object.keys(template.languages)) {
        country.languages[language] = template.languages[language];
      }
    }

    if (Array.isArray(template.cities)) {
      for (let city of template.cities) {
        mergeCity(country, city);
      }
    }
  }

  function mergeCity(country, template) {
    assert(template.city);
    for (let c of country.cities) {
      if (c.city === template.city) {
        return mergeCityReal(c, template);
      }
    }

    console.log('Adding city', template.city);
    country.cities.push(template);
  }

  function mergeCityReal(city, template) {
    console.log('Merge city', template.city);

    if ('languages' in template) {
      for (let language of Object.keys(template.languages)) {
        city.languages[language] = template.languages[language];
      }
    }
  }

  async function localizeCountry(server) {
    console.log('Localize country', server.name);

    let countryUrl;
    if (countryIDs.has(server.name)) {
      countryUrl = countryIDs.get(server.name);
    } else {
      const sparql = `SELECT ?country
                      WHERE
                      {
                        # wdt:P31 is "instance-of"
                        # wd:Q6256 is "country"
                        { ?country wdt:P31 wd:Q6256 ;
                                  { ?country rdfs:label "${
          server.name}"@en . } UNION { ?country skos:altLabel "${
          server.name}"@en . } }
                        UNION
                        { ?country wdt:P31 wd:Q3624078 ;
                                  { ?country rdfs:label "${
          server.name}"@en . } UNION { ?country skos:altLabel "${
          server.name}"@en . } }
                        UNION
                        # wd:Q515 is "city"
                        { ?country wdt:P31 wd:Q515 ;
                                  { ?country rdfs:label "${
          server.name}"@en . } UNION { ?country skos:altLabel "${
          server.name}"@en . } }
                      }`;

      const url = wbk.sparqlQuery(sparql)
      const result = await retrieveJson(url);

      if (!('head' in result))
        throw new Error('Invalid SPARQL result (no head)');
      if (!('results' in result))
        throw new Error('Invalid SPARQL result (no results)');
      if (!('bindings' in result.results))
        throw new Error('Invalid SPARQL result (no results/bindings)');

      if (result.results.bindings.length === 0) {
        console.log(sparql);
        throw new Error('No results');
      }

      countryUrl = result.results.bindings[0].country.value;
    }

    console.log(' - WikiData Resource:', countryUrl);

    const sparql = `SELECT ?countryName (lang(?countryName) as ?lang) 
                    WHERE { <${countryUrl}> rdfs:label ?countryName . }`;
    const url = wbk.sparqlQuery(sparql)
    const result = await retrieveJson(url);

    if (!('head' in result)) throw new Error('Invalid SPARQL result (no head)');
    if (!('results' in result))
      throw new Error('Invalid SPARQL result (no results)');
    if (!('bindings' in result.results))
      throw new Error('Invalid SPARQL result (no results/bindings)');

    if (result.results.bindings.length === 0) {
      console.log(sparql);
      throw new Error('No results');
    }

    const translation = {countryCode: server.code, languages: {}, cities: []};

    for (let lang of result.results.bindings) {
      const langCode = lang.countryName['xml:lang'];
      const value = lang.countryName['value'];

      if (value === server.name) continue;

      if (languages.has(langCode)) {
        translation.languages[languages.get(langCode)] = value;
      }

      for (let language of languages) {
        if (langCode === language[0]) continue;
        if (translation.languages[language[1]]) continue;
        if (!language[0].startsWith(langCode + '-')) continue;
        translation.languages[language[1]] = value;
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

    let cityUrl;
    if (cityIDs.has(city.name)) {
      cityUrl = cityIDs.get(city.name);
    } else {
      const sparql = `SELECT ?city
                      WHERE
                      {
                        # wdt:P31 is "instance-of"
                        # wd:Q515 is "city"
                        # wd:Q200250 is "metropolis"
                        # wd:Q5119 is "capital"
                        { ?city wdt:P31 wd:Q515 ;
                                wdt:P17 <${countryUrl}> ;
                                { ?city rdfs:label "${
          city.name}"@en . } UNION { ?city skos:altLabel "${city.name}"@en . } }
                        UNION
                        { ?city wdt:P31 ?bigcity ;
                                wdt:P17 <${countryUrl}> ;
                                { ?city rdfs:label "${
          city.name}"@en . } UNION { ?city skos:altLabel "${city.name}"@en . }
                          ?bigcity wdt:P279 wd:Q515 . }
                        UNION
                        { ?city wdt:P31 ?metropolis ;
                                wdt:P17 <${countryUrl}> ;
                                { ?city rdfs:label "${
          city.name}"@en . } UNION { ?city skos:altLabel "${city.name}"@en . }
                          ?metropolis wdt:P279 ?bigcity .
                          ?bigcity wdt:P279 wd:Q515 . }
                        UNION
                        { ?city wdt:P31 wd:Q200250 ;
                                wdt:P17 <${countryUrl}> ;
                                { ?city rdfs:label "${
          city.name}"@en . } UNION { ?city skos:altLabel "${city.name}"@en . } }
                        UNION
                        { ?city wdt:P31 wd:Q5119 ;
                                wdt:P17 <${countryUrl}> ;
                                { ?city rdfs:label "${
          city.name}"@en . } UNION { ?city skos:altLabel "${city.name}"@en . } }
                      }`;

      const url = wbk.sparqlQuery(sparql)
      const result = await retrieveJson(url);

      if (!('head' in result))
        throw new Error('Invalid SPARQL result (no head)');
      if (!('results' in result))
        throw new Error('Invalid SPARQL result (no results)');
      if (!('bindings' in result.results))
        throw new Error('Invalid SPARQL result (no results/bindings)');

      if (result.results.bindings.length === 0) {
        console.log(sparql);
        throw new Error('No results');
      }

      cityUrl = result.results.bindings[0].city.value;
    }

    console.log(' - WikiData Resource:', cityUrl);

    const sparql = `SELECT ?cityName (lang(?cityName) as ?lang) 
                    WHERE { <${cityUrl}> rdfs:label ?cityName . }`;

    const url = wbk.sparqlQuery(sparql)
    const result = await retrieveJson(url);

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

      if (city.name === value) continue;

      if (languages.has(langCode)) {
        translation.languages[languages.get(langCode)] = value;
      }

      for (let language of languages) {
        if (langCode === language[0]) continue;
        if (translation.languages[language[1]]) continue;
        if (!language[0].startsWith(langCode + '-')) continue;
        translation.languages[language[1]] = value;
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
