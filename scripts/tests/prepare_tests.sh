#!/bin/bash
if [[ ! -f ".env" ]]
then
    echo ".env file does not exist"
    echo "please create a .env file that with similar data:
        MVPN_API_BASE_URL=http://localhost:5000
        MVPN_BIN={your build path}/tests/dummyvpn/dummyvpn
        ARTIFACT_DIR=tests/artifact"
    exit 1
fi

# install node modules to run functional tests if it doesn't exist
if [[ ! -d "./node_modules" ]]
then
    echo "🔴 node modules does not exist, run npm install"
    npm install
fi

# generate addons to run tests if they dont exist
if [[ ! -d "./tests/functional/addons/01_empty_manifest" ]]
then
    echo "🔴 addons manifests do not exist, run python script to generate it"
    ./scripts/addon/generate_all_tests.py
fi