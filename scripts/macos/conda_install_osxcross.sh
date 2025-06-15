#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

conda install -y -c conda-forge \
    cctools_osx-64=973.0.1 \
    cctools_osx-arm64=973.0.1 \
    clang_osx-64=16.0.6 \
    clang_osx-arm64=16.0.6 \
    clangxx_osx-64=16.0.6 \
    clangxx_osx-arm64=16.0.6
