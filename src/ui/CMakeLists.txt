# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

qt_add_qml_module(mozillavpn-ui
    VERSION 1.0
    URI Mozilla.VPN
    RESOURCE_PREFIX /qt/qml #TODO: Remove this once we only compile with 6.5+
    STATIC
    OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Mozilla/VPN
    QML_FILES
        main.qml
        popups/MessageAddonPopover.qml
        popups/ServerUnavailablePopup.qml
        screens/devices/ViewDevices.qml
        screens/devices/ViewDeviceLimit.qml
        screens/devices/VPNDeviceList.qml
        screens/devices/VPNDeviceListItem.qml
        screens/devices/VPNRemoveDevicePopup.qml
        screens/getHelp/ViewGetHelp.qml
        screens/getHelp/contactUs/ViewContactUsForm.qml
        screens/getHelp/contactUs/ViewContactUsThankYou.qml
        screens/getHelp/developerMenu/ViewColorObjects.qml
        screens/getHelp/developerMenu/ViewImagesPlayground.qml
        screens/getHelp/developerMenu/ViewDeveloperMenu.qml
        screens/getHelp/developerMenu/ViewFeatureList.qml
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
        screens/ScreenPermissionRequired.qml
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
        screens/settings/appPermissions/ViewAppPermissions.qml
        screens/settings/appPermissions/AppPermissionsList.qml
        screens/settings/ViewAboutUs.qml
        screens/settings/ViewAppearance.qml
        screens/settings/ViewDNSSettings.qml
        screens/settings/ViewFirefoxExtensionInfo.qml
        screens/settings/ViewLanguage.qml
        screens/settings/ViewLicense.qml
        screens/settings/ViewLicenses.qml
        screens/settings/ViewNotifications.qml
        screens/settings/ViewPreferences.qml
        screens/settings/ViewReset.qml
        screens/settings/ViewSettingsMenu.qml
        screens/settings/privacy/ViewPrivacy.qml
        screens/settings/privacy/PrivacyFeaturesList.qml
        screens/subscriptionNeeded/ViewSubscriptionNeeded.qml
        sharedViews/ViewErrorFullScreen.qml
        sharedViews/ViewPermissionRequiredOSX.qml
        sharedViews/ViewUpdate.qml
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
    mzutils
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

target_sources(mozillavpn-ui PRIVATE singletons/BatteryOptimizer.h)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Android")
    target_compile_definitions(mozillavpn-ui PRIVATE MZ_ANDROID)
    target_sources(mozillavpn-ui PRIVATE singletons/VPNAndroidUtils.h)
    target_sources(mozillavpn-ui PRIVATE singletons/VPNAndroidCommons.h)
endif()
if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    target_sources(mozillavpn-ui PRIVATE singletons/VPNMacOSUtils.h)
endif()


# The onboarding animations are plain QML generated from the Lottie JSON. When
# the Qt lottietoqml generator is available (Qt 6.11+) we regenerate it at build
# time; otherwise we fall back to the checked-in copies under animations/. Either
# way no Lottie runtime is bundled.
include(${CMAKE_SOURCE_DIR}/src/cmake/lottietoqml.cmake)
find_package(Qt6 QUIET COMPONENTS LottieTools)
if(TARGET Qt6::lottietoqml)
    message(STATUS "Onboarding animations: regenerating QML from Lottie JSON with Qt6::lottietoqml")
    mz_target_qml_from_lottie(
        mozillavpn-ui
        FILES
           resources/animations/globe_animation.json
           resources/animations/lock_animation.json
           resources/animations/vpnactive_animation.json
           resources/animations/vpnlogo-drop_animation.json
        OUTPUTS
           animations/GlobeAnimation.qml
           animations/LockAnimation.qml
           animations/VpnActiveAnimation.qml
           animations/VpnLogoDropAnimation.qml
    )
else()
    message(STATUS "Onboarding animations: Qt6::lottietoqml not found, using checked-in QML under src/ui/animations/")
    qt_target_qml_sources(mozillavpn-ui
        QML_FILES
           animations/GlobeAnimation.qml
           animations/LockAnimation.qml
           animations/VpnActiveAnimation.qml
           animations/VpnLogoDropAnimation.qml
    )
endif()