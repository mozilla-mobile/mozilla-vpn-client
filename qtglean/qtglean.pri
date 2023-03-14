# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

exists($$PWD/prebuilt/glean/generated/metrics.cpp) {
  SOURCES += $$PWD/prebuilt/glean/generated/metrics.cpp
  HEADERS += $$PWD/prebuilt/glean/generated/metrics.h
  SOURCES += $$PWD/prebuilt/glean/generated/pings.cpp
  HEADERS += $$PWD/prebuilt/glean/generated/pings.h
  INCLUDEPATH += $$PWD/prebuilt
}
else{
    error(Glean generated files are missing. Please run `python3 ./qtglean/glean_parser_ext/run_glean_parser.py`)
}

SOURCES += $$PWD/src/cpp/boolean.cpp
SOURCES += $$PWD/src/cpp/counter.cpp
SOURCES += $$PWD/src/cpp/event.cpp
SOURCES += $$PWD/src/cpp/ping.cpp
SOURCES += $$PWD/src/cpp/timingdistribution.cpp

HEADERS += $$PWD/include/glean/boolean.h
HEADERS += $$PWD/include/glean/counter.h
HEADERS += $$PWD/include/glean/event.h
HEADERS += $$PWD/include/glean/ping.h
HEADERS += $$PWD/include/glean/timingdistribution.h
HEADERS += $$PWD/include/glean/metrictypes.h
INCLUDEPATH += $$PWD/include
