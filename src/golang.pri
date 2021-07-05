# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

GOFLAGS=-mod vendor
exists($$(HOME)) {
    GOCACHE=$$system(go env GOCACHE)
} else {
    GOCACHE=/tmp/go-cache
}

## Download the Go module dependencies for offline builds
godeps.input = GO_MODULES
godeps.output = ${QMAKE_FILE_PATH}/vendor/modules.txt
godeps.commands = @echo Go dependencies ${QMAKE_FILE_IN} \
    && cd ${QMAKE_FILE_PATH} && go mod vendor
godeps.depends += ${QMAKE_FILE_IN}
godeps.clean_commands = rm -rf ${QMAKE_FILE_PATH}/vendor
godeps.CONFIG = no_link

## Build the Go module into a callable C library
golibs.input = GO_MODULES
golibs.output = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.a
golibs.commands = @echo Building ${QMAKE_FILE_IN} \
    && cd ${QMAKE_FILE_PATH} \
    && GOCACHE=$${GOCACHE} go build -buildmode=c-archive $${GOFLAGS} -v -o ${QMAKE_FILE_BASE}.a ${QMAKE_FILE_BASE}.go 
golibs.clean = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.a
golibs.variable_out = LIBS
golibs.depends += ${QMAKE_FILE_PATH}/vendor/modules.txt
golibs.CONFIG += target_predeps

## Dummy compiler for the library header file
gohdr.input = GO_MODULES
gohdr.output = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.h
gohdr.commands = $$escape_expand(\\n)  # force creation of rule
gohdr.clean = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.h
gohdr.depends += ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.a
gohdr.variable_out = HEADERS

## Add Go compiler tools to qmake
QMAKE_EXTRA_COMPILERS += godeps
QMAKE_EXTRA_COMPILERS += golibs
QMAKE_EXTRA_COMPILERS += gohdr
