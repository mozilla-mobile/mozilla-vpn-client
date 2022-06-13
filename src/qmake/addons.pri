# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

addonGen.input = ADDONS
addonGen.output = ${QMAKE_FILE_IN}/generated/addons/manifest.qrc
addonGen.commands = python3 $$PWD/../../scripts/addon/fetch_manifest.py
addonGen.CONFIG = target_predeps no_link
addonGen.variable_out = RESOURCES

QMAKE_EXTRA_COMPILERS += addonGen
ADDONS = $$PWD/../../addons
