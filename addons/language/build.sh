#!/bin/bash

DEST=$1
if [ "$DEST" == "" ]; then
  echo "Usage: $0 <dest/dir>"
  exit 1
fi

if ! [ -d "$DEST" ]; then
  echo "Dir $1 does not exist"
  exit 1
fi

for i in i18n/*ts; do
  lrelease -idbased $i
done

rcc language.qrc -o $DEST/language.rcc -binary
