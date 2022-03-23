#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

. $(dirname $0)/commons.sh

print N "This script generates the i18n servers.json"
print N ""

if [ "$1" == "" ] || ! [ -f "$1" ]; then
  print G "Usage:"
  print N "\t$0 /path/mozillavpn"
  exit 1
fi

if ! [ -d "src" ] || ! [ -d "tests" ]; then
  die "This script must be executed at the root of the repository."
fi

printn Y "Retrieving mozillavpn version... "
"$1" -v 2>/dev/null || die "Failed."
print G "done."

print Y "Running the app..."
"$1" ui --testing &>/tmp/VPN_LOG.txt &
PID=$!
print G "done."

print Y "Running the localization script..."

export SERVER_OUTPUT=translations/servers.json
export SERVER_API=translations/servers-api.json
export SERVER_TEMPLATE=translations/servers-template.json

mocha tests/functional/localizeServers.js || ERROR=yes

wait $PID

if [ "$ERROR" = yes ]; then
  cat /tmp/VPN_LOG.txt
  print R "Nooo"
  exit 1
fi

echo "All done! The final server list can be found here: $SERVER_OUTPUT"
echo "This is the merging of the generated server names ($SERVER_API) and the template file ($SERVER_TEMPLATE)."
