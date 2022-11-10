# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


set_property(TARGET mozillavpn APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/../android/
)

target_link_libraries(mozillavpn PRIVATE
    Qt6::Test
    Qt6::Xml)


target_link_libraries(
    mozillavpn PRIVATE
    -ljnigraphics)

target_sources(mozillavpn PRIVATE 
    platforms/android/androidcontroller.cpp
    platforms/android/androidglean.cpp
    platforms/android/androidiaphandler.cpp
    platforms/android/androidnetworkwatcher.cpp
    platforms/android/androidnotificationhandler.cpp
    platforms/android/androidutils.cpp
    platforms/android/androidvpnactivity.cpp
    platforms/android/androiddatamigration.cpp
    platforms/android/androidappimageprovider.cpp
    platforms/android/androidapplistprovider.cpp
    platforms/android/androidsharedprefs.cpp
    tasks/purchase/taskpurchase.cpp
    platforms/android/androidcontroller.h
    platforms/android/androidglean.h
    platforms/android/androidiaphandler.h
    platforms/android/androidnetworkwatcher.h
    platforms/android/androidnotificationhandler.h
    platforms/android/androidutils.h
    platforms/android/androidvpnactivity.h
    platforms/android/androiddatamigration.h
    platforms/android/androidappimageprovider.h
    platforms/android/androidapplistprovider.h
    platforms/android/androidsharedprefs.h
    tasks/purchase/taskpurchase.h
    platforms/linux/linuxpingsender.cpp 
    platforms/linux/linuxcryptosettings.cpp
    platforms/linux/linuxpingsender.h
)

if(ADJUST_TOKEN)
    message(Adjust SDK enabled)
    # SDK Token present, let's enable that.
    add_compile_definitions("MVPN_ADJUST")
    target_sources(mozillavpn PRIVATE
        adjust/adjustfiltering.cpp
        adjust/adjusthandler.cpp
        adjust/adjustproxy.cpp
        adjust/adjustproxyconnection.cpp
        adjust/adjustproxypackagehandler.cpp
        adjust/adjusttasksubmission.cpp
        adjust/adjustfiltering.h
        adjust/adjusthandler.h
        adjust/adjustproxy.h
        adjust/adjustproxyconnection.h
        adjust/adjustproxypackagehandler.h
        adjust/adjusttasksubmission.h
    )
else()
    if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
        message(${CMAKE_BUILD_TYPE})
        message( FATAL_ERROR "Adjust token cannot be empty for release builds")
    endif()
endif()