/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

 const assert = require('assert');
 const { actions } = require('./actions.js');
 const { elementState, generalElements, homeScreen } = require('./elements.js');
 const vpn = require('./helper.js');

describe('Server list', function() {
   let servers;
   let currentCountryCode;
   let currentCity;
 
   this.timeout(240000);
   this.ctx.authenticationNeeded = true;
 
  beforeEach(async () => {
     await vpn.waitForElementAndClick(homeScreen.SERVER_LIST_BUTTON);
 
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
    await vpn.waitForElementAndClick(homeScreen.selectSingleHopServerView.MULTIHOP_SELECTOR_TAB);
    assert(await vpn.getElementProperty(homeScreen.selectMultiHopServerView.VPN_COLLAPSIBLE_CARD, 'expanded') === 'false')

    await vpn.waitForElement(homeScreen.selectMultiHopServerView.ENTRY_BUTTON);
    await vpn.waitForElementProperty(homeScreen.selectMultiHopServerView.ENTRY_BUTTON, 'visible', 'true');

    await vpn.waitForElement(homeScreen.selectMultiHopServerView.EXIT_BUTTON);
    await vpn.waitForElementProperty(homeScreen.selectMultiHopServerView.EXIT_BUTTON, 'visible', 'true'); 
    
    await vpn.waitForElementAndClick(homeScreen.selectMultiHopServerView.VPN_MULTHOP_CHEVRON)
    assert(await vpn.getElementProperty(homeScreen.selectMultiHopServerView.VPN_COLLAPSIBLE_CARD, 'expanded'))
  });
  
  it.only('check the countries and cities for multihop entries', async () => {
    await vpn.waitForElementAndClick(homeScreen.selectSingleHopServerView.MULTIHOP_SELECTOR_TAB);
    await vpn.waitForElementAndClick(homeScreen.selectMultiHopServerView.ENTRY_BUTTON);

    for (let server of servers) {
      const countryId = homeScreen.serverListView.generateCountryId(server.code);
      await vpn.waitForElement(countryId);
      await vpn.waitForElementProperty(countryId, 'visible', 'true');

      await actions.serverList.selectCountryFromList(countryId)
      await vpn.wait();

      if (currentCountryCode === server.code) {
        assert(await vpn.getElementProperty(countryId, 'cityListVisible') === 'true');
      }

      if (await vpn.getElementProperty(countryId, 'cityListVisible') ===
          'false') {
        await vpn.clickOnElement(countryId);
      }

      for (let city of server.cities) {
        const cityId = homeScreen.serverListView.generateCityId(countryId, city.name);
        console.log('  Waiting for cityId:', cityId);
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

  it('check the countries and cities for multihop exits', async () => {
    await vpn.waitForElementAndClick(homeScreen.selectSingleHopServerView.MULTIHOP_SELECTOR_TAB);    
    await vpn.waitForElementAndClick(homeScreen.selectMultiHopServerView.EXIT_BUTTON);

    for (let server of servers) {
      const countryId = homeScreen.serverListView.generateCountryId(server.code);

      await vpn.waitForElement(countryId);
      await vpn.waitForElementProperty(countryId, 'visible', 'true');

      await actions.serverList.selectCountryFromList(countryId);
      await vpn.wait();

      if (currentCountryCode === server.code) {
        assert(
            await vpn.getElementProperty(countryId, elementState.CITYLIST_VISIBLE) ===
            'true');
      }      

      if (await vpn.getElementProperty(countryId, elementState.CITYLIST_VISIBLE) ===
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
      await vpn.waitForElementAndClick(homeScreen.selectSingleHopServerView.MULTIHOP_SELECTOR_TAB);
      await vpn.waitForElementAndClick(homeScreen.selectMultiHopServerView.ENTRY_BUTTON);         
      
      for (let server of servers) {        
        countryId = homeScreen.serverListView.generateCountryId(server.code);
        await vpn.waitForElement(countryId);

        await actions.serverList.selectCountryFromList(countryId);
        await vpn.wait();
        
        if (await vpn.getElementProperty(countryId, elementState.CITYLIST_VISIBLE) === 'false') {
          await vpn.clickOnElement(countryId);
        }
        
        await vpn.waitForElementProperty(countryId, elementState.CITYLIST_VISIBLE, 'true');       

        for (let city of server.cities) {
          const cityId = countryId + '/serverCityList/serverCity-' + city.name.replace(/ /g, '_');
          await vpn.waitForElement(cityId);

          await actions.serverList.selectCityFromList(cityId, countryId);
          await vpn.waitForElementProperty(cityId, 'visible', 'true');
          const cityName = await vpn.getElementProperty(cityId, 'radioButtonLabelText'.split(" "));

          await vpn.wait();
          await vpn.clickOnElement(cityId);
          await vpn.wait();

          // Back to the main view.          
          await vpn.waitForElement(homeScreen.selectMultiHopServerView.ENTRY_BUTTON);          
          await vpn.waitForElementProperty(homeScreen.selectMultiHopServerView.ENTRY_BUTTON, 'visible', 'true');          
          await vpn.waitForElementAndClick(homeScreen.selectMultiHopServerView.ENTRY_BUTTON);

          // One selected
          await vpn.waitForElement(cityId);
          await vpn.waitForElementProperty(cityId, 'checked', 'true');
          assert(cityName.includes(city.name))
        }        
      }
  });

  it('Pick cities for exits', async () => {
    let countryId;      
    await vpn.waitForElementAndClick(homeScreen.selectSingleHopServerView.MULTIHOP_SELECTOR_TAB);
    await vpn.waitForElementAndClick(homeScreen.selectMultiHopServerView.EXIT_BUTTON);
    
    for (let server of servers) {        
      countryId = homeScreen.serverListView.generateCountryId(server.code);
      await vpn.waitForElement(countryId);

      await actions.serverList.selectCountryFromList(countryId)
      await vpn.wait();
      
      if (await vpn.getElementProperty(countryId, elementState.CITYLIST_VISIBLE) === 'false') {
        await vpn.clickOnElement(countryId);
      }
      
      await vpn.waitForElementProperty(countryId, elementState.CITYLIST_VISIBLE, 'true');       

      for (let city of server.cities) {
        const cityId = countryId + '/serverCityList/serverCity-' + city.name.replace(/ /g, '_');
        await vpn.waitForElement(cityId);

        await actions.serverList.selectCityFromList(cityId, countryId)
        await vpn.waitForElementProperty(cityId, 'visible', 'true');
        const cityName = await vpn.getElementProperty(cityId, 'radioButtonLabelText'.split(" "));

        await vpn.wait();
        await vpn.clickOnElement(cityId);
        await vpn.wait();

        // Back to the main view.          
        await vpn.waitForElement(homeScreen.selectMultiHopServerView.EXIT_BUTTON);          
        await vpn.waitForElementProperty(homeScreen.selectMultiHopServerView.EXIT_BUTTON, 'visible', 'true');          
        await vpn.waitForElementAndClick(homeScreen.selectMultiHopServerView.EXIT_BUTTON);

        // One selected
        await vpn.waitForElement(cityId);
        await vpn.waitForElementProperty(cityId, 'checked', 'true');
        assert(cityName.includes(city.name))
      }        
    }
  });  

  it('Server switching -- same country different cities', async () => {
    await actions.settings.setServerSwitchNotification(true)
    await actions.settings.setConnectionChangeNotification(true)

    let newCurrentCountry;
    let newCurrentCity;
    let currentCountry;
    let currentCity;
    
    // wait for select entry and select entry
    await vpn.waitForElementAndClick(homeScreen.selectSingleHopServerView.MULTIHOP_SELECTOR_TAB);
    await vpn.waitForElementAndClick(homeScreen.selectMultiHopServerView.ENTRY_BUTTON);
    
    // exit server details
    const firstServer = servers[0]
    const cityTwo = firstServer.cities[0]
    const cityThree = firstServer.cities[1]
    const exitFirstCountryId = homeScreen.serverListView.generateCountryId(firstServer.code);    

    // entry server details
    const secondServer = servers[1]
    const cityOne = secondServer.cities[0]
    const entryCountryId = homeScreen.serverListView.generateCountryId(secondServer.code);

    // select the first country    
    await actions.serverList.selectCountryFromList(entryCountryId);
    await vpn.wait()
    if (await vpn.getElementProperty(entryCountryId, 'cityListVisible') === 'false') {
      await vpn.clickOnElement(entryCountryId);
    }
    await vpn.waitForElementProperty(entryCountryId, 'cityListVisible', 'true');
    
    // select first city
    const cityOneId = homeScreen.serverListView.generateCityId(entryCountryId, cityOne.name);
        
    await vpn.setElementProperty(
      elementState.COUNTRY_VIEW, 'contentY', 'i',
      parseInt(await vpn.getElementProperty(cityOneId, 'y')) +
          parseInt(await vpn.getElementProperty(entryCountryId, 'y')));
    await vpn.wait()
    await vpn.waitForElementAndClick(cityOneId)

    // Back at the main view. select the exit entries
    await vpn.waitForElementAndClick(homeScreen.selectMultiHopServerView.EXIT_BUTTON);

    // select first country again
    await actions.serverList.selectCountryFromList(exitFirstCountryId)
    await vpn.wait();
    if (await vpn.getElementProperty(exitFirstCountryId, 'cityListVisible') === 'false') {
      await vpn.clickOnElement(exitFirstCountryId);
    }
    await vpn.waitForElementProperty(exitFirstCountryId, 'cityListVisible', 'true');
    
    // select first city in exit country
    const cityTwoId = homeScreen.serverListView.generateCityId(exitFirstCountryId, cityTwo.name);
    
    await vpn.setElementProperty(
      elementState.COUNTRY_VIEW, 'contentY', 'i',
      parseInt(await vpn.getElementProperty(cityTwoId, 'y')) +
          parseInt(await vpn.getElementProperty(exitFirstCountryId, 'y')));
    await vpn.wait()
    await vpn.waitForElementAndClick(cityTwoId);

    // navigate back to connection view
    await vpn.waitForElementAndClick(homeScreen.selectSingleHopServerView.BACK_BUTTON);

    // define connected server    
    currentCountry = firstServer.localizedName;
    currentCity = cityTwo.localizedName; 
    
    // connect vpn
    await vpn.activate();

    // wait and assert vpn connection
    await vpn.waitForCondition(async () => {
      return await vpn.getElementProperty(generalElements.CONTROLLER_TITLE, 'text') ==
          'VPN is on';
    });
    assert.strictEqual(vpn.lastNotification().title, 'VPN Connected');      
    assert.strictEqual(vpn.lastNotification().message, `Connected to ${currentCountry}, ${currentCity}`);

    // back to main view
    await vpn.waitForElementAndClick(homeScreen.SERVER_LIST_BUTTON);

    // Back at the main view. select the exit entries
    await vpn.waitForElementAndClick(homeScreen.selectMultiHopServerView.EXIT_BUTTON);

    // select first country again
    await actions.serverList.selectCountryFromList(exitFirstCountryId)
    await vpn.wait();
    if (await vpn.getElementProperty(exitFirstCountryId, 'cityListVisible') === 'false') {
      await vpn.clickOnElement(exitFirstCountryId);
    }
    await vpn.waitForElementProperty(exitFirstCountryId, 'cityListVisible', 'true');
    
    // select first city in exit country
    const cityThreeId = homeScreen.serverListView.generateCityId(exitFirstCountryId, cityThree.name);

    await vpn.setElementProperty(
      elementState.COUNTRY_VIEW, 'contentY', 'i',
      parseInt(await vpn.getElementProperty(cityThreeId, 'y')) +
          parseInt(await vpn.getElementProperty(exitFirstCountryId, 'y')));    
    await vpn.waitForElementAndClick(cityThreeId);

    // Back at the main view. select the exit entries
    await vpn.waitForElementAndClick(homeScreen.selectSingleHopServerView.BACK_BUTTON);

    // define new connected server    
    newCurrentCountry = firstServer.localizedName;
    newCurrentCity = cityThree.localizedName;

    // wait and assert server switching for multihop
    await vpn.waitForCondition(async () => {
      return vpn.lastNotification().title == "VPN Switched Servers"
    }, 20)
    assert.strictEqual(
        vpn.lastNotification().message,
        `Switched from ${currentCountry}, ${currentCity} to ${
            newCurrentCountry}, ${newCurrentCity}`);
  });

  it('Server switching -- different country different cities', async () => {
    await actions.settings.setServerSwitchNotification(true)
    await actions.settings.setConnectionChangeNotification(true)

    let newCurrentCountry;
    let newCurrentCity;
    let currentCountry;
    let currentCity;
    
    // wait for select entry and select entry
    await vpn.waitForElementAndClick(homeScreen.selectSingleHopServerView.MULTIHOP_SELECTOR_TAB);
    await vpn.waitForElementAndClick(homeScreen.selectMultiHopServerView.ENTRY_BUTTON);
    
    // exit server details
    const firstServer = servers[0]
    const cityTwo = firstServer.cities[0]    
    const exitFirstCountryId = homeScreen.serverListView.generateCountryId(firstServer.code); 
    
    // second exit server details
    const thirdServer = servers[2]    
    const cityThree = thirdServer.cities[0]
    const exitThirdCountryId = homeScreen.serverListView.generateCountryId(thirdServer.code); 

    // entry server details
    const secondServer = servers[1]
    const cityOne = secondServer.cities[0]
    const entryCountryId = homeScreen.serverListView.generateCountryId(secondServer.code);

    // select the first country    
    await actions.serverList.selectCountryFromList(entryCountryId);
    await vpn.wait()
    if (await vpn.getElementProperty(entryCountryId, 'cityListVisible') === 'false') {
      await vpn.clickOnElement(entryCountryId);
    }
    await vpn.waitForElementProperty(entryCountryId, 'cityListVisible', 'true');
    
    // select first city
    const cityOneId = homeScreen.serverListView.generateCityId(entryCountryId, cityOne.name);        
    await actions.serverList.selectCityFromList(cityOneId, entryCountryId)
    await vpn.waitForElementAndClick(cityOneId)

    // Back at the main view. select the exit entries
    await vpn.waitForElementAndClick(homeScreen.selectMultiHopServerView.EXIT_BUTTON);

    // select first country again
    await actions.serverList.selectCountryFromList(exitFirstCountryId)
    await vpn.wait();
    if (await vpn.getElementProperty(exitFirstCountryId, 'cityListVisible') === 'false') {
      await vpn.clickOnElement(exitFirstCountryId);
    }
    await vpn.waitForElementProperty(exitFirstCountryId, 'cityListVisible', 'true');
    
    // select first city in exit country
    const cityTwoId = homeScreen.serverListView.generateCityId(exitFirstCountryId, cityTwo.name);    
    await actions.serverList.selectCityFromList(cityTwoId, exitFirstCountryId)
    await vpn.waitForElementAndClick(cityTwoId);    

    // navigate back to connection view
    await vpn.waitForElementAndClick(homeScreen.selectSingleHopServerView.BACK_BUTTON);

    // define connected server    
    currentCountry = firstServer.localizedName;
    currentCity = cityTwo.localizedName; 
    
    // connect vpn
    await vpn.activate();

    // wait and assert vpn connection
    await vpn.waitForCondition(async () => {
      return await vpn.getElementProperty(generalElements.CONTROLLER_TITLE, 'text') ==
          'VPN is on';
    });
    assert.strictEqual(vpn.lastNotification().title, 'VPN Connected');      
    assert.strictEqual(vpn.lastNotification().message, `Connected to ${currentCountry}, ${currentCity}`);

    // back to main view
    await vpn.waitForElementAndClick(homeScreen.SERVER_LIST_BUTTON);

    // Back at the main view. select the exit entries
    await vpn.waitForElementAndClick(homeScreen.selectMultiHopServerView.EXIT_BUTTON);

    // select first country again
    await actions.serverList.selectCountryFromList(exitThirdCountryId)
    await vpn.wait();
    if (await vpn.getElementProperty(exitThirdCountryId, 'cityListVisible') === 'false') {
      await vpn.clickOnElement(exitThirdCountryId);
    }
    await vpn.waitForElementProperty(exitThirdCountryId, 'cityListVisible', 'true');
    
    // select first city in exit country
    const cityThreeId = homeScreen.serverListView.generateCityId(exitThirdCountryId, cityThree.name);
    await actions.serverList.selectCityFromList(cityThreeId, exitThirdCountryId)
    await vpn.waitForElementAndClick(cityThreeId);

    // Back at the main view. select the exit entries
    await vpn.waitForElementAndClick(homeScreen.selectSingleHopServerView.BACK_BUTTON);

    // define new connected server    
    newCurrentCountry = thirdServer.localizedName;
    newCurrentCity = cityThree.localizedName;

    // wait and assert server switching for multihop
    await vpn.waitForCondition(async () => {
      return vpn.lastNotification().title == "VPN Switched Servers"
    }, 20)
    assert.strictEqual(
        vpn.lastNotification().message,
        `Switched from ${currentCountry}, ${currentCity} to ${
            newCurrentCountry}, ${newCurrentCity}`);
  });

  it('Single and multihop switching', async () => {
    await actions.settings.setServerSwitchNotification(true)
    await actions.settings.setConnectionChangeNotification(true)

    let currentCountry;
    let currentCity;
    
    // wait for select entry and select entry
    await vpn.waitForElementAndClick(homeScreen.selectSingleHopServerView.MULTIHOP_SELECTOR_TAB);
    await vpn.waitForElementAndClick(homeScreen.selectMultiHopServerView.ENTRY_BUTTON);
    
    // exit server details
    const firstServer = servers[0]
    const cityTwo = firstServer.cities[0]    
    const exitFirstCountryId = homeScreen.serverListView.generateCountryId(firstServer.code); 

    // entry server details
    const secondServer = servers[1]
    const cityOne = secondServer.cities[0]
    const entryCountryId = homeScreen.serverListView.generateCountryId(secondServer.code);

    // select the first country    
    await actions.serverList.selectCountryFromList(entryCountryId);
    await vpn.wait()
    if (await vpn.getElementProperty(entryCountryId, 'cityListVisible') === 'false') {
      await vpn.clickOnElement(entryCountryId);
    }
    await vpn.waitForElementProperty(entryCountryId, 'cityListVisible', 'true');
    
    // select first city
    const cityOneId = homeScreen.serverListView.generateCityId(entryCountryId, cityOne.name);        
    await actions.serverList.selectCityFromList(cityOneId, entryCountryId)
    await vpn.waitForElementAndClick(cityOneId)

    // Back at the main view. select the exit entries
    await vpn.waitForElementAndClick(homeScreen.selectMultiHopServerView.EXIT_BUTTON);

    // select first country again
    await actions.serverList.selectCountryFromList(exitFirstCountryId)
    await vpn.wait();
    if (await vpn.getElementProperty(exitFirstCountryId, 'cityListVisible') === 'false') {
      await vpn.clickOnElement(exitFirstCountryId);
    }
    await vpn.waitForElementProperty(exitFirstCountryId, 'cityListVisible', 'true');
    
    // select first city in exit country
    const cityTwoId = homeScreen.serverListView.generateCityId(exitFirstCountryId, cityTwo.name);    
    await actions.serverList.selectCityFromList(cityTwoId, exitFirstCountryId)
    await vpn.waitForElementAndClick(cityTwoId);    

    // navigate back to connection view
    await vpn.waitForElementAndClick(homeScreen.selectSingleHopServerView.BACK_BUTTON);

    // define connected server    
    currentCountry = firstServer.localizedName;
    currentCity = cityTwo.localizedName; 
    
    // connect vpn
    await vpn.activate();

    // wait and assert vpn connection
    await vpn.waitForCondition(async () => {
      return await vpn.getElementProperty(generalElements.CONTROLLER_TITLE, 'text') ==
          'VPN is on';
    });
    assert.strictEqual(vpn.lastNotification().title, 'VPN Connected');      
    assert.strictEqual(vpn.lastNotification().message, `Connected to ${currentCountry}, ${currentCity}`);

    // back to main view
    await vpn.waitForElementAndClick(homeScreen.SERVER_LIST_BUTTON);
    

    // switch from multihop to singlehop
    await vpn.waitForElementAndClick(homeScreen.selectSingleHopServerView.SINGLEHOP_SELECTOR_TAB)
    await vpn.waitForElementAndClick(homeScreen.selectSingleHopServerView.BACK_BUTTON)

    // wait and assert vpn connection
    await vpn.waitForCondition(async () => {
      return await vpn.getElementProperty(generalElements.CONTROLLER_TITLE, 'text') ==
          'VPN is on';
    });
    assert.strictEqual(vpn.lastNotification().title, 'VPN Connected');      
    assert.strictEqual(vpn.lastNotification().message, `Connected to ${currentCountry}, ${currentCity}`);  
  });
});