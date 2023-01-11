#!/bin/bash

curl -u $BROWSERSTACK_USERNAME:$BROWSERSTACK_ACCESS_KEY \
    -X POST "https://api-cloud.browserstack.com/app-automate/upload" \
    -F "file=@//$MOZ_FETCHES_DIR/mozillavpn-arm64-v8a-debug.apk" \
    -F "custom_id=mozillavpn-arm64-v8a-debug"