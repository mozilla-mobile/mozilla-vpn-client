#!/bin/bash

# Get Secrets for building
echo "Fetching Tokens!"
./taskcluster/scripts/get-secret.py -s project/mozillavpn/level-1/browserstack -k user -f user
./taskcluster/scripts/get-secret.py -s project/mozillavpn/level-1/browserstack -k pass -f pass

mkdir -p /builds/worker/artifacts/

curl -u -s $(cat user):$(cat pass) \
    -X POST "https://api-cloud.browserstack.com/app-automate/upload" \
    -F "file=@//$MOZ_FETCHES_DIR/mozillavpn-arm64-v8a-debug.apk" > /builds/worker/artifacts/text.txt