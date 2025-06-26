#!/bin/bash
set -e

BASE_PREFIX=$(conda info --base)
if [[ "$CONDA_PREFIX" == "$BASE_PREFIX" ]]; then
    echo "Please run this script inside a non-base conda environment."
    exit 1
fi

## In our .env we ship a lot of stuff that we do not need
## Let's remove that. 

## Saves another 1gb
sdkmanager --uninstall --sdk_root=$ANDROID_HOME emulator
## Clean the PKG cache, cleans 7gb ~~

conda clean -a -y 
