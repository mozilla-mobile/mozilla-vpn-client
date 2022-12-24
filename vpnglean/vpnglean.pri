# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

SOURCES += $$PWD/src/event.cpp
SOURCES += $$PWD/src/ping.cpp

HEADERS += $$PWD/include/glean/event.h
HEADERS += $$PWD/include/glean/ping.h
HEADERS += $$PWD/include/glean/metrictypes.h

INCLUDEPATH += $$OBJECTS_DIR
INCLUDEPATH += $$PWD/include

PYTHON_BIN = $$system(which python3)
isEmpty(PYTHON_BIN) {
    PYTHON_BIN = python3
}
message(Using python: $$PYTHON_BIN)

## Custom compiler to invoke the glean code generator.
vpnglean_generator.input = GLEAN_METRICS
vpnglean_generator.output = $${OBJECTS_DIR}/glean/generated/.buildstamp
vpnglean_generator.commands = @echo Generating glean code from ${QMAKE_FILE_IN} \
    && $$PYTHON_BIN $$PWD/glean_parser_ext/run_glean_parser.py \
        --outdir $${OBJECTS_DIR}/glean/generated \
    && touch $${OBJECTS_DIR}/glean/generated/vpnglean-buildstamp.h
vpnglean_generator.variable_out = HEADERS

GLEAN_METRICS = $$PWD/../glean/metrics.yaml
QMAKE_EXTRA_COMPILERS += vpnglean_generator

## Dummy rules for the generated metrics code.
vpnglean_metric_header.input = GLEAN_METRICS
vpnglean_metric_header.output = $${OBJECTS_DIR}/glean/generated/metrics.h
vpnglean_metric_header.commands = $$escape_expand(\\n)  # force creation of rule
vpnglean_metric_header.depends += $${OBJECTS_DIR}/glean/generated/vpnglean-buildstamp.h
vpnglean_metric_header.variable_out = HEADERS

vpnglean_metric_sources.input = GLEAN_METRICS
vpnglean_metric_sources.output = $${OBJECTS_DIR}/glean/generated/metrics.cpp
vpnglean_metric_sources.commands = $$escape_expand(\\n)  # force creation of rule
vpnglean_metric_sources.depends += $${OBJECTS_DIR}/glean/generated/vpnglean-buildstamp.h
vpnglean_metric_sources.variable_out = SOURCES

QMAKE_EXTRA_COMPILERS += vpnglean_metric_header vpnglean_metric_sources

## Dummy rules for the generated pings code.
vpnglean_pings_header.input = GLEAN_METRICS
vpnglean_pings_header.output = $${OBJECTS_DIR}/glean/generated/pings.h
vpnglean_pings_header.commands = $$escape_expand(\\n)  # force creation of rule
vpnglean_pings_header.depends += $${OBJECTS_DIR}/glean/generated/vpnglean-buildstamp.h
vpnglean_pings_header.variable_out = HEADERS

vpnglean_pings_sources.input = GLEAN_METRICS
vpnglean_pings_sources.output = $${OBJECTS_DIR}/glean/generated/pings.cpp
vpnglean_pings_sources.commands = $$escape_expand(\\n)  # force creation of rule
vpnglean_pings_sources.depends += $${OBJECTS_DIR}/glean/generated/vpnglean-buildstamp.h
vpnglean_pings_sources.variable_out = SOURCES

QMAKE_EXTRA_COMPILERS += vpnglean_pings_header vpnglean_pings_sources
