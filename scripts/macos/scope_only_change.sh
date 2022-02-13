#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

lottie="$(git diff lottie/ | wc -l)"
qml_src="$(git diff src/ui | wc -l)"
qml_neb="$(git diff nebula/ui | wc -l)"
cpp="$(git diff src/ | wc -l)"
changeCount=$(($lottie + $qml_neb + $qml_src + $cpp))

if [ $lottie -gt 0 ]; then
    echo "number of changes in lottie code: $lottie"
    echo "running lottie unit tests"
    ./scripts/macos/lottie_tests.sh
fi

if [ $cpp -gt 0 ]; then
    echo "number of changes in Main C++ code: $cpp"
    echo "running unit tests"    
    ./scripts/macos/unit_tests.sh
fi

if [[ $qml_src -gt 0 || $qml_neb -gt 0 ]]; then
    echo "number of changes in qml src code: $qml_src"
    echo "number of changes in qml nebula code: $qml_neb"
    echo "run qml tests"    
    ./scripts/macos/qml_tests.sh
fi

if [ $changeCount -lt 1 ]; then
    echo "There are no changes in the src, nebula or qml ui"
fi
