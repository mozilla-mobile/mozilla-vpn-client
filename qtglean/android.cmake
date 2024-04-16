# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

include(${CMAKE_SOURCE_DIR}/scripts/cmake/rustlang.cmake)

set(GLEAN_VENDORED_PATH ${CMAKE_SOURCE_DIR}/3rdparty/glean)

execute_process(
    COMMAND ${CMAKE_COMMAND} -E env
        ${CARGO_BUILD_TOOL} run --manifest-path ${GLEAN_VENDORED_PATH}/tools/embedded-uniffi-bindgen/Cargo.toml
            -- generate --language kotlin --out-dir ${CMAKE_CURRENT_BINARY_DIR}/../glean
            ${GLEAN_VENDORED_PATH}/glean-core/src/glean.udl
    COMMAND ${PYTHON_EXECUTABLE} -m glean_parser translate -f kotlin
            -o ${CMAKE_CURRENT_BINARY_DIR}/../glean/generated  -s "glean_namespace=mozilla.telemetry.glean" -s "namespace=mozilla.telemetry.glean.GleanMetrics" --allow-reserved
            ${GLEAN_VENDORED_PATH}/glean-core/metrics.yaml ${GLEAN_VENDORED_PATH}/glean-core/pings.yaml
    COMMAND ${PYTHON_EXECUTABLE} -m glean_parser translate -f kotlin
        -o ${CMAKE_CURRENT_BINARY_DIR}/../clienttelemetry -s "glean_namespace=mozilla.telemetry.glean" -s "namespace=org.mozilla.firefox.vpn.qt.GleanMetrics"
            ${CMAKE_SOURCE_DIR}/src/telemetry/pings.yaml
            ${CMAKE_SOURCE_DIR}/src/telemetry/metrics.yaml
    # Yes, this is exactly the same as above. But notice that namespace is different.
    COMMAND ${PYTHON_EXECUTABLE} -m glean_parser translate -f kotlin
            -o ${CMAKE_CURRENT_BINARY_DIR}/../daemontelemetry -s "glean_namespace=mozilla.telemetry.glean" -s "namespace=org.mozilla.firefox.vpn.daemon.GleanMetrics"
                ${CMAKE_SOURCE_DIR}/src/telemetry/pings.yaml
                ${CMAKE_SOURCE_DIR}/src/telemetry/metrics.yaml
    COMMAND_ERROR_IS_FATAL ANY
)
