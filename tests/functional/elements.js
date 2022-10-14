/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const elements = {
    MULTIHOP_SELECTOR_TAB: "multiHopSelector/tabMultiHop",
    SINGLEHOP_SELECTOR_TAB: "multiHopSelector/tabSingleHop",
    SERVERLIST_BACK_BUTTON: "serverListBackButton",
    SERVER_ENTRY_BUTTON: "buttonSelectEntry",
    SERVER_EXIT_BUTTON: "buttonSelectExit",
    SERVER_LIST_BUTTON: "serverListButton",
    SERVER_COUNTRY_VIEW: "serverCountryView",
    CONTROLLER_TITLE: "controllerTitle",
    CITYLIST_VISIBLE: "cityListVisible",
    VPN_MULTHOP_CHEVRON: 'vpnCollapsibleCardChevron',
    VPN_COLLAPSIBLE_CARD: 'vpnCollapsibleCard',
    NAVIGATION_BAR: 'navigationBar',
    NAVIGATION_BAR_HOME: 'navigationLayout/navButton-home',
    NAVIGATION_BAR_MESSAGES: 'navigationLayout/navButton-messages',
    NAVIGATION_BAR_SETTINGS: 'navigationLayout/navButton-settings',
    VPN_BOLD_LABEL: 'vpnBoldLabel',
    DEVELOPER_OPTIONS_BUTTON: 'developer',
    CUSTOM_ADDON_ROW: 'customAddOnCheckBoxRow',
    CUSTOM_ADDON_CHECKBOX: 'customAddOnCheckBoxRow/checkbox',
    STAGING_SERVER_ROW: 'stagingServerCheckBoxRow',
    STAGING_SERVER_CHECKBOX: 'stagingServerCheckBoxRow/checkbox',
    GET_HELP_MENU_BUTTON: 'settingsGetHelp',
    GET_HELPMENU_BACK_BUTTON: 'getHelpBack',

    generateCountryId: (serverCode) => {
        return 'serverCountryList/serverCountry-' + serverCode;
    },

    generateCityId: (countryId, cityName) => {
        return countryId + '/serverCityList/serverCity-' + cityName.replace(/ /g, '_');
    }    
}
 

module.exports = elements