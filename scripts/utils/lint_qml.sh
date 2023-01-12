#!/bin/bash
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
if [ -f .env ]; then
  . .env
fi

. $(dirname $0)/commons.sh

POSITIONAL=()
MODE="stage"
SAVE="0"

helpFunction() {
  print G "Usage:"
  print N "\t$0  -pr | check pull request instead of staged files."
  print N "\t$0  -s  | save lint result"
  print N "\t$0  -help | show this help text"
  print N ""
  exit 0
}

while [[ $# -gt 0 ]]; do
  key="$1"

  case $key in
    -pr | -p )
        MODE="pr"
        shift
        ;;
    -h | --help)
        helpFunction
        ;;
    -s | --save)
        SAVE="1"
        shift
        ;;
  *)
    if [[ "$QT_HOST_PATH" ]]; then
      helpFunction
    fi
    QT_HOST_PATH="$1"
    shift
    ;;
  esac
done



if [[ "$MODE" == "pr" ]]; then
  print N "\t Calling QML Lint for PR"
  $QT_HOST_PATH/bin/qmllint -I nebula/ui -I src/ui $(gh pr view $PR_NUMBER --json files --jq '.files.[].path' | grep ".qml") 2> qml_lint_result.txt
else
  print N "\t Calling QML Lint staged git files"
  $QT_HOST_PATH/bin/qmllint -I nebula/ui -I src/ui $(git diff --name-only --cached | grep ".qml") 2> qml_lint_result.txt
fi

# Check if the file contains any warnings
! grep -q "Warning:" qml_lint_result.txt 
RESULT=$?
# Re print for CI
cat qml_lint_result.txt
# In case it was not asked to keep, delete.
if [[ "$SAVE" == "0" ]]; then
  rm qml_lint_result.txt
fi

exit $RESULT