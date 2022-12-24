# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

exists($$PWD/generated/metrics.cpp) {
  SOURCES += $$PWD/generated/metrics.cpp
  HEADERS += $$PWD/generated/metrics.h
  SOURCES += $$PWD/generated/pings.cpp
  HEADERS += $$PWD/generated/pings.h
}
else{
    error(Glean generated files are missing. Please run `python3 ./vpnglean/glean_parser_ext/run_glean_parser.py`)
}

SOURCES += $$PWD/src/event.cpp
SOURCES += $$PWD/src/pings.cpp

HEADERS += $PWD/include/glean/event.h
HEADERS += $PWD/include/glean/pings.h
HEADERS += $PWD/include/glean/metrictypes.h
