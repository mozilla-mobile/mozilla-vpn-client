/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
 const assert = require('assert');
 const vpn = require('./helper.js');

 describe('Server list', function() {
   let servers;
   let currentCountryCode;
   let currentCity;
   let currentCountry;
   let prevCurrentCountry
   let prevCurrentCity
 
   this.timeout(240000);
   this.ctx.authenticationNeeded = true;
 
   beforeEach(async () => {
     await vpn.waitForElementAndClick('serverListButton');
     await vpn.wait();
 
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

   it('Checking multihop functionality', async () => {
    for(let i=0; i<2; i++){            
      const idx = getRandomInt()

      // wait for select entry      
      await vpn.waitForElementAndClick('multiHopSelector/tabMultiHop');
             
      // select entry
      await vpn.waitForElementAndClick('buttonSelectEntry');
      
      // wait for country of choice to be visible    
      const server = servers[i]
      const countryId = 'serverCountryList/serverCountry-' + server.code;
      const exitCountryId = 'serverCountryList/serverCountry-' + servers[idx].code;
  
      // select the country of choice      
      await vpn.setElementProperty('serverCountryView', 'contentY', 'i', parseInt(await vpn.getElementProperty(countryId, 'y')));
      await vpn.wait();

      if (await vpn.getElementProperty(countryId, 'cityListVisible') === 'false') {
        await vpn.clickOnElement(countryId);
      }  
      
      await vpn.waitForElementProperty(countryId, 'cityListVisible', 'true');
  
      // navigate and select city    
      const cityOne = server.cities[getRandomInt(server.cities.length)]
      const cityTwo = servers[idx].cities[getRandomInt(servers[idx].cities.length)]
      const cityOneId = countryId + '/serverCityList/serverCity-' + cityOne.name.replace(/ /g, '_');

      // select city      
      await vpn.setElementProperty(
          'serverCountryView', 'contentY', 'i',
          parseInt(await vpn.getElementProperty(cityOneId, 'y')) +
              parseInt(await vpn.getElementProperty(countryId, 'y')));
      await vpn.wait()         
      await vpn.waitForElementAndClick(cityOneId);
      
      // Back at the main view. select the exit entries     
      await vpn.waitForElementAndClick('buttonSelectExit');
      
      // set country view
      await vpn.setElementProperty('serverCountryView', 'contentY', 'i', parseInt(await vpn.getElementProperty(exitCountryId, 'y')));
      await vpn.wait();
  
      // check if country is visible
      const cityTwoId = exitCountryId + '/serverCityList/serverCity-' + cityTwo.name.replace(/ /g, '_');
      if (await vpn.getElementProperty(exitCountryId, 'cityListVisible') ===  'false') {
        await vpn.clickOnElement(exitCountryId);
      }
    
      // set city view and select  
      await vpn.setElementProperty(
        'serverCountryView', 'contentY', 'i',
        parseInt(await vpn.getElementProperty(cityTwoId, 'y')) +
            parseInt(await vpn.getElementProperty(exitCountryId, 'y')));
      await vpn.wait()      
      await vpn.waitForElementAndClick(cityTwoId);
      
      // navigate back to 
      await vpn.waitForElementAndClick('serverListBackButton');

      currentCountry = servers[idx].localizedName;
      currentCity = cityTwo.localizedName;      

      if(i < 1){
        await vpn.activate();
        await vpn.waitForCondition(async () => {
          return await vpn.getElementProperty('controllerTitle', 'text') ==
              'VPN is on';
        });

        assert.strictEqual(vpn.lastNotification().title, 'VPN Connected');
        assert.strictEqual(vpn.lastNotification().message, `Connected to ${currentCountry}, ${currentCity}`);
        
        prevCurrentCountry = currentCountry
        prevCurrentCity = currentCity
      } else {

        await vpn.waitForCondition(async () => {
          return vpn.lastNotification().title == "VPN Switched Servers"
        }, 5000)
        assert.strictEqual(vpn.lastNotification().title, 'VPN Switched Servers');
        assert.strictEqual(vpn.lastNotification().message, `Switched from ${prevCurrentCountry}, ${prevCurrentCity} to ${currentCountry}, ${currentCity}`);

        if(i !== servers.length - 1){
          prevCurrentCountry = currentCountry
          prevCurrentCity = currentCity
        }
      }


      // go back to server list
      await vpn.waitForElementAndClick('serverListButton');
    }

    // Verify switch to single hop
    await vpn.waitForElementAndClick('multiHopSelector/tabSingleHop');
    await vpn.waitForElementAndClick('serverListBackButton');

    await vpn.waitForCondition(async () => {
      return vpn.lastNotification().title == "VPN Switched Servers"
    }, 5000)
    assert.strictEqual(vpn.lastNotification().title, 'VPN Switched Servers');
  
    await vpn.deactivate()
   })
});

function getRandomInt(max = 3) {
  return Math.floor(Math.random() * max);
}