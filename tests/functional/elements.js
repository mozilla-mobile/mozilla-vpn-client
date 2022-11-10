/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const generalElements = {
    CONTROLLER_TITLE: "controllerTitle",
    CONTROLLER_SUBTITLE: 'controllerSubTitle',
    CONTROLLER_TOGGLE: 'controllerToggle'
}

const elementState = {
    COUNTRY_VIEW: "serverCountryView",
    CITYLIST_VISIBLE: "cityListVisible",
}

const navBar = {
    NAVIGATION_BAR: 'navigationBar',
    HOME: 'navigationLayout/navButton-home',
    MESSAGES: 'navigationLayout/navButton-messages',
    SETTINGS: 'navigationLayout/navButton-settings',
}

const telemetryScreen = {
    TELEMETRY_POLICY_BUTTON: 'telemetryPolicyButton',
    DECLINE_TELEMETRY: 'declineTelemetryLink',
    POST_AUTHENTICATION_BUTTON: 'postAuthenticationButton'
}

/*
    Settings screen and related screens
*/
const settingsScreen = {
    SETTINGS: 'settingsView',
    SCREEN: 'settingsView-flickable', // or SETTINGS_VIEW: 'settingsView',
    USER_PROFILE: 'settingsUserProfile-manageAccountButton',
    TIPS_AND_TRICKS: 'settingsTipsAndTricks',
    NETWORK_SETTINGS: 'settingsNetworking',
    SYSTEM_PREFERENCE: 'settingsPreferences',
    MY_DEVICE: 'settingsDeviceList',
    GET_HELP: 'settingsGetHelp',
    ABOUT_US: 'settingsAboutUs',
    SIGN_OUT: 'settingsLogout',
    BACK: 'settings-back',

    tipsAndTricksView: {
        SCREEN: 'settingsTipsAndTricksPage',
        BACK: 'tipsAndTricks-back'
    },

    networkSettingsView: {
        SCREEN: 'settingsView-flickable',
    },

    systemPreferenceView: {
        LANGUAGE: 'settingsLanguages',
        NOTIFICATIONS: 'settingsNotifications',

        languageSettingsView: {
            systemLanguageToggle: 'settingsSystemLanguageToggle'
        }
    },

    aboutUsView: {
        SCREEN: 'viewAboutUs',
        LIST: 'aboutUsList',
        TOS: 'aboutUsList/aboutUsList-tos',
        PRIVACY: 'aboutUsList/aboutUsList-privacy',
        LICENSE: 'aboutUsList/aboutUsList-license'
    },

    myDevicesView: {
        BACK: 'deviceList-back',
        DEVICE_LIST: 'deviceListView',
        DEVICE_LIMIT_HEADER: 'deviceLimitHeader',
        CONFIRM_REMOVAL_BUTTON: 'confirmRemoveDeviceButton'
    }
}

const getHelpScreen = {
    BACK: 'getHelpBack',
    LINKS: 'getHelpLinks',
    HELP_CENTER: 'helpCenter',
    SUPPORT: 'inAppSupport',
    LOGS: 'viewLogs',
    FEEDBACK: 'settingsGiveFeedback',

    giveFeedbackView: {
        SCREEN: 'giveFeedbackView'
    },

    contactSupportView: {
        USER_INFO: 'contactUs-userInfo',
        UNAUTH_USER_INPUTS: 'contactUs-unauthedUserInputs'
    }
}

/*
    In-app messaging screen and related screens
*/
const inAppMessagingScreen = {
    SCREEN: 'messageInboxView'
}

/*
    Home related screens
*/
const homeScreen = {
    HOME_SCREEN: 'screenHome',
    SERVER_LIST_BUTTON: "serverListButton",
    HOME_SCREEN_BACK: 'screenHome-back',
    CANCEL_FOOTER_LINK: 'cancelFooterLink',
    CONNECTION_INFO_TOGGLE: 'connectionInfoToggleButton',
    CONNECTION_BENCHMARK: 'VPNConnectionBenchmark',
    CONNECTION_INFO_RETRY: 'connectionInfoErrorRetryButton',

    // tips and tricks
    TIPS_AND_TRICKS_VIEW: 'settingsTipsAndTricksPage',
    TIPS_AND_TRICKS_POPUP_LOADER: 'tipsAndTricksIntroPopupLoader',
    TIPS_AND_TRICKS_POPUP_CLOSE: 'tipsAndTricksIntroPopupCloseButton',
    TIPS_AND_TRICKS_POPUP_DISCOVER: 'tipsAndTricksIntroPopupDiscoverNowButton',
    TIPS_AND_TRICKS_POPUP_BACK: 'tipsAndTricksIntroPopupGoBackButton',

    // tutorials
    TUTORIAL_POPUP_PRIMARY_BUTTON: 'tutorialPopupPrimaryButton',
    TUTORIAL_POPUP_SECONDARY_BUTTON: 'tutorialPopupSecondaryButton',
    TUTORIAL_UI: 'tutorialUiRoot',
    TUTORIAL_LEAVE: 'tutorialLeave',
    TUTORIAL_LIST_HIGHLIGHT: 'layoutAll/columnAll/tutorialsSectionAll/tipsAndTricksSetionLoader/tutorialList/highlightedTutorial',

    selectSingleHopServerView: {
        BACK_BUTTON: "serverListBackButton",
        MULTIHOP_SELECTOR_TAB: "multiHopSelector/tabMultiHop",
        SINGLEHOP_SELECTOR_TAB: "multiHopSelector/tabSingleHop",
    },
    
    selectMultiHopServerView: {
        VPN_MULTHOP_CHEVRON: 'vpnCollapsibleCardChevron',
        VPN_COLLAPSIBLE_CARD: 'vpnCollapsibleCard',
        ENTRY_BUTTON: "buttonSelectEntry",
        EXIT_BUTTON: "buttonSelectExit",
    },

    serverListView: {
        generateCountryId: (serverCode) => {
            return 'serverCountryList/serverCountry-' + serverCode;
        },
    
        generateCityId: (countryId, cityName) => {
            return countryId + '/serverCityList/serverCity-' + cityName.replace(/ /g, '_');
        }
    }
}

const initialScreen = {
    SCREEN: 'initialStackView',
    GET_HELP_LINK: 'getHelpLink',
    GET_STARTED: 'getStarted',
    LEARN_MORE_LINK: 'learnMoreLink',
    SKIP_ONBOARDING: 'skipOnboarding',
    ONBOARDING_NEXT: 'onboardingNext',
    AUTHENTICATE_VIEW: 'authenticatingView',
    CAP_PORTAL_BUTTON: 'captivePortalAlertActionButton',
}

const authScreen = {
    EMAIL_INPUT: 'authStart-textInput',
    EMAIL_INPUT_CONDITIOIN: 'authSignUp-passwordConditionEmailAddress',
    START_BUTTON: 'authStart-button',
    START_BACK_BUTTON: 'authStart-backButton',
    SIGNIN_PASS_INPUT: 'authSignIn-passwordInput',
    SIGNUP_PASS_INPUT: 'authSignUp-passwordInput',
    PASS_INPUT_CONDITION: 'authSignUp-passwordConditionCommon',
    PASS_LENGTH_CONDITION: 'authSignUp-passwordConditionLength',
    SIGNIN_BUTTON: 'authSignIn-button',
    SIGNUP_BUTTON: 'authSignUp-button',
    SIGNIN_BACK: 'authSignIn-backButton',
    SIGNUP_BACK: 'authSignUp-backButton',
    SIGNUP_GET_HELP_LINK: 'authSignUp-getHelpLink',
    SIGNIN_GET_HELP_LINK: 'authSignIn-getHelpLink',
    ERROR_POPUP_BUTTON: 'authErrorPopup-button',
    GET_HELP_LINK: 'authStart-getHelpLink',

    verificationByEmailView: {
        TEXT_INPUT: 'authVerificationSessionByEmailNeeded-textInput',
        BACK: 'authVerificationSessionByEmailNeeded-backButton',
        GET_HELP: 'authVerificationSessionByEmailNeeded-getHelpLink',
        EMAIL_BUTTON: 'authVerificationSessionByEmailNeeded-button',
    },

    verificationBySessionTotpView: {
        GET_HELP_LINK: 'authVerificationSessionByTotpNeeded-getHelpLink',
        BACK: 'authVerificationSessionByTotpNeeded-backButton',
        TEXT_INPUT: 'authVerificationSessionByTotpNeeded-textInput',
        TOTP_NEEDED_BUTTON: 'authVerificationSessionByTotpNeeded-button'
    },

    authUnblockCodeView: {
        GET_HELP_LINK: 'authUnblockCodeNeeded-getHelpLink',
        BACK: 'authUnblockCodeNeeded-backButton',
        CODE_INPUT: 'authUnblockCodeNeeded-textInput',
        CODE_BUTTON: 'authUnblockCodeNeeded-button',
    }
}

const mobileOnBoardingScreen = {
    SCREEN: 'mobileOnboarding',
    SWIPE_VIEW: 'swipeView',
    SIGNUP_BUTTON: 'signUpButton',
    ALREADY_SUBBED_LINK: 'alreadyASubscriberLink',
    PANEL_TITLE: 'panelTitle',
    PANEL_DESCRIPTION: 'panelDescription'
}

module.exports = {
    mobileOnBoardingScreen,
    initialScreen,
    generalElements,
    navBar,
    elementState,
    telemetryScreen,
    authScreen,
    settingsScreen,
    getHelpScreen,
    inAppMessagingScreen,
    homeScreen,    
}