#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.



# should already be done by XCode cloud cloning but just to make sure
git submodule init || die
git submodule update || die

pip3 install -r requirements.txt
python3 scripts/utils/import_languages.py
python3 scripts/utils/generate_glean.py
# Delete unit tests, so we can get to testing faster
sed -i '/tests\/unit/d' mozillavpn.pro
qmake6 CONFIG+=DUMMY QMAKE_CXXFLAGS+=--coverage QMAKE_LFLAGS+=--coverage CONFIG+=debug CONFIG+=inspector QT+=svg
make -j$(nproc)

cp ./src/mozillavpn build/
cp -r ./src/.obj build/