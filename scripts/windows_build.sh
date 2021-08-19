#!/bin/sh
set -x
set -e

# Dependencies
python3 scripts/tooltool.py fetch -m windows/qt-dependencies.tt.manifest
python3 scripts/tooltool.py fetch -m windows/vs2017-15.9.tt.manifest
# TODO: internal pypi mirror, if it doesn't get used by default?
python3 -m virtualenv build
build/scripts/pip install glean_parser==3.5 pyyaml

# Build
build/scripts/python scripts/generate_glean.py
cmd.exe ./scripts/windows_compile.bat

# Package
mkdir unsigned
# copy things to package staging area

# Copy artifact
cd unsigned
zip -r ../build/src/artifacts/unsigned.zip .
