/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */ 

const elements = require('./elements.js');
const vpn = require('./helper.js');
 
const setConnectionChangeNotification = async (status) => {
    return await vpn.setSetting('connection-change-notification', status ? "true" : "false");
}

const setServerSwitchNotification = async (status) => {    
    return await vpn.setSetting('server-switch-notification', status ? "true" : "false");
}

const selectCountryFromList = async (countryId) => {
  await vpn.setElementProperty(elements.SERVER_COUNTRY_VIEW, 'contentY', 'i', parseInt(await vpn.getElementProperty(countryId, 'y')));
  await vpn.wait()
}

const selectCityFromList = async (cityId, countryId) => {
  await vpn.setElementProperty(elements.SERVER_COUNTRY_VIEW, 'contentY', 'i', parseInt(await vpn.getElementProperty(cityId, 'y')) + parseInt(await vpn.getElementProperty(countryId, 'y')));
  await vpn.wait()
}

module.exports = {
    actions: {
        settings: {
            setConnectionChangeNotification,
            setServerSwitchNotification,
        },
        locations: {
            selectCountryFromList,
            selectCityFromList
        }
    }
}