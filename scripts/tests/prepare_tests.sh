#!/bin/bash
if [[ ! -f ".env" ]]
then
    echo ".env file does not exist"
    touch .env
    cat << 'EOF' > .env
        export PATH=$PATH:~/Qt/6.2.4/macos/bin:$PATH
        export QT_MACOS_BIN=~/Qt/6.2.4/macos/bin
        MVPN_API_BASE_URL=http://localhost:5000
        MVPN_BIN=build/tests/dummyvpn/dummyvpn
        ARTIFACT_DIR=tests/artifact
EOF
fi

# check if build directory exists
if [[ ! -d "./build/tests/dummyvpn" ]]
then
    echo "dummyvpn dir does not exist, run cmake command to build it"
    cmake -B build .
fi

# rebuild if changes in the /src
if [[ ! $(git status --porcelain --untracked-files=no | wc -l) -eq "0" ]]; then
    echo "  🔴 Changes detected. Please rebuild dummyvpn."
fi

# create dummyvpn app if it doesnt exist
if [[ ! -f "./build/tests/dummyvpn/dummyvpn" ]]
then
    echo "dummyvpn does not exist, run cmake command to create it"
    cmake --build build -j$(nproc) --target dummyvpn
fi

# install node modules to run functional tests if it doesn't exist
if [[ ! -d "./node_modules" ]]
then
    echo "node modules does not exist, run npm install"
    npm install
fi

# generate addons to run tests if they dont exist
if [[ ! -d "./tests/functional/addons/01_empty_manifest" ]]
then
    echo "addons manifests do not exist, run python script to generate it"
    ./scripts/addon/generate_all_tests.py
fi