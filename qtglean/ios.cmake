# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# When building for iOS we want to include the Glean iOS SDK on the build as well.
# Instead of using it out of the box, we want it to be linked to the qtglean_ffi target,
# so we generate the SDK oursevels.

enable_language(Swift)
include(${CMAKE_SOURCE_DIR}/scripts/cmake/rustlang.cmake)

set(GLEAN_VENDORED_PATH ${CMAKE_SOURCE_DIR}/3rdparty/glean)

add_library(iosglean SHARED)

if(NOT MSVC AND NOT IOS)
  target_compile_options(iosglean PRIVATE -Wall -Werror -Wno-conversion)
endif()

target_include_directories(iosglean PUBLIC ${CMAKE_SOURCE_DIR})
target_include_directories(iosglean PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(iosglean PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/glean)

set_target_properties(iosglean PROPERTIES
    FRAMEWORK TRUE
    FRAMEWORK_VERSION C
    OUTPUT_NAME "Glean"
    MACOSX_FRAMEWORK_INFO_PLIST "${GLEAN_VENDORED_PATH}/glean-core/ios/Glean/Info.plist"
    MACOSX_FRAMEWORK_IDENTIFIER "${BUILD_IOS_APP_IDENTIFIER}.Glean"
    XCODE_ATTRIBUTE_SWIFT_VERSION "5.0"
    XCODE_ATTRIBUTE_CLANG_ENABLE_MODULES "YES"
    XCODE_ATTRIBUTE_APPLICATION_EXTENSION_API_ONLY "YES"
    XCODE_ATTRIBUTE_MACH_O_TYPE "staticlib"
    XCODE_ATTRIBUTE_SWIFT_OBJC_BRIDGING_HEADER "${GLEAN_VENDORED_PATH}/glean-core/ios/Glean/Glean.h"
    XCODE_ATTRIBUTE_SWIFT_PRECOMPILE_BRIDGING_HEADER "NO"
    PUBLIC_HEADER "${GLEAN_VENDORED_PATH}/glean-core/ios/Glean/Glean.h;${CMAKE_CURRENT_BINARY_DIR}/glean/gleanFFI.h"
)

target_sources(iosglean PRIVATE
    ${GLEAN_VENDORED_PATH}/glean-core/ios/Glean/Config/Configuration.swift
    ${GLEAN_VENDORED_PATH}/glean-core/ios/Glean/Debug/GleanDebugTools.swift
    ${GLEAN_VENDORED_PATH}/glean-core/ios/Glean/Dispatchers.swift
    ${GLEAN_VENDORED_PATH}/glean-core/ios/Glean/Glean.swift
    ${GLEAN_VENDORED_PATH}/glean-core/ios/Glean/GleanMetrics.swift
    ${GLEAN_VENDORED_PATH}/glean-core/ios/Glean/GleanMetrics.swift
    ${GLEAN_VENDORED_PATH}/glean-core/ios/Glean/Info.plist
    ${GLEAN_VENDORED_PATH}/glean-core/ios/Glean/Metrics/BooleanMetric.swift
    ${GLEAN_VENDORED_PATH}/glean-core/ios/Glean/Metrics/CounterMetric.swift
    ${GLEAN_VENDORED_PATH}/glean-core/ios/Glean/Metrics/DatetimeMetric.swift
    ${GLEAN_VENDORED_PATH}/glean-core/ios/Glean/Metrics/EventMetric.swift
    ${GLEAN_VENDORED_PATH}/glean-core/ios/Glean/Metrics/LabeledMetric.swift
    ${GLEAN_VENDORED_PATH}/glean-core/ios/Glean/Metrics/MemoryDistributionMetric.swift
    ${GLEAN_VENDORED_PATH}/glean-core/ios/Glean/Metrics/Ping.swift
    ${GLEAN_VENDORED_PATH}/glean-core/ios/Glean/Metrics/QuantityMetric.swift
    ${GLEAN_VENDORED_PATH}/glean-core/ios/Glean/Metrics/RateMetric.swift
    ${GLEAN_VENDORED_PATH}/glean-core/ios/Glean/Metrics/StringListMetric.swift
    ${GLEAN_VENDORED_PATH}/glean-core/ios/Glean/Metrics/StringMetric.swift
    ${GLEAN_VENDORED_PATH}/glean-core/ios/Glean/Metrics/TextMetric.swift
    ${GLEAN_VENDORED_PATH}/glean-core/ios/Glean/Metrics/TimespanMetric.swift
    ${GLEAN_VENDORED_PATH}/glean-core/ios/Glean/Metrics/TimingDistributionMetric.swift
    ${GLEAN_VENDORED_PATH}/glean-core/ios/Glean/Metrics/UrlMetric.swift
    ${GLEAN_VENDORED_PATH}/glean-core/ios/Glean/Metrics/UuidMetric.swift
    ${GLEAN_VENDORED_PATH}/glean-core/ios/Glean/Net/HttpPingUploader.swift
    ${GLEAN_VENDORED_PATH}/glean-core/ios/Glean/Scheduler/GleanLifecycleObserver.swift
    ${GLEAN_VENDORED_PATH}/glean-core/ios/Glean/Scheduler/MetricsPingScheduler.swift
    ${GLEAN_VENDORED_PATH}/glean-core/ios/Glean/Utils/Logger.swift
    ${GLEAN_VENDORED_PATH}/glean-core/ios/Glean/Utils/Sysctl.swift
    ${GLEAN_VENDORED_PATH}/glean-core/ios/Glean/Utils/Unreachable.swift
    ${GLEAN_VENDORED_PATH}/glean-core/ios/Glean/Utils/Utils.swift
)
target_sources(iosglean PUBLIC
    ${GLEAN_VENDORED_PATH}/glean-core/ios/Glean/Glean.h
)

# Build gleanFFI.h and glean.swift files using UniFFI
# and build the internal Glean metrics file
execute_process(
    COMMAND ${CMAKE_COMMAND} -E env
        ${CARGO_BUILD_TOOL} run --manifest-path ${GLEAN_VENDORED_PATH}/tools/embedded-uniffi-bindgen/Cargo.toml
            -- generate --language swift --out-dir ${CMAKE_CURRENT_BINARY_DIR}/glean
            ${GLEAN_VENDORED_PATH}/glean-core/src/glean.udl
    COMMAND ${CMAKE_COMMAND} -E env
            SOURCE_ROOT=${CMAKE_CURRENT_BINARY_DIR}
            PROJECT=${BUILD_IOS_APP_IDENTIFIER}
        ${GLEAN_VENDORED_PATH}/glean-core/ios/sdk_generator.sh
            -o ${CMAKE_CURRENT_BINARY_DIR}/glean/generated --allow-reserved
            ${GLEAN_VENDORED_PATH}/glean-core/metrics.yaml ${GLEAN_VENDORED_PATH}/glean-core/pings.yaml
    COMMAND_ERROR_IS_FATAL ANY
)
target_sources(iosglean PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/glean/gleanFFI.modulemap
    ${CMAKE_CURRENT_BINARY_DIR}/glean/glean.swift
    ${CMAKE_CURRENT_BINARY_DIR}/glean/generated/Metrics.swift
)
target_sources(iosglean PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}/glean/gleanFFI.h
)

list(APPEND PINGS_LIST ${CMAKE_SOURCE_DIR}/src/shared/telemetry/pings_deprecated.yaml)
list(APPEND PINGS_LIST ${CMAKE_SOURCE_DIR}/src/shared/telemetry/pings.yaml)
list(APPEND METRICS_LIST ${CMAKE_SOURCE_DIR}/src/shared/telemetry/metrics_deprecated.yaml)
list(APPEND METRICS_LIST ${CMAKE_SOURCE_DIR}/src/shared/telemetry/metrics.yaml)

get_filename_component(APPS_DIR ${CMAKE_SOURCE_DIR}/src/apps ABSOLUTE)
file(GLOB APPS_NAMES LIST_DIRECTORIES true RELATIVE ${APPS_DIR} ${APPS_DIR}/*)
foreach(APP ${APPS_NAMES})
    if(EXISTS ${APPS_DIR}/${APP}/telemetry/pings_deprecated.yaml)
        list(APPEND PINGS_LIST ${APPS_DIR}/${APP}/telemetry/pings_deprecated.yaml)
    endif()
    if(EXISTS ${APPS_DIR}/${APP}/telemetry/pings.yaml)
        list(APPEND PINGS_LIST ${APPS_DIR}/${APP}/telemetry/pings.yaml)
    endif()
    if(EXISTS ${APPS_DIR}/${APP}/telemetry/metrics_deprecated.yaml)
        list(APPEND METRICS_LIST ${APPS_DIR}/${APP}/telemetry/metrics_deprecated.yaml)
    endif()
    if(EXISTS ${APPS_DIR}/${APP}/telemetry/metrics.yaml)
        list(APPEND METRICS_LIST ${APPS_DIR}/${APP}/telemetry/metrics.yaml)
    endif()
endforeach()

# We execute this as a command and not a process,
# because different from the Glean internal stuff
# the VPN metrics and pings files can change constantly
# and we want to re-run the command for each build
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/generated/VPNMetrics.swift
    DEPENDS ${PINGS_LIST} ${METRICS_LIST}
    COMMAND ${GLEAN_VENDORED_PATH}/glean-core/ios/sdk_generator.sh -o ${CMAKE_CURRENT_BINARY_DIR}/generated
        ${PINGS_LIST} ${METRICS_LIST}
    # We need to rename otherwise XCode gets confused with the same name file name as the Glean internal metrics
    COMMAND mv ${CMAKE_CURRENT_BINARY_DIR}/generated/Metrics.swift ${CMAKE_CURRENT_BINARY_DIR}/generated/VPNMetrics.swift
)
set_source_files_properties(
    ${CMAKE_CURRENT_BINARY_DIR}/generated/VPNMetrics.swift
    PROPERTIES GENERATED TRUE
)
target_sources(${MAIN_TARGET} PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/generated/VPNMetrics.swift
    ${CMAKE_SOURCE_DIR}/src/shared/platforms/ios/iosgleanbridge.swift
    ${CMAKE_SOURCE_DIR}/src/shared/platforms/ios/iosgleanbridge.mm
    ${CMAKE_SOURCE_DIR}/src/shared/platforms/ios/iosgleanbridge.h
)

target_link_libraries(iosglean PRIVATE qtglean)
target_link_libraries(${MAIN_TARGET} PRIVATE iosglean)
