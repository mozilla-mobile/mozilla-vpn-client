#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

. $(dirname $0)/../utils/commons.sh

if [ -f .env ]; then
  . .env
fi

export AUTOGRAPH_URL=https://edge.prod.autograph.services.mozaws.net/sign

helpFunction() {
  print G "Usage:"
  print N "\t$0 <path to *.apk>"
  print N ""
  print N ""
  exit 0
}

if [ -z "$1" ]
then
      helpFunction
      die "No File Path Provided :c"
fi



if [ -z "$AUTOGRAPH_TOKEN" ]
then
      print N "Could not find 'AUTOGRAPH_TOKEN'"
      die "Export it or put it into .env"
fi

cd $1

for i in *.apk; do
  UNSIGNED_BUILD=$i
  SIGNED_BUILD=$(echo $i | sed -e 's/unsigned/signed/')
  print N "Signing $i"
  curl -H "Authorization: $AUTOGRAPH_TOKEN" -F "input=@$UNSIGNED_BUILD" -o $SIGNED_BUILD $AUTOGRAPH_URL
done

print G "All Done ! :)"
