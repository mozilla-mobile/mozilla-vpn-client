# Adjust data collection

This document contains the list of data collected by the adjust.sdk framework on mobile.
Currently these fields are sent:

- adid: main adjust generated identifier
- app_token: What identifies one adjust app from another
- att_status: determing apple ATT status
- attribution_deeplink: attributing to deeplink-based engagements
- bundle_id: apple universal linking, some partner requests, general troubleshooting
- device_type: to determine if it's request is a tablet or phone
- environment: production or sandbox (so test data vs live data)
- event token: distinguishes what that event is in adjust (is it a purchase, or a signup for your app)
- idfv
- needs response details: sdk <> backend communication
- os_name: tell us whether it's android, ios, or web
- os_version
- package name: same as bundle id
- reftag / reference tag: adjust click id
- tracking enabled: LAT flag, legacy OS's will still have this value.
- zone offset: part of the created_at timestamp

Some other fields are replaced with dummy values. For the list, see the
`denyList` vector in adjustfiltering.cpp
