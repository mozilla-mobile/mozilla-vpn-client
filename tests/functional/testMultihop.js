/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
 const { beforeEach } = require('mocha');
 const vpn = require('./helper.js');
 
 describe('Server list', function() {
   let servers;
   let currentCountryCode;
   let currentCity;
 
   this.timeout(240000);
   this.ctx.authenticationNeeded = true;
 
   beforeEach(async () => {
     await vpn.waitForPropertyAndClickOnElement('serverListButton');
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

   it('Checking multihop for australia', async () => {
    for(let i=0; i<servers.length; i++){
      // wait for select entry      
      await vpn.waitForPropertyAndClickOnElement('multiHopSelector/tabMultiHop');
  
      // select entry
      await vpn.waitForPropertyAndClickOnElement('buttonSelectEntry');    
      
      // wait for country of choice to be visible    
      const server = servers[i]
      const countryId = 'serverCountryList/serverCountry-' + server.code;
      const exitCountyId = 'serverCountryList/serverCountry-' + servers[0].code;      
  
      // select the country of choice      
      await vpn.setElementProperty('serverCountryView', 'contentY', 'i', parseInt(await vpn.getElementProperty(countryId, 'y')));
      await vpn.wait();
      if (await vpn.getElementProperty(countryId, 'cityListVisible') === 'false') {
        await vpn.clickOnElement(countryId);
      }      
      await vpn.waitForElementProperty(countryId, 'cityListVisible', 'true');
  
      // navigate and select city    
      const cityOne = server.cities[0]
      const cityTwo = servers[0].cities[1]
      const cityOneId = countryId + '/serverCityList/serverCity-' + cityOne.name.replace(/ /g, '_');
      
      // select city      
      await vpn.setElementProperty(
          'serverCountryView', 'contentY', 'i',
          parseInt(await vpn.getElementProperty(cityOneId, 'y')) +
              parseInt(await vpn.getElementProperty(countryId, 'y')));
      await vpn.wait()         
      await vpn.waitForPropertyAndClickOnElement(cityOneId);            
      await vpn.wait()
      
            
      // Back at the main view. select the exit entries     
      await vpn.waitForPropertyAndClickOnElement('buttonSelectExit');      
      await vpn.wait();
      
      // set country view        
      await vpn.setElementProperty('serverCountryView', 'contentY', 'i', parseInt(await vpn.getElementProperty(exitCountyId, 'y')));
      await vpn.wait();
  
      // check if country is visible
      const cityTwoId = exitCountyId + '/serverCityList/serverCity-' + cityTwo.name.replace(/ /g, '_');
      if (await vpn.getElementProperty(exitCountyId, 'cityListVisible') ===  'false') {
        await vpn.clickOnElement(exitCountyId);
      }
    
      // set city view      
      await vpn.setElementProperty(
        'serverCountryView', 'contentY', 'i',
        parseInt(await vpn.getElementProperty(cityTwoId, 'y')) +
            parseInt(await vpn.getElementProperty(exitCountyId, 'y')));
      await vpn.wait()
  
      // select city      
      await vpn.waitForPropertyAndClickOnElement(cityTwoId);
      await vpn.wait()
    }    
   })  
 });
 