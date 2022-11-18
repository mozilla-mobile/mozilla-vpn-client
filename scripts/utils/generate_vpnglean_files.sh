#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e

# Generates the C++ and Rust metrics and pings files.

WORKSPACE_ROOT="$( cd "$(dirname "$0")/../.." ; pwd -P )"

GLEAN_YAML_FILES_PATH="${WORKSPACE_ROOT}/glean"
GLEAN_GENERATED_FILES_PATH="${WORKSPACE_ROOT}/vpnglean/src/generated"

mkdir -p $GLEAN_GENERATED_FILES_PATH

## Generate C++ files
python3 ${WORKSPACE_ROOT}/vpnglean/glean_parser_ext/run_glean_parser.py cpp $GLEAN_GENERATED_FILES_PATH/pings.h $GLEAN_YAML_FILES_PATH/pings.yaml
python3 ${WORKSPACE_ROOT}/vpnglean/glean_parser_ext/run_glean_parser.py cpp $GLEAN_GENERATED_FILES_PATH/metrics.h $GLEAN_YAML_FILES_PATH/metrics.yaml
## Generate Rust files
python3 ${WORKSPACE_ROOT}/vpnglean/glean_parser_ext/run_glean_parser.py rust $GLEAN_GENERATED_FILES_PATH/pings.rs $GLEAN_YAML_FILES_PATH/pings.yaml
python3 ${WORKSPACE_ROOT}/vpnglean/glean_parser_ext/run_glean_parser.py rust $GLEAN_GENERATED_FILES_PATH/metrics.rs $GLEAN_YAML_FILES_PATH/metrics.yaml
