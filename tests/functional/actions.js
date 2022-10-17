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

const goToSettings = async () => {
    await vpn.openSettings();
}

const activateDeveloperOptions = async () => {
    await vpn.waitForElement(elements.VPN_BOLD_LABEL);
    await vpn.clickOnElement(elements.VPN_BOLD_LABEL);
    await vpn.clickOnElement(elements.VPN_BOLD_LABEL);
    await vpn.clickOnElement(elements.VPN_BOLD_LABEL);
    await vpn.clickOnElement(elements.VPN_BOLD_LABEL);
    await vpn.clickOnElement(elements.VPN_BOLD_LABEL);
    await vpn.clickOnElement(elements.VPN_BOLD_LABEL);
    await vpn.waitForCondition(async () => {
        return  await vpn.getElementProperty('developer', 'visible')
    }, 200)
    await vpn.waitForElementAndClick(elements.DEVELOPER_OPTIONS_BUTTON)
}

const enableCustomAddOns = async () => {
    if (await vpn.getElementProperty(elements.CUSTOM_ADDON_ROW, 'isChecked') === "false") {
        console.log('checking addon checkbox');
        await vpn.waitForElementAndClick('customAddOnCheckBoxRow/checkbox')
    }
    await vpn.wait(7000)
}

const useStagingServers = async () => {
    if (await vpn.getElementProperty(elements.STAGING_SERVER_ROW, 'isChecked') === "false") {
        console.log('checking staging server checkbox');
        await vpn.waitForElementAndClick(elements.STAGING_SERVER_CHECKBOX);
    }
    await vpn.wait(9000)
}

const goToGetHelpView = async () => {
    await vpn.waitForElement(elements.GET_HELP_MENU_BUTTON);
    await vpn.waitForElementProperty(elements.GET_HELP_MENU_BUTTON, 'visible', 'true');
    await vpn.clickOnElement(elements.GET_HELP_MENU_BUTTON);

    await vpn.wait();
    await vpn.waitForElement(elements.GET_HELPMENU_BACK_BUTTON);
    await vpn.waitForElementProperty(elements.GET_HELPMENU_BACK_BUTTON, 'visible', 'true');
}

const sendText = async (id, text) => {
    await vpn.waitForElement(id)
    await vpn.setElementProperty(id, 'text', 's', text);
}

const clickElement = async (id) => {
    // conditionally wait for element
    // wait for animation if any
    // click
}

module.exports = {
    actions: {
        settings: {
            setConnectionChangeNotification,
            setServerSwitchNotification,
            goToSettings,
            activateDeveloperOptions,
            enableCustomAddOns,
            goToGetHelpView,
            useStagingServers
        },
        locations: {
            selectCountryFromList,
            selectCityFromList
        },
        general: {
            sendText,
            clickElement
        }
    }
}