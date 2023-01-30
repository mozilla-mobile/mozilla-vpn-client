#!/bin/bash
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
if [ -f .env ]; then
  . .env
fi

. $(dirname $0)/commons.sh

POSITIONAL=()
COMMIT="HEAD"
MODE="stage"
SAVE="0"

helpFunction() {
  print G "Verify QML content for style and linting errors"
  print N ""
  print N "Usage: $0 [options] [<commit>]"
  print N ""
  print N "options:"
  print N "   -pr          check pull request instead of staged files."
  print N "   -s           save lint result"
  print N "   -c <commit>  check files changed since <commit>"
  print N "   -help        show this message and exit"
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
    -c | --commit)
        COMMIT="$2"
        MODE="stage"
        shift
        shift
        ;;
    *)
        print N "Unsupported argument: $1"
        exit 1
  esac
done

if [[ -n "$QT_HOST_PATH" ]]; then
  QT_QMLLINT_BIN=${QT_HOST_PATH}/bin/qmllint
else
  QT_QMLLINT_BIN=$(qmake6 -query QT_HOST_BINS)/qmllint
fi

if [[ "$MODE" == "pr" ]]; then
  print N "\t Checking for QML Lint in PR $PR_NUMBER"
  QML_FILES=$(gh pr view $PR_NUMBER --json files --jq '.files.[].path' | grep '\.qml$')
else
  print N "\t Checking for QML Lint since ${COMMIT}"
  QML_FILES=$(git diff --name-only $COMMIT | grep '\.qml$')
fi

touch qml_lint_result.txt
if [[ -n "${QML_FILES}" ]]; then
  $QT_QMLLINT_BIN -I nebula/ui -I src/ui ${QML_FILES} 2>> qml_lint_result.txt
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
