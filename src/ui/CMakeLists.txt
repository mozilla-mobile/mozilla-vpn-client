# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

qt_add_qml_module(mozillavpn-ui
    VERSION 1.0
    URI Mozilla.VPN
    NO_PLUGIN_OPTIONAL
    STATIC
    OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Mozilla/VPN
    QML_FILES
        main.qml
        authenticationInApp/ViewAuthenticationFallbackInBrowser.qml
        authenticationInApp/ViewAuthenticationInitializing.qml
        authenticationInApp/ViewAuthenticationSignIn.qml
        authenticationInApp/ViewAuthenticationSignUp.qml
        authenticationInApp/ViewAuthenticationStart.qml
        authenticationInApp/ViewAuthenticationUnblockCodeNeeded.qml
        authenticationInApp/ViewAuthenticationVerificationSessionByEmailNeeded.qml
        authenticationInApp/ViewAuthenticationVerificationSessionByTotpNeeded.qml
        authenticationInApp/ViewAuthenticationStubAccount.qml
        authenticationInApp/ViewAuthenticationSsoAccount.qml
        deleteAccount/ViewDeleteAccount.qml
        deleteAccount/ViewDeleteAccountRequest.qml
        popups/ServerUnavailablePopup.qml
        screens/authenticationInApp/ViewAuthenticationInApp.qml
        screens/devices/ViewDevices.qml
        screens/devices/ViewDeviceLimit.qml
        screens/devices/VPNDeviceList.qml
        screens/devices/VPNDeviceListItem.qml
        screens/devices/VPNRemoveDevicePopup.qml
        screens/getHelp/ViewGetHelp.qml
        screens/getHelp/contactUs/ViewContactUsForm.qml
        screens/getHelp/contactUs/ViewContactUsThankYou.qml
        screens/getHelp/developerMenu/ViewAnimationsPlayground.qml
        screens/getHelp/developerMenu/ViewDeveloperMenu.qml
        screens/getHelp/developerMenu/ViewFeatureList.qml
        screens/getHelp/developerMenu/ViewTelemetryDebugging.qml
        screens/getHelp/developerMenu/ViewThemeList.qml
        screens/getHelp/developerMenu/ViewUiTesting.qml
        screens/home/controller/ConnectionStability.qml
        screens/home/controller/ConnectionTimer.qml
        screens/home/controller/ControllerImage.qml
        screens/home/controller/ControllerNav.qml
        screens/home/controller/ControllerView.qml
        screens/home/controller/ip/IPAddress.qml
        screens/home/controller/ip/IPInfoPanel.qml
        screens/home/controller/VPNToggle.qml
        screens/home/servers/RecentConnections.qml
        screens/home/servers/ServerCountry.qml
        screens/home/servers/ServerLabel.qml
        screens/home/servers/ServerLatencyIndicator.qml
        screens/home/servers/ServerList.qml
        screens/home/servers/ServerListToggle.qml
        screens/home/ViewHome.qml
        screens/home/ViewMultiHop.qml
        screens/home/ViewServers.qml
        screens/initialize/ViewInitialize.qml
        screens/messaging/ViewMessage.qml
        screens/messaging/ViewMessagesInbox.qml
        screens/onboarding/OnboardingDataSlide.qml
        screens/onboarding/OnboardingDevicesSlide.qml
        screens/onboarding/OnboardingPrivacySlide.qml
        screens/onboarding/OnboardingStartSlideDesktop.qml
        screens/onboarding/OnboardingStartSlideMobile.qml
        screens/onboarding/ViewOnboarding.qml
        screens/ScreenRemovingDevice.qml
        screens/ScreenAuthenticating.qml
        screens/ScreenAuthenticationInApp.qml
        screens/ScreenBackendFailure.qml
        screens/ScreenBillingNotAvailable.qml
        screens/ScreenCaptivePortal.qml
        screens/ScreenCrashReporting.qml
        screens/ScreenDeviceLimit.qml
        screens/ScreenGetHelp.qml
        screens/ScreenHome.qml
        screens/ScreenInitialize.qml
        screens/ScreenMessaging.qml
        screens/ScreenNoSubscriptionFoundError.qml
        screens/ScreenOnboarding.qml
        screens/ScreenSettings.qml
        screens/ScreenSubscriptionBlocked.qml
        screens/ScreenSubscriptionGenericError.qml
        screens/ScreenSubscriptionExpiredError.qml
        screens/ScreenSubscriptionInProgressIAP.qml
        screens/ScreenSubscriptionInProgressWeb.qml
        screens/ScreenSubscriptionInUseError.qml
        screens/ScreenSubscriptionNeeded.qml
        screens/ScreenSubscriptionNotValidated.qml
        screens/ScreenUpdateRecommended.qml
        screens/ScreenUpdateRequired.qml
        screens/ScreenViewLogs.qml
        screens/ScreenDeleteAccount.qml
        screens/settings/appPermissions/ViewAppPermissions.qml
        screens/settings/appPermissions/AppPermissionsList.qml
        screens/settings/ViewAboutUs.qml
        screens/settings/ViewDNSSettings.qml
        screens/settings/ViewLanguage.qml
        screens/settings/ViewLicense.qml
        screens/settings/ViewLicenses.qml
        screens/settings/ViewNotifications.qml
        screens/settings/ViewPreferences.qml
        screens/settings/ViewReset.qml
        screens/settings/ViewSettingsMenu.qml
        screens/settings/privacy/ViewPrivacy.qml
        screens/settings/privacy/PrivacyFeaturesList.qml
        screens/settings/ViewSubscriptionManagement/SubscriptionManagementItem.qml
        screens/settings/ViewSubscriptionManagement/ViewReauthenticationFlow.qml
        screens/settings/ViewSubscriptionManagement/ViewSubscriptionManagement.qml
        screens/subscriptionNeeded/ViewSubscriptionNeeded.qml
        sharedViews/ViewErrorFullScreen.qml
        sharedViews/ViewUpdate.qml
        sharedViews/ViewFullScreen.qml
    SOURCES
        composer/composer.cpp
        composer/composer.h
        composer/composerblock.cpp
        composer/composerblock.h
        composer/composerblockbutton.cpp
        composer/composerblockbutton.h
        composer/composerblockorderedlist.cpp
        composer/composerblockorderedlist.h
        composer/composerblocktext.cpp
        composer/composerblocktext.h
        composer/composerblocktitle.cpp
        composer/composerblocktitle.h
        composer/composerblockunorderedlist.cpp
        composer/composerblockunorderedlist.h
        singletons/VPN.h
        singletons/VPNAppPermissions.h
        singletons/VPNCaptivePortal.h
        singletons/VPNConnectionHealth.h
        singletons/VPNController.h
        singletons/VPNCurrentServer.h
        singletons/VPNDeviceModel.h
        singletons/VPNIPAddressLookup.h
        singletons/VPNProducts.h
        singletons/VPNProfileFlow.h
        singletons/VPNPurchase.h
        singletons/VPNRecentConnectionsModel.h
        singletons/VPNRecommendedLocationModel.h
        singletons/VPNReleaseMonitor.h
        singletons/VPNServerCountryModel.h
        singletons/VPNServerLatency.h
        singletons/VPNSubscriptionData.h
        singletons/VPNSupportCategoryModel.h
        singletons/VPNUser.h
        types/MZAddonMessage.h
)

target_link_libraries(mozillavpn-ui PRIVATE
    Qt6::Quick
    Qt6::Widgets
)

target_include_directories(mozillavpn-ui PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_include_directories(mozillavpn-ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/composer
    ${CMAKE_CURRENT_SOURCE_DIR}/singletons
    ${CMAKE_CURRENT_SOURCE_DIR}/types
)
target_include_directories(mozillavpn-uiplugin PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/composer
    ${CMAKE_CURRENT_SOURCE_DIR}/singletons
    ${CMAKE_CURRENT_SOURCE_DIR}/types
)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Android")
    target_sources(mozillavpn-ui PRIVATE singletons/VPNAndroidUtils.h)
endif()
