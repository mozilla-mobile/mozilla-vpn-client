# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

get_filename_component(MZ_SHARED_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/shared ABSOLUTE)

target_sources(shared-sources INTERFACE
     ${MZ_SHARED_SOURCE_DIR}/platforms/macos/macoscryptosettings.mm
     ${MZ_SHARED_SOURCE_DIR}/platforms/ios/ioscommons.h
     ${MZ_SHARED_SOURCE_DIR}/platforms/ios/ioscommons.mm
)

# Include the Adjust SDK
if(BUILD_ADJUST_SDK_TOKEN)
    add_compile_definitions(MZ_ADJUST)
    target_compile_options(shared-sources INTERFACE -DADJUST_SDK_TOKEN=${BUILD_ADJUST_SDK_TOKEN})
    target_sources(shared-sources INTERFACE
        ${MZ_SHARED_SOURCE_DIR}/adjust/adjustfiltering.cpp
        ${MZ_SHARED_SOURCE_DIR}/adjust/adjustfiltering.h
        ${MZ_SHARED_SOURCE_DIR}/adjust/adjusthandler.cpp
        ${MZ_SHARED_SOURCE_DIR}/adjust/adjusthandler.h
        ${MZ_SHARED_SOURCE_DIR}/adjust/adjustproxy.cpp
        ${MZ_SHARED_SOURCE_DIR}/adjust/adjustproxy.h
        ${MZ_SHARED_SOURCE_DIR}/adjust/adjustproxyconnection.cpp
        ${MZ_SHARED_SOURCE_DIR}/adjust/adjustproxyconnection.h
        ${MZ_SHARED_SOURCE_DIR}/adjust/adjustproxypackagehandler.cpp
        ${MZ_SHARED_SOURCE_DIR}/adjust/adjustproxypackagehandler.h
        ${MZ_SHARED_SOURCE_DIR}/adjust/adjusttasksubmission.cpp
        ${MZ_SHARED_SOURCE_DIR}/adjust/adjusttasksubmission.h
        ${MZ_SHARED_SOURCE_DIR}/platforms/ios/iosadjusthelper.mm
        ${MZ_SHARED_SOURCE_DIR}/platforms/ios/iosadjusthelper.h
    )
    target_link_libraries(shared-sources INTERFACE adjust)
endif()
