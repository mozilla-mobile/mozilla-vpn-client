# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(adjust STATIC)

if(NOT MSVC AND NOT IOS)
  target_compile_options(adjust-sources PRIVATE -Wall -Werror -Wno-conversion)
endif()

set_target_properties(adjust PROPERTIES FOLDER "Libs")

set(ADJUST_SOURCE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/adjust-ios-sdk)

target_include_directories(adjust PUBLIC ${ADJUST_SOURCE_DIR}/Adjust)
target_include_directories(adjust PRIVATE ${ADJUST_SOURCE_DIR}/Adjust/ADJAdditions)
target_compile_options(adjust PRIVATE -fobjc-arc)
target_sources(adjust PRIVATE
    ${ADJUST_SOURCE_DIR}/Adjust/ADJActivityHandler.h
    ${ADJUST_SOURCE_DIR}/Adjust/ADJActivityKind.h
    ${ADJUST_SOURCE_DIR}/Adjust/ADJActivityPackage.h
    ${ADJUST_SOURCE_DIR}/Adjust/ADJActivityState.h
    ${ADJUST_SOURCE_DIR}/Adjust/ADJAdjustFactory.h
    ${ADJUST_SOURCE_DIR}/Adjust/ADJAdRevenue.h
    ${ADJUST_SOURCE_DIR}/Adjust/ADJAttribution.h
    ${ADJUST_SOURCE_DIR}/Adjust/ADJAttributionHandler.h
    ${ADJUST_SOURCE_DIR}/Adjust/ADJBackoffStrategy.h
    ${ADJUST_SOURCE_DIR}/Adjust/ADJAdditions/NSData+ADJAdditions.h
    ${ADJUST_SOURCE_DIR}/Adjust/ADJAdditions/NSNumber+ADJAdditions.h
    ${ADJUST_SOURCE_DIR}/Adjust/ADJAdditions/NSString+ADJAdditions.h
    ${ADJUST_SOURCE_DIR}/Adjust/ADJConfig.h
    ${ADJUST_SOURCE_DIR}/Adjust/ADJEvent.h
    ${ADJUST_SOURCE_DIR}/Adjust/ADJEventFailure.h
    ${ADJUST_SOURCE_DIR}/Adjust/ADJEventSuccess.h
    ${ADJUST_SOURCE_DIR}/Adjust/ADJLinkResolution.h
    ${ADJUST_SOURCE_DIR}/Adjust/ADJLogger.h
    ${ADJUST_SOURCE_DIR}/Adjust/ADJPackageBuilder.h
    ${ADJUST_SOURCE_DIR}/Adjust/ADJPackageHandler.h
    ${ADJUST_SOURCE_DIR}/Adjust/ADJPackageParams.h
    ${ADJUST_SOURCE_DIR}/Adjust/ADJRequestHandler.h
    ${ADJUST_SOURCE_DIR}/Adjust/ADJResponseData.h
    ${ADJUST_SOURCE_DIR}/Adjust/ADJSdkClickHandler.h
    ${ADJUST_SOURCE_DIR}/Adjust/ADJSessionFailure.h
    ${ADJUST_SOURCE_DIR}/Adjust/ADJSessionParameters.h
    ${ADJUST_SOURCE_DIR}/Adjust/ADJSessionSuccess.h
    ${ADJUST_SOURCE_DIR}/Adjust/ADJSubscription.h
    ${ADJUST_SOURCE_DIR}/Adjust/ADJThirdPartySharing.h
    ${ADJUST_SOURCE_DIR}/Adjust/ADJTimerCycle.h
    ${ADJUST_SOURCE_DIR}/Adjust/ADJTimerOnce.h
    ${ADJUST_SOURCE_DIR}/Adjust/ADJUrlStrategy.h
    ${ADJUST_SOURCE_DIR}/Adjust/ADJUserDefaults.h
    ${ADJUST_SOURCE_DIR}/Adjust/Adjust.h
    ${ADJUST_SOURCE_DIR}/Adjust/ADJUtil.h
    ${ADJUST_SOURCE_DIR}/Adjust/ADJActivityHandler.m
    ${ADJUST_SOURCE_DIR}/Adjust/ADJActivityKind.m
    ${ADJUST_SOURCE_DIR}/Adjust/ADJActivityPackage.m
    ${ADJUST_SOURCE_DIR}/Adjust/ADJActivityState.m
    ${ADJUST_SOURCE_DIR}/Adjust/ADJAdjustFactory.m
    ${ADJUST_SOURCE_DIR}/Adjust/ADJAdRevenue.m
    ${ADJUST_SOURCE_DIR}/Adjust/ADJAttribution.m
    ${ADJUST_SOURCE_DIR}/Adjust/ADJAttributionHandler.m
    ${ADJUST_SOURCE_DIR}/Adjust/ADJBackoffStrategy.m
    ${ADJUST_SOURCE_DIR}/Adjust/ADJAdditions/NSData+ADJAdditions.m
    ${ADJUST_SOURCE_DIR}/Adjust/ADJAdditions/NSNumber+ADJAdditions.m
    ${ADJUST_SOURCE_DIR}/Adjust/ADJAdditions/NSString+ADJAdditions.m
    ${ADJUST_SOURCE_DIR}/Adjust/ADJConfig.m
    ${ADJUST_SOURCE_DIR}/Adjust/ADJEvent.m
    ${ADJUST_SOURCE_DIR}/Adjust/ADJEventFailure.m
    ${ADJUST_SOURCE_DIR}/Adjust/ADJEventSuccess.m
    ${ADJUST_SOURCE_DIR}/Adjust/ADJLinkResolution.m
    ${ADJUST_SOURCE_DIR}/Adjust/ADJLogger.m
    ${ADJUST_SOURCE_DIR}/Adjust/ADJPackageBuilder.m
    ${ADJUST_SOURCE_DIR}/Adjust/ADJPackageHandler.m
    ${ADJUST_SOURCE_DIR}/Adjust/ADJPackageParams.m
    ${ADJUST_SOURCE_DIR}/Adjust/ADJRequestHandler.m
    ${ADJUST_SOURCE_DIR}/Adjust/ADJResponseData.m
    ${ADJUST_SOURCE_DIR}/Adjust/ADJSdkClickHandler.m
    ${ADJUST_SOURCE_DIR}/Adjust/ADJSessionFailure.m
    ${ADJUST_SOURCE_DIR}/Adjust/ADJSessionParameters.m
    ${ADJUST_SOURCE_DIR}/Adjust/ADJSessionSuccess.m
    ${ADJUST_SOURCE_DIR}/Adjust/ADJSubscription.m
    ${ADJUST_SOURCE_DIR}/Adjust/ADJThirdPartySharing.m
    ${ADJUST_SOURCE_DIR}/Adjust/ADJTimerCycle.m
    ${ADJUST_SOURCE_DIR}/Adjust/ADJTimerOnce.m
    ${ADJUST_SOURCE_DIR}/Adjust/ADJUrlStrategy.m
    ${ADJUST_SOURCE_DIR}/Adjust/ADJUserDefaults.m
    ${ADJUST_SOURCE_DIR}/Adjust/Adjust.m
    ${ADJUST_SOURCE_DIR}/Adjust/ADJUtil.m
)
