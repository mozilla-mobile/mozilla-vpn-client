#!/bin/bash

# Get Secrets for building
echo "Fetching Tokens!"
./taskcluster/scripts/get-secret.py -s project/mozillavpn/level-1/browserstack -k user -f user
./taskcluster/scripts/get-secret.py -s project/mozillavpn/level-1/browserstack -k pass -f pass

curl -u $(cat user):$(cat pass) \
    -X POST "https://api-cloud.browserstack.com/app-automate/upload" \
    -F "file=@//$MOZ_FETCHES_DIR/mozillavpn-arm64-v8a-debug.apk" \
    -F "custom_id=mozillavpn-arm64-v8a-debug"