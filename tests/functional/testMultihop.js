/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

 const assert = require('assert');
 const { actions } = require('./actions.js');
 const elements = require('./elements.js');
 const vpn = require('./helper.js');

describe('Server list', function() {
   let servers;
   let currentCountryCode;
   let currentCity;
   let currentCountry;
 
   this.timeout(240000);
   this.ctx.authenticationNeeded = true;
 
  beforeEach(async () => {
     await vpn.waitForElementAndClick(elements.SERVER_LIST_BUTTON);
 
     servers = await vpn.servers();
     currentCountryCode = await vpn.getSetting('current-server-country-code');
     currentCity = await vpn.getSetting('current-server-city');
 
     for (let server of servers) {
       if (currentCountryCode === server.code) {
         for (let city of server.cities) {
           if (city.name == currentCity) {
             currentCity = city.localizedName;
             break;
           }
         }
       }
     }
     console.log(
         'Current city (localized):', currentCity,
         '| Current country code:', currentCountryCode);
  });

  it('opening the entry and exit server list', async () => {
    await vpn.waitForElementAndClick(elements.MULTIHOP_SELECTOR_TAB);
    assert(await vpn.getElementProperty(elements.VPN_COLLAPSIBLE_CARD, 'expanded') === 'false')

    await vpn.waitForElement(elements.SERVER_ENTRY_BUTTON);
    await vpn.waitForElementProperty(elements.SERVER_ENTRY_BUTTON, 'visible', 'true');

    await vpn.waitForElement(elements.SERVER_EXIT_BUTTON);
    await vpn.waitForElementProperty(elements.SERVER_EXIT_BUTTON, 'visible', 'true'); 
    
    await vpn.waitForElementAndClick(elements.VPN_MULTHOP_CHEVRON)
    assert(await vpn.getElementProperty(elements.VPN_COLLAPSIBLE_CARD, 'expanded'))
  });

  it('check the countries and cities for multihop entries', async () => {
    await vpn.waitForElementAndClick(elements.MULTIHOP_SELECTOR_TAB);    

    await vpn.waitForElementAndClick(elements.SERVER_ENTRY_BUTTON);

    for (let server of servers) {
      const countryId = 'serverCountryList/serverCountry-' + server.code;
      await vpn.waitForElement(countryId);
      await vpn.waitForElementProperty(countryId, 'visible', 'true');

      await vpn.setElementProperty(
        elements.SERVER_COUNTRY_VIEW, 'contentY', 'i',
          parseInt(await vpn.getElementProperty(countryId, 'y')));
      await vpn.wait();

      if (currentCountryCode === server.code) {
        assert(
            await vpn.getElementProperty(countryId, elements.CITYLIST_VISIBLE) ===
            'true');
      }

      if (await vpn.getElementProperty(countryId, elements.CITYLIST_VISIBLE) ===
          'false') {
        await vpn.clickOnElement(countryId);
      }

      for (let city of server.cities) {
        const cityId = countryId + '/serverCityList/serverCity-' +
            city.name.replace(/ /g, '_');
      
        await vpn.waitForElement(cityId);
        await vpn.waitForElementProperty(cityId, 'visible', 'true');
      }
    }
  })

  it('check the countries and cities for multihop exits', async () => {
    await vpn.waitForElementAndClick(elements.MULTIHOP_SELECTOR_TAB);    

    await vpn.waitForElementAndClick(elements.SERVER_EXIT_BUTTON);

    for (let server of servers) {
      const countryId = 'serverCountryList/serverCountry-' + server.code;

      await vpn.waitForElement(countryId);
      await vpn.waitForElementProperty(countryId, 'visible', 'true');

      await vpn.setElementProperty(
          elements.SERVER_COUNTRY_VIEW, 'contentY', 'i',
          parseInt(await vpn.getElementProperty(countryId, 'y')));
      await vpn.wait();

      if (currentCountryCode === server.code) {
        assert(
            await vpn.getElementProperty(countryId, elements.CITYLIST_VISIBLE) ===
            'true');
      }      

      if (await vpn.getElementProperty(countryId, elements.CITYLIST_VISIBLE) ===
          'false') {
        await vpn.clickOnElement(countryId);
      }
      
      for (let city of server.cities) {
        const cityId = countryId + '/serverCityList/serverCity-' +
            city.name.replace(/ /g, '_');

        await vpn.waitForElement(cityId);
        await vpn.waitForElementProperty(cityId, 'visible', 'true');
        await vpn.waitForElementProperty(
            cityId, 'checked',
            currentCountryCode === server.code &&
                    currentCity === city.localizedName ?
                'true' :
                'false');
      }
    }
  })

  it('Pick cities for entries', async () => {
      let countryId;      
      await vpn.waitForElementAndClick(elements.MULTIHOP_SELECTOR_TAB);
      await vpn.waitForElementAndClick(elements.SERVER_ENTRY_BUTTON);         
      
      for (let server of servers) {        
        countryId = 'serverCountryList/serverCountry-' + server.code;
        await vpn.waitForElement(countryId);

        await vpn.setElementProperty(
            elements.SERVER_COUNTRY_VIEW, 'contentY', 'i',
            parseInt(await vpn.getElementProperty(countryId, 'y')));
        await vpn.wait();
        
        if (await vpn.getElementProperty(countryId, elements.CITYLIST_VISIBLE) === 'false') {
          await vpn.clickOnElement(countryId);
        }
        
        await vpn.waitForElementProperty(countryId, elements.CITYLIST_VISIBLE, 'true');       

        for (let city of server.cities) {
          const cityId = countryId + '/serverCityList/serverCity-' + city.name.replace(/ /g, '_');
          await vpn.waitForElement(cityId);

          await vpn.setElementProperty(
              'serverCountryView', 'contentY', 'i',
              parseInt(await vpn.getElementProperty(cityId, 'y')) +
                  parseInt(await vpn.getElementProperty(countryId, 'y')));
          await vpn.waitForElementProperty(cityId, 'visible', 'true');
          const cityName = await vpn.getElementProperty(cityId, 'radioButtonLabelText'.split(" "));

          await vpn.wait();
          await vpn.clickOnElement(cityId);
          await vpn.wait();

          // Back to the main view.          
          await vpn.waitForElement(elements.SERVER_ENTRY_BUTTON);          
          await vpn.waitForElementProperty(elements.SERVER_ENTRY_BUTTON, 'visible', 'true');          
          await vpn.waitForElementAndClick(elements.SERVER_ENTRY_BUTTON);

          // One selected
          await vpn.waitForElement(cityId);
          await vpn.waitForElementProperty(cityId, 'checked', 'true');
          assert(cityName.includes(city.name))
        }        
      }
  });

  it('Pick cities for exits', async () => {
    let countryId;      
    await vpn.waitForElementAndClick(elements.MULTIHOP_SELECTOR_TAB);
    await vpn.waitForElementAndClick(elements.SERVER_EXIT_BUTTON);
    
    for (let server of servers) {        
      countryId = 'serverCountryList/serverCountry-' + server.code;
      await vpn.waitForElement(countryId);

      await vpn.setElementProperty(
          elements.SERVER_COUNTRY_VIEW, 'contentY', 'i',
          parseInt(await vpn.getElementProperty(countryId, 'y')));
      await vpn.wait();
      
      if (await vpn.getElementProperty(countryId, elements.CITYLIST_VISIBLE) === 'false') {
        await vpn.clickOnElement(countryId);
      }
      
      await vpn.waitForElementProperty(countryId, elements.CITYLIST_VISIBLE, 'true');       

      for (let city of server.cities) {
        const cityId = countryId + '/serverCityList/serverCity-' + city.name.replace(/ /g, '_');
        await vpn.waitForElement(cityId);

        await vpn.setElementProperty(
            elements.SERVER_COUNTRY_VIEW, 'contentY', 'i',
            parseInt(await vpn.getElementProperty(cityId, 'y')) +
                parseInt(await vpn.getElementProperty(countryId, 'y')));
        await vpn.waitForElementProperty(cityId, 'visible', 'true');
        const cityName = await vpn.getElementProperty(cityId, 'radioButtonLabelText'.split(" "));

        await vpn.wait();
        await vpn.clickOnElement(cityId);
        await vpn.wait();

        // Back to the main view.          
        await vpn.waitForElement(elements.SERVER_EXIT_BUTTON);          
        await vpn.waitForElementProperty(elements.SERVER_EXIT_BUTTON, 'visible', 'true');          
        await vpn.waitForElementAndClick(elements.SERVER_EXIT_BUTTON);

        // One selected
        await vpn.waitForElement(cityId);
        await vpn.waitForElementProperty(cityId, 'checked', 'true');
        assert(cityName.includes(city.name))
      }        
    }
  });

  it('Server switching', async () => {
    let newCurrentCountry;
    let newCurrentCity;

    let serverOne = servers[Math.floor(Math.random() * servers.length)]
    let serverTwo = await _chooseNewServer(serverOne, servers)    
    let serverThree;
    
    await actions.settings.serverSwitchNotification()
    await actions.settings.connectionChangeNotification()

    // wait for select entry      
    await vpn.waitForElementAndClick(elements.MULTIHOP_SELECTOR_TAB);
            
    // select entry
    await vpn.waitForElementAndClick(elements.SERVER_ENTRY_BUTTON);
    
    // wait for country of choice to be visible
    const countryId = 'serverCountryList/serverCountry-' + serverOne.code;
    const exitCountryId = 'serverCountryList/serverCountry-' + serverTwo.code;

    // select the country of choice
    await vpn.setElementProperty(elements.SERVER_COUNTRY_VIEW, 'contentY', 'i', parseInt(await vpn.getElementProperty(countryId, 'y')));
    await vpn.wait();
    if (await vpn.getElementProperty(countryId, 'cityListVisible') === 'false') {
      await vpn.clickOnElement(countryId);
    }
    await vpn.waitForElementProperty(countryId, 'cityListVisible', 'true');

    // navigate and select city    
    const cityOne = serverOne.cities[Math.floor(Math.random() * serverOne.cities.length)]
    const cityTwo = serverTwo.cities[Math.floor(Math.random() * serverTwo.cities.length)]
    const cityOneId = countryId + '/serverCityList/serverCity-' + cityOne.name.replace(/ /g, '_');

    // select city
    await vpn.setElementProperty(
        elements.SERVER_COUNTRY_VIEW, 'contentY', 'i',
        parseInt(await vpn.getElementProperty(cityOneId, 'y')) +
            parseInt(await vpn.getElementProperty(countryId, 'y')));
    await vpn.wait()         
    await vpn.waitForElementAndClick(cityOneId);
    
    // Back at the main view. select the exit entries     
    await vpn.waitForElementAndClick(elements.SERVER_EXIT_BUTTON);
    
    // set country view
    await vpn.setElementProperty(elements.SERVER_COUNTRY_VIEW, 'contentY', 'i', parseInt(await vpn.getElementProperty(exitCountryId, 'y')));
    await vpn.wait();

    // check if country is visible
    const cityTwoId = exitCountryId + '/serverCityList/serverCity-' + cityTwo.name.replace(/ /g, '_');
    if (await vpn.getElementProperty(exitCountryId, 'cityListVisible') ===  'false') {
      await vpn.clickOnElement(exitCountryId);
    }
  
    // set city view and select  
    await vpn.setElementProperty(
      elements.SERVER_COUNTRY_VIEW, 'contentY', 'i',
      parseInt(await vpn.getElementProperty(cityTwoId, 'y')) +
          parseInt(await vpn.getElementProperty(exitCountryId, 'y')));
    await vpn.wait()      
    await vpn.waitForElementAndClick(cityTwoId);
    
    // navigate back to connection view
    await vpn.waitForElementAndClick(elements.SERVERLIST_BACK_BUTTON);

    // define connected server    
    currentCountry = serverTwo.localizedName;
    currentCity = cityTwo.localizedName; 
    
    // connect vpn
    await vpn.activate();
    
    // wait and assert vpn connection
    await vpn.waitForCondition(async () => {
      return await vpn.getElementProperty(elements.CONTROLLER_TITLE, 'text') ==
          'VPN is on';
    });
    assert.strictEqual(vpn.lastNotification().title, 'VPN Connected');      
    assert.strictEqual(vpn.lastNotification().message, `Connected to ${currentCountry}, ${currentCity}`);

    // back to main view
    await vpn.waitForElementAndClick(elements.SERVER_LIST_BUTTON);

    // choose random server
    await vpn.waitForElementAndClick(elements.SERVER_EXIT_BUTTON);
    serverThree = await _chooseNewServer(serverTwo, servers)
    const randomCountryId = 'serverCountryList/serverCountry-' + serverThree.code;    
    await vpn.waitForElement(randomCountryId);

    // open third server city(s)
    await vpn.setElementProperty(
      elements.SERVER_COUNTRY_VIEW, 'contentY', 'i',
      parseInt(await vpn.getElementProperty(randomCountryId, 'y')));
    if (await vpn.getElementProperty(randomCountryId, elements.CITYLIST_VISIBLE) === 'false') {
      await vpn.clickOnElement(randomCountryId);
    }
  
    // define third server city
    const cityThree = serverThree.cities[Math.floor(Math.random() * serverThree.cities.length)]
    const cityThreeId = randomCountryId + '/serverCityList/serverCity-' + cityThree.name.replace(/ /g, '_');
    await vpn.waitForElement(cityThreeId);

    await vpn.setElementProperty(
      elements.SERVER_COUNTRY_VIEW, 'contentY', 'i',
      parseInt(await vpn.getElementProperty(cityThreeId, 'y')) +
          parseInt(await vpn.getElementProperty(randomCountryId, 'y')));
    await vpn.waitForElementProperty(cityThreeId, 'visible', 'true');
    
    // define new connected server
    newCurrentCountry = serverThree.localizedName;
    newCurrentCity = cityThree.localizedName; 

    await vpn.wait();
    await vpn.clickOnElement(cityThreeId);
    await vpn.wait();    
    await vpn.waitForElementAndClick(elements.SERVERLIST_BACK_BUTTON);
    
    // wait and assert server switching for multihop
    await vpn.waitForCondition(async () => {
      return vpn.lastNotification().title == "VPN Switched Servers"
    }, 20)
    assert.strictEqual(
        vpn.lastNotification().message,
        `Switched from ${currentCountry}, ${currentCity} to ${
            newCurrentCountry}, ${newCurrentCity}`);
    

    // switch to single hop    
    await vpn.waitForElementAndClick(elements.SERVER_LIST_BUTTON);
    await vpn.waitForElementAndClick(elements.SINGLEHOP_SELECTOR_TAB);
    await vpn.waitForElementAndClick(elements.SERVERLIST_BACK_BUTTON);

    // assert switching from multihop to single
    assert.strictEqual(vpn.lastNotification().title, 'VPN Switched Servers');
    assert.strictEqual(
        vpn.lastNotification().message,
        `Switched from ${currentCountry}, ${currentCity} to ${
            newCurrentCountry}, ${newCurrentCity}`);
  
    await vpn.deactivate()
  });
});

// choose a new server, server in arg being the server that cannot be chosen
async function _chooseNewServer(server, servers){
  let newServer;
  while (true) {
    newServer = servers[Math.floor(Math.random() * servers.length)];
    if (newServer.code != server.code) break;
  }

  return newServer
}