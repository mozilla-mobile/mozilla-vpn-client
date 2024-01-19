/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

class QmlQueryComposer {
  constructor(path) {
    this.path = path;
  }

  /**
   * Creates a new QmlQueryComposer with the subquery appended
   * @param {String | QmlQueryComposer} other
   * @returns QmlQueryComposer = QmlQueryComposer+QmlQueryComposer|String
   */
  query(other) {
    if (typeof other === 'string') {
      const path = this.path + other;
      return new QmlQueryComposer(path)
    } else if (typeof other === 'object') {
      if (!other.path) {
        throw new Error('Cannot Append Object - no .path on:' + other);
      }
      const path = this.path + other.path;
      return new QmlQueryComposer(path)
    }
    throw new Error('Unsupported Type: append(string|QmlQueryComposer)');
  }

  visible() {
    return this.prop('visible', true);
  }

  hidden() {
    return this.prop('visible', false);
  }

  enabled() {
    return this.prop('enabled', true);
  }

  disabled() {
    return this.prop('enabled', false);
  }

  checked() {
    return this.prop('checked', true);
  }

  unchecked() {
    return this.prop('checked', false);
  }

  ready() {
    return this.prop('busy', false);
  }

  opened() {
    return this.prop('opened', true);
  }

  prop(propName, propValue = undefined) {
    if (propValue === undefined) {
      return new QmlQueryComposer(`${this.path}{${propName}}`);
    }
    return new QmlQueryComposer(`${this.path}{${propName}=${propValue}}`);
  }

  toString() {
    return this.path;
  }
};

const screenHome = {
  BACK: new QmlQueryComposer('//screenHome-back'),
  CAP_PORTAL_BUTTON: new QmlQueryComposer('//captivePortalAlertActionButton'),
  CONNECTION_INFO_ERROR: new QmlQueryComposer('//connectionInfoError'),
  CONNECTION_INFO_RETRY:
      new QmlQueryComposer('//connectionInfoErrorRetryButton'),
  CONNECTION_INFO_RESTART:
      new QmlQueryComposer('//connectionInfoRestartButton'),
  CONNECTION_INFO_TOGGLE: new QmlQueryComposer('//connectionInfoToggleButton'),
  CONTROLLER_SUBTITLE: new QmlQueryComposer('//controllerSubTitle'),
  CONTROLLER_TITLE: new QmlQueryComposer('//controllerTitle'),
  CONTROLLER_TOGGLE: new QmlQueryComposer('//controllerToggle'),
  IP_INFO_PANEL: new QmlQueryComposer('//ipInfoPanel'),
  IP_INFO_TOGGLE: new QmlQueryComposer('//ipInfoToggleButton'),
  SCREEN: new QmlQueryComposer('//screenHome'),
  SECURE_AND_PRIVATE_SUBTITLE:
      new QmlQueryComposer('//secureAndPrivateSubtitle'),
  SERVER_LIST_BUTTON_LABEL:
      new QmlQueryComposer('//serverListButton-label-label'),
  SERVER_LIST_BUTTON: new QmlQueryComposer('//serverListButton-btn'),
  STACKVIEW: new QmlQueryComposer('//screenHome-stackView'),

  serverListView: {
    generateCountryId: (serverCode) => {
      return new QmlQueryComposer(
          `//serverCountryList/serverCountry-${serverCode}`);
    },

    generateCityId: (countryId, cityName) => {
      return new QmlQueryComposer(
          `${countryId}//serverCity-${cityName.replace(/ /g, '_')}`);
    },

    BACK_BUTTON: new QmlQueryComposer('//serverListBackButton'),
    HELP_BUTTON: new QmlQueryComposer('//serverHelpButton'),
    SERVER_HELP_SHEET: new QmlQueryComposer('//serverHelpSheet'),
    SERVER_HELP_SHEET_CLOSE_BUTTON: new QmlQueryComposer('//serverHelpSheet-closeButton'),
    SERVER_HELP_SHEET_LEARN_MORE_BUTTON: new QmlQueryComposer('//helpSheetContentLoader/learnMoreLink'),
    COUNTRY_VIEW: new QmlQueryComposer('//serverCountryView'),
    ENTRY_BUTTON: new QmlQueryComposer('//buttonSelectEntry'),
    EXIT_BUTTON: new QmlQueryComposer('//buttonSelectExit'),
    SEARCH_BAR: new QmlQueryComposer('//countrySearchBar'),
    SEARCH_BAR_TEXT_FIELD: new QmlQueryComposer('//searchBarTextField'),
    SEARCH_BAR_ERROR: new QmlQueryComposer('//searchBarError'),
    SINGLEHOP_SELECTOR_TAB: new QmlQueryComposer(
        '//segmentedNavToggle/segmentedToggleBtnLayout/tabSingleHop'),
    MULTIHOP_SELECTOR_TAB: new QmlQueryComposer(
        '//segmentedNavToggle/segmentedToggleBtnLayout/tabMultiHop'),
    MULTIHOP_VIEW: new QmlQueryComposer('//multiHopStackView'),
    ALL_SERVERS_TAB: new QmlQueryComposer('//tabAllServers'),
  }
};

const screenTipsAndTricks = {
  VIEW: new QmlQueryComposer('//settingsTipsAndTricksPage'),
}

const screenInitialize = {
  ALREADY_A_SUBSCRIBER_LINK: new QmlQueryComposer('//alreadyASubscriberLink'),
  AUTHENTICATE_VIEW: new QmlQueryComposer('//authenticatingView'),
  GET_HELP_LINK: new QmlQueryComposer('//getHelpLink'),
  PANEL_DESCRIPTION: new QmlQueryComposer('//panelDescription'),
  PANEL_TITLE: new QmlQueryComposer('//panelTitle'),
  SCREEN: new QmlQueryComposer('//initialStackView'),
  SIGN_UP_BUTTON: new QmlQueryComposer('//signUpButton'),
  SWIPE_VIEW: new QmlQueryComposer('//swipeView'),
  SWIPE_VIEW_REPEATER: new QmlQueryComposer('//swipeViewRepeater'),
};

const screenAuthenticating = {
  CANCEL_FOOTER_LINK: new QmlQueryComposer('//cancelFooterLink'),
};

const screenPostAuthentication = {
  BUTTON: new QmlQueryComposer('//postAuthenticationButton'),
};

const screenTelemetry = {
  BUTTON: new QmlQueryComposer('//telemetryPolicyButton'),
  DECLINE_LINK: new QmlQueryComposer('//declineTelemetryLink'),
};

const screenAuthenticationInApp = {
  AUTH_TERMS_OF_SERVICE: new QmlQueryComposer('//termsOfService'),
  AUTH_PRIVACY_NOTICE: new QmlQueryComposer('//privacyPolicy'),

  AUTH_START_TEXT_INPUT: new QmlQueryComposer('//authStart-textInput'),
  AUTH_START_BUTTON: new QmlQueryComposer('//authStart-button'),
  AUTH_START_BACK_BUTTON: new QmlQueryComposer('//authStart-backButton'),
  AUTH_START_GET_HELP_LINK: new QmlQueryComposer('//authStart-getHelpLink'),

  AUTH_SIGNIN_SCREEN: new QmlQueryComposer('//authSignIn'),
  AUTH_SIGNIN_PASSWORD_INPUT:
      new QmlQueryComposer('//authSignIn-passwordInput'),
  AUTH_SIGNIN_BUTTON: new QmlQueryComposer('//authSignIn-button'),
  AUTH_SIGNIN_GET_HELP_LINK: new QmlQueryComposer('//authSignIn-getHelpLink'),
  AUTH_SIGNIN_BACK_BUTTON: new QmlQueryComposer('//authSignIn-backButton'),
  AUTH_SIGNIN_PASSWORD_PASTE_BUTTON:
      new QmlQueryComposer('//authSignIn-inputPasteButton'),
  AUTH_SIGNIN_CANCEL_BUTTON: new QmlQueryComposer('//authSignIn-cancel'),
  AUTH_SIGNIN_CHANGE_EMAIL_BUTTON:
      new QmlQueryComposer('//authSignIn-changeEmail'),
  AUTH_SIGNIN_FORGOT_PASSWORD_BUTTON:
      new QmlQueryComposer('//authSignIn-forgotPassword'),

  AUTH_SIGNUP_PASSWORD_INPUT:
      new QmlQueryComposer('//authSignUp-passwordInput'),
  AUTH_SIGNUP_BUTTON: new QmlQueryComposer('//authSignUp-button'),
  AUTH_SIGNUP_PASSWORD_LENGTH_CONDITION:
      new QmlQueryComposer('//authSignUp-passwordConditionLength'),
  AUTH_SIGNUP_PASSWORD_EMAIL_CONDITION:
      new QmlQueryComposer('//authSignUp-passwordConditionEmailAddress'),
  AUTH_SIGNUP_PASSWORD_COMMON_CONDITION:
      new QmlQueryComposer('//authSignUp-passwordConditionCommon'),
  AUTH_SIGNUP_GET_HELP_LINK: new QmlQueryComposer('//authSignUp-getHelpLink'),
  AUTH_SIGNUP_BACK_BUTTON: new QmlQueryComposer('//authSignUp-backButton'),
  AUTH_SIGNUP_PASSWORD_PASTE_BUTTON:
      new QmlQueryComposer('//authSignUp-inputPasteButton'),
  AUTH_SIGNUP_CANCEL_BUTTON: new QmlQueryComposer('//authSignUp-cancel'),
  AUTH_SIGNUP_BACK_BUTTON: new QmlQueryComposer('//authSignUp-backButton'),

  AUTH_EMAILVER_TEXT_INPUT:
      new QmlQueryComposer('//authVerificationSessionByEmailNeeded-textInput'),
  AUTH_EMAILVER_BUTTON:
      new QmlQueryComposer('//authVerificationSessionByEmailNeeded-button'),
  AUTH_EMAILVER_GET_HELP_LINK: new QmlQueryComposer(
      '//authVerificationSessionByEmailNeeded-getHelpLink'),
  AUTH_EMAILVER_PASTE_BUTTON: new QmlQueryComposer(
      '//authVerificationSessionByEmailNeeded-inputPasteButton'),
  AUTH_EMAILVER_CANCEL_BUTTON:
      new QmlQueryComposer('//authVerificationSessionByEmailNeeded-cancel'),
  AUTH_EMAILVER_RESEND_CODE_BUTTON:
      new QmlQueryComposer('//authVerificationSessionByEmailNeeded-resendCode'),

  AUTH_TOTP_TEXT_INPUT:
      new QmlQueryComposer('//authVerificationSessionByTotpNeeded-textInput'),
  AUTH_TOTP_BUTTON:
      new QmlQueryComposer('//authVerificationSessionByTotpNeeded-button'),
  AUTH_TOTP_GET_HELP_LINK:
      new QmlQueryComposer('//authVerificationSessionByTotpNeeded-getHelpLink'),
  AUTH_TOTP_PASTE_BUTTON: new QmlQueryComposer(
      '//authVerificationSessionByTotpNeeded-inputPasteButton'),
  AUTH_TOTP_CANCEL_BUTTON:
      new QmlQueryComposer('//authVerificationSessionByTotpNeeded-cancel'),

  AUTH_UNBLOCKCODE_TEXT_INPUT:
      new QmlQueryComposer('//authUnblockCodeNeeded-textInput'),
  AUTH_UNBLOCKCODE_BUTTON:
      new QmlQueryComposer('//authUnblockCodeNeeded-button'),
  AUTH_UNBLOCKCODE_GET_HELP_LINK:
      new QmlQueryComposer('//authUnblockCodeNeeded-getHelpLink'),
  AUTH_UNBLOCKCODE_BACK_BUTTON:
      new QmlQueryComposer('//authUnblockCodeNeeded-backButton'),
  AUTH_UNBLOCKCODE_PASTE_BUTTON:
      new QmlQueryComposer('//authUnblockCodeNeeded-inputPasteButton'),
  AUTH_UNBLOCKCODE_CANCEL_BUTTON:
      new QmlQueryComposer('//authUnblockCodeNeeded-cancel'),
  AUTH_UNBLOCKCODE_RESEND_CODE_BUTTON:
      new QmlQueryComposer('//authUnblockCodeNeeded-resendCode'),

  AUTH_STUB_SET_PASSWORD_HEADLINE:
      new QmlQueryComposer('//authStubAccount-headline'),
  AUTH_STUB_SET_PASSWORD_SIGN_IN_BUTTON:
      new QmlQueryComposer('//authStubAccount-buttonSignIn'),
  AUTH_SSO_SET_PASSWORD_HEADLINE:
      new QmlQueryComposer('//authSsoAccount-headline'),
  AUTH_SSO_SET_PASSWORD_SIGN_IN_BUTTON:
      new QmlQueryComposer('//authSsoAccount-buttonSignIn'),

  AUTH_ERROR_POPUP_BUTTON: new QmlQueryComposer('//authErrorPopup-button'),
};

const screenGetHelp = {
  BACK_BUTTON: new QmlQueryComposer('//getHelpBack'),
  HELP_CENTER: new QmlQueryComposer('//helpCenter'),
  LINKS: new QmlQueryComposer('//getHelpLinks'),
  LOGS: new QmlQueryComposer('//viewLogs'),
  DEVELOPER_MENU: new QmlQueryComposer('//developer'),
  STACKVIEW: new QmlQueryComposer('//getHelpStackView'),
  SUPPORT: new QmlQueryComposer('//inAppSupport'),

  contactSupportView: {
    UNAUTH_USER_INPUTS: new QmlQueryComposer('//contactUs-unauthedUserInputs'),
    USER_INFO: new QmlQueryComposer('//contactUs-userInfo'),
  }
};

const screenDeveloperMenu = {
  SCREEN: new QmlQueryComposer('//developerScreen-flickable'),
  RESET_AND_QUIT_BUTTON: new QmlQueryComposer('//resetAndQuitButton'),
};

const appExclusionsView = {
  ADD_APPLICATION_BUTTON: new QmlQueryComposer('//addApplication'),
  APP_LIST: new QmlQueryComposer('//appList'),
  APP_ROW1: new QmlQueryComposer('//app-0'),
  CHECKBOX1: new QmlQueryComposer('//app-0/checkbox'),
  CHECKBOX2: new QmlQueryComposer('//app-1/checkbox'),
  CLEAR_ALL: new QmlQueryComposer('//clearAll'),
  SCREEN: new QmlQueryComposer('//appPermissions')
};

const screenSettings = {
  ABOUT_US: new QmlQueryComposer('//settingsAboutUs'),
  BACK: new QmlQueryComposer('//settings-back'),
  GET_HELP: new QmlQueryComposer('//settingsGetHelp'),
  APP_EXCLUSIONS: new QmlQueryComposer('//appExclusionSettings'),
  PRIVACY: new QmlQueryComposer('//privacySettings'),
  MY_DEVICES: new QmlQueryComposer('//settingsDevice'),
  SCREEN: new QmlQueryComposer('//settingsView-flickable'),
  SIGN_OUT: new QmlQueryComposer('//settingsLogout'),
  STACKVIEW: new QmlQueryComposer('//settings-stackView'),
  APP_PREFERENCES: new QmlQueryComposer('//settingsPreferences'),
  TIPS_AND_TRICKS: new QmlQueryComposer('//settingsTipsAndTricks'),
  USER_PROFILE:
      new QmlQueryComposer('//settingsUserProfile'),
  USER_PROFILE_DISPLAY_NAME:
      new QmlQueryComposer('//settingsUserProfile-displayName'),
  USER_PROFILE_EMAIL_ADDRESS:
      new QmlQueryComposer('//settingsUserProfile-emailAddress'),

  privacyView: {
    BLOCK_ADS: new QmlQueryComposer('//blockAds'),
    BLOCK_ADS_CHECKBOX: new QmlQueryComposer('//blockAds//checkbox'),
    BLOCK_TRACKERS: new QmlQueryComposer('//blockTrackers'),
    BLOCK_TRACKERS_CHECKBOX: new QmlQueryComposer('//blockTrackers//checkbox'),
    BLOCK_MALWARE: new QmlQueryComposer('//blockMalware'),
    BLOCK_MALWARE_CHECKBOX: new QmlQueryComposer('//blockMalware//checkbox'),

    INFORMATION_CARD:
        new QmlQueryComposer('//privacySettingsViewInformationCard'),

    MODAL_LOADER: new QmlQueryComposer('//privacyOverwriteLoader'),
    MODAL_CLOSE_BUTTON:
        new QmlQueryComposer('//privacyOverwritePopupPopupCloseButton'),
    MODAL_PRIMARY_BUTTON:
        new QmlQueryComposer('//privacyOverwritePopupDiscoverNowButton'),
    MODAL_SECONDARY_BUTTON:
        new QmlQueryComposer('//privacyOverwritePopupGoBackButton'),

    VIEW_PRIVACY_WARNING: new QmlQueryComposer('//viewPrivacyWarning'),

    HELP_BUTTON: new QmlQueryComposer('//privacyHelpButton'),
    PRIVACY_HELP_SHEET: new QmlQueryComposer('//privacyHelpSheet'),
    PRIVACY_HELP_SHEET_CLOSE_BUTTON: new QmlQueryComposer('//privacyHelpSheet-closeButton'),
    PRIVACY_HELP_SHEET_LEARN_MORE_BUTTON: new QmlQueryComposer('//helpSheetContentLoader/learnMoreLink'),
  },

  myDevicesView: {
    BACK: new QmlQueryComposer('//deviceList-back'),
    CONFIRM_REMOVAL_BUTTON: new QmlQueryComposer('//confirmRemoveDeviceButton'),
    DEVICE_LIST: new QmlQueryComposer('//deviceList'),
    DEVICE_LIMIT_HEADER: new QmlQueryComposer('//deviceLimitHeader'),
    REMOVE_DEVICE_BUTTON: new QmlQueryComposer(
        '//deviceList/deviceListLayout/device-device_1/swipeActionLoader/swipeActionDelete'),

  },

  tipsAndTricksView: {
    BACK: new QmlQueryComposer('//tipsAndTricks-back'),
    SCREEN: new QmlQueryComposer('//settingsTipsAndTricksPage'),
  },

  appPreferencesView: {
    START_AT_BOOT: new QmlQueryComposer('//settingStartAtBoot'),
    DATA_COLLECTION: new QmlQueryComposer('//dataCollection'),
    LANGUAGE: new QmlQueryComposer('//settingsLanguages'),
    NOTIFICATIONS: new QmlQueryComposer('//settingsNotifications'),
    DNS_SETTINGS: new QmlQueryComposer('//dnsSettings'),

    dnsSettingsView: {
      STANDARD_DNS: new QmlQueryComposer('//dnsStandard'),
      CUSTOM_DNS: new QmlQueryComposer('//dnsCustom'),
      CUSTOM_DNS_INPUT: new QmlQueryComposer('//dnsCustomInput'),
      INFORMATION_CARD_LOADER:
          new QmlQueryComposer('//DNSSettingsInformationCardLoader'),

      INFORMATION_CARD:
          new QmlQueryComposer('//DNSSettingsViewInformationCard'),

      MODAL_LOADER: new QmlQueryComposer('//dnsOverwriteLoader'),
      MODAL_CLOSE_BUTTON:
          new QmlQueryComposer('//dnsOverwritePopupPopupCloseButton'),
      MODAL_PRIMARY_BUTTON:
          new QmlQueryComposer('//dnsOverwritePopupDiscoverNowButton'),
      MODAL_SECONDARY_BUTTON:
          new QmlQueryComposer('//dnsOverwritePopupGoBackButton'),

      HELP_BUTTON: new QmlQueryComposer('//dnsHelpButton'),
      PRIVACY_HELP_SHEET: new QmlQueryComposer('//dnsHelpSheet'),
      PRIVACY_HELP_SHEET_CLOSE_BUTTON: new QmlQueryComposer('//dnsHelpSheet-closeButton'),
      PRIVACY_HELP_SHEET_LEARN_MORE_BUTTON: new QmlQueryComposer('//helpSheetContentLoader/learnMoreLink'),
    },

    languageSettingsView: {
      languageItem: function(lang) {
        return new QmlQueryComposer(`//languageList/language-column-${lang}`);
      },
      languageItemLabel: function(lang) {
        return new QmlQueryComposer(
            `//languageList/language-column-${lang}/language-${lang}`);
      },

      SCREEN: new QmlQueryComposer('//settingsLanguagesView-flickable'),
      SYSTEM_LANGUAGE_RADIO_BUTTON:
          new QmlQueryComposer('//systemLanguageRadioButton'),
    },

    notificationView: {
      CAPTIVE_PORTAL_ALERT: new QmlQueryComposer('//settingCaptivePortalAlert'),
      CONNECTION_CHANGE_ALERT: new QmlQueryComposer('//connectionChangeAlert'),
      SERVER_UNAVAILABLE_ALERT:
          new QmlQueryComposer('//serverUnavailableNotification'),
      SWITCH_SERVER_ALERT: new QmlQueryComposer('//switchServersAlert'),
      UNSECURE_NETWORK_ALERT:
          new QmlQueryComposer('//settingUnsecuredNetworkAlert'),
    },
  },

  aboutUsView: {
    LICENSE: new QmlQueryComposer('//aboutUsList/aboutUsList-license'),
    LIST: new QmlQueryComposer('//aboutUsList'),
    PRIVACY: new QmlQueryComposer('//aboutUsList/aboutUsList-privacy'),
    SCREEN: new QmlQueryComposer('//viewAboutUs'),
    TOS: new QmlQueryComposer('//aboutUsList/aboutUsList-tos'),
  },

  subscriptionView: {
    ACCOUNT_DELETION: new QmlQueryComposer('//accountDeletionButton'),
    ANNUAL_UPGRADE: new QmlQueryComposer(
        '//subscriptionItem/subscriptionItem-plan/subscriptionItem-plan-parent/subscriptionItem-plan-upgradeToAnnualSub-layout'),
    ANNUAL_UPGRADE_BUTTON: new QmlQueryComposer(
        '//subscriptionItem/subscriptionItem-plan/subscriptionItem-plan-parent/subscriptionItem-plan-upgradeToAnnualSub-layout/subscriptionItem-plan-upgradeToAnnualSub-upgradeButton'),
    PLAN: new QmlQueryComposer('//subscriptionItem-plan-valueText'),
    SCREEN: new QmlQueryComposer('//subscriptionManagmentView'),
    FLICKABLE: new QmlQueryComposer('//subscriptionManagmentView-flickable'),
    STATUS_PILL: new QmlQueryComposer('//subscriptionItem-status-pill'),

    ACTIVATED: new QmlQueryComposer(
        '//subscriptionItem/subscriptionItem-activated/subscriptionItem-activated-parent/subscriptionItem-activated-container/subscriptionItem-activated-valueText'),
    CANCELLED: new QmlQueryComposer(
        '//subscriptionItem/subscriptionItem-cancelled/subscriptionItem-cancelled-parent/subscriptionItem-cancelled-container/subscriptionItem-cancelled-valueText'),
    CANCELLED_LABEL: new QmlQueryComposer(
        '//subscriptionItem/subscriptionItem-cancelled/subscriptionItem-cancelled-parent/subscriptionItem-cancelled-container/subscriptionItem-cancelled-labelText'),
    BRAND: new QmlQueryComposer(
        '//subscriptionItem/subscriptionItem-brand/subscriptionItem-brand-parent/subscriptionItem-brand-container/subscriptionItem-brand-valueText'),
    EXPIRES: new QmlQueryComposer(
        '//subscriptionItem/subscriptionItem-expires/subscriptionItem-expires-parent/subscriptionItem-expires-container/subscriptionItem-expires-valueText'),
    PAYMENT_METHOD: new QmlQueryComposer(
        '//subscriptionItem/subscriptionItem-brand/subscriptionItem-brand-parent/subscriptionItem-brand-container/subscriptionItem-brand-paymentMethod/paymentLabel'),
    PAYMENT_METHOD_LABEL: new QmlQueryComposer(
        '//subscriptionItem/subscriptionItem-payment/subscriptionItem-payment-parent/subscriptionItem-payment-container/subscriptionItem-payment-paymentMethod/paymentLabel'),

    SUBSCRIPTION_USER_PROFILE_DISPLAY_NAME:
        new QmlQueryComposer('//subscriptionUserProfile-displayName'),
    SUBSCRIPTION_USER_PROFILE_EMAIL_ADDRESS:
        new QmlQueryComposer('//subscriptionUserProfile-emailAddress'),
    SUBSCRIPTION_USER_PROFILE_BUTTON_ACCOUNT:
        new QmlQueryComposer('//subscriptionUserProfile-manageAccountButton'),
    SUBSCRIPTION_USER_PROFILE_BUTTON_SUB:
        new QmlQueryComposer('//manageSubscriptionButton'),
  },
};

const screenBackendFailure = {
  HEARTBEAT_TRY_BUTTON: new QmlQueryComposer('//heartbeatTryButton'),
};

const screenMessaging = {
  SCREEN: new QmlQueryComposer('//messageInboxView'),
  messageItem: function(id) {
    return new QmlQueryComposer(`//messageItem-${id}`);
  },
  messageView: function(id) {
    return new QmlQueryComposer(`//messageView-${id}`);
  },
};

const screenDeleteAccount = {
  BUTTON: new QmlQueryComposer('//deleteAccountForRealButton'),
  LABEL: new QmlQueryComposer('//accountDeletionLabel'),
  SCREEN: new QmlQueryComposer('//viewDeleteAccountRequest'),

  CHECKBOX1: new QmlQueryComposer(
      '//accountDeletionLayout/accountDeletionCheckbox-check1'),
  CHECKBOX1_CB: new QmlQueryComposer(
      '//accountDeletionLayout/accountDeletionCheckbox-check1/checkbox'),
  CHECKBOX2: new QmlQueryComposer(
      '//accountDeletionLayout/accountDeletionCheckbox-check2'),
  CHECKBOX2_CB: new QmlQueryComposer(
      '//accountDeletionLayout/accountDeletionCheckbox-check2/checkbox'),
  CHECKBOX3: new QmlQueryComposer(
      '//accountDeletionLayout/accountDeletionCheckbox-check3'),
  CHECKBOX3_CB: new QmlQueryComposer(
      '//accountDeletionLayout/accountDeletionCheckbox-check3/checkbox'),
  CHECKBOX4: new QmlQueryComposer(
      '//accountDeletionLayout/accountDeletionCheckbox-check4'),
  CHECKBOX4_CB: new QmlQueryComposer(
      '//accountDeletionLayout/accountDeletionCheckbox-check4/checkbox'),
};

const screenSubscriptionNeeded = {
  SUBSCRIPTION_NEEDED_VIEW: new QmlQueryComposer('//vpnSubscriptionNeededView'),
  SUBSCRIPTION_NEEDED_BUTTON:
      new QmlQueryComposer('//vpnSubscriptionNeededButton'),
  SUBSCRIPTION_NEEDED_GET_HELP:
      new QmlQueryComposer('//vpnSubscriptionNeededGetHelp'),
  SUBSCRIPTION_NEEDED_TERMS_OF_SERVICE:
      new QmlQueryComposer('//vpnSubscriptionNeededTermsOfService'),
  SUBSCRIPTION_NEEDED_PRIVACY_NOTICE:
      new QmlQueryComposer('//vpnSubscriptionNeededPrivacyNotice'),
  SUBSCRIPTION_NEEDED_SIGN_OUT:
      new QmlQueryComposer('//vpnSubscriptionNeededSignOut'),
  SUBSCRIPTION_NEEDED_RESTORE_PURCHASE:
      new QmlQueryComposer('//vpnSubscriptionNeededRestorePurchase'),
};

const screenInBrowserSubscriptionLoading = {
  SUBSCRIPTION_LOADING_CANCEL: new QmlQueryComposer('//cancelFooterLink'),
  SUBSCRIPTION_LOADING_VIEW: new QmlQueryComposer('//subscriptionInProgressWeb'),
};

const screenOnboarding = {
  SCREEN: new QmlQueryComposer('//screenOnboarding'),
  ONBOARDING_VIEW: new QmlQueryComposer('//viewOnboarding'),
  STEP_NAV_STACK_VIEW: new QmlQueryComposer('//stepNavStackView'),
  STEP_PROG_BAR_DATA_BUTTON: new QmlQueryComposer('//progressBarRow/data'),
  STEP_PROG_BAR_PRIVACY_BUTTON: new QmlQueryComposer('//progressBarRow/privacy'),
  STEP_PROG_BAR_DEVICES_BUTTON: new QmlQueryComposer('//progressBarRow/devices'),
  DATA_SLIDE: new QmlQueryComposer('//onboardingDataSlide'),
  DATA_CHECKBOX_ROW: new QmlQueryComposer('//dataCollectionCheckBox'),
  DATA_CHECKBOX: new QmlQueryComposer('//dataCollectionCheckBox/checkbox'),
  DATA_PRIVACY_LINK: new QmlQueryComposer('//dataPrivacyLink'),
  DATA_NEXT_BUTTON: new QmlQueryComposer('//dataNextButton'),
  PRIVACY_SLIDE: new QmlQueryComposer('//onboardingPrivacySlide'),
  PRIVACY_NEXT_BUTTON: new QmlQueryComposer('//privacyNextButton'),
  PRIVACY_BLOCK_ADS_CHECKBOX: new QmlQueryComposer('//blockAds//checkbox'),
  PRIVACY_BLOCK_TRACKERS_CHECKBOX: new QmlQueryComposer('//blockTrackers//checkbox'),
  PRIVACY_BLOCK_MALWARE_CHECKBOX: new QmlQueryComposer('//blockMalware//checkbox'),
  PRIVACY_BACK_BUTTON: new QmlQueryComposer('//privacyBackButton'),
  DEVICES_SLIDE: new QmlQueryComposer('//onboardingDevicesSlide'),
  DEVICES_TOGGLE_BTN_ANDROID: new QmlQueryComposer('//segmentedToggleBtnLayout/tabAndroid'),
  DEVICES_TOGGLE_BTN_IOS: new QmlQueryComposer('//segmentedToggleBtnLayout/tabApple'),
  DEVICES_DEVICE_TYPE_TOGGLE: new QmlQueryComposer('//deviceTypeToggle'),
  DEVICES_PLAY_STORE_QRCODE: new QmlQueryComposer('//playStoreQrCode'),
  DEVICES_APP_STORE_QRCODE: new QmlQueryComposer('//appStoreQrCode'),
  DEVICES_NEXT_BUTTON: new QmlQueryComposer('//devicesNextButton'),
  DEVICES_BACK_BUTTON: new QmlQueryComposer('//devicesBackButton'),
  START_SLIDE: new QmlQueryComposer('//onboardingStartSlide'),
  START_START_AT_BOOT_CHECKBOX: new QmlQueryComposer('//startAtBootCheckBox//checkbox'),
  START_NEXT_BUTTON: new QmlQueryComposer('//startNextButton'),
  START_BACK_BUTTON: new QmlQueryComposer('//startBackButton'),
};

const navBar = {
  HOME: new QmlQueryComposer('//navigationLayout/navButton-home'),
  MESSAGES: new QmlQueryComposer('//navigationLayout/navButton-messages'),
  SETTINGS: new QmlQueryComposer('//navigationLayout/navButton-settings'),
};

const global = {
  SCREEN_LOADER: new QmlQueryComposer('//screenLoader'),
};

module.exports = {
  appExclusionsView,
  screenHome,
  screenInitialize,
  screenPostAuthentication,
  screenTelemetry,
  screenAuthenticationInApp,
  screenAuthenticating,
  screenDeveloperMenu,
  screenGetHelp,
  screenSettings,
  screenBackendFailure,
  screenTipsAndTricks,
  screenMessaging,
  screenDeleteAccount,
  screenSubscriptionNeeded,
  screenInBrowserSubscriptionLoading,
  screenOnboarding,
  global,
  navBar,
}
