# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(shared-sources INTERFACE
     ${CMAKE_CURRENT_SOURCE_DIR}/shared/platforms/macos/macoscryptosettings.mm
     ${CMAKE_CURRENT_SOURCE_DIR}/shared/platforms/ios/ioscommons.h
     ${CMAKE_CURRENT_SOURCE_DIR}/shared/platforms/ios/ioscommons.mm
)

# Include the Adjust SDK
if(BUILD_ADJUST_SDK_TOKEN)
    add_compile_definitions(MZ_ADJUST)
    target_compile_options(shared-sources INTERFACE -DADJUST_SDK_TOKEN=${BUILD_ADJUST_SDK_TOKEN})
    target_sources(shared-sources INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/adjust/adjustfiltering.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/adjust/adjustfiltering.h
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/adjust/adjusthandler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/adjust/adjusthandler.h
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/adjust/adjustproxy.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/adjust/adjustproxy.h
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/adjust/adjustproxyconnection.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/adjust/adjustproxyconnection.h
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/adjust/adjustproxypackagehandler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/adjust/adjustproxypackagehandler.h
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/adjust/adjusttasksubmission.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/adjust/adjusttasksubmission.h
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/platforms/ios/iosadjusthelper.mm
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/platforms/ios/iosadjusthelper.h
    )
    target_link_libraries(shared-sources INTERFACE adjust)
endif()
