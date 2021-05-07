# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

golibs.input = GO_MODULES
golibs.output = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.a
golibs.commands = @echo Building ${QMAKE_FILE_IN} \
    && cd ${QMAKE_FILE_PATH} \
    && go build -buildmode=c-archive -v -o ${QMAKE_FILE_BASE}.a ${QMAKE_FILE_BASE}.go 
golibs.clean = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.a
golibs.variable_out = LIBS
golibs.depends += \
    ${QMAKE_FILE_PATH}/go.mod \
    ${QMAKE_FILE_PATH}/go.sum
    
gohdr.input = GO_MODULES
gohdr.output = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.h
gohdr.commands = $$escape_expand(\\n)  # force creation of rule
gohdr.clean = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.h
gohdr.depends += ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.a
gohdr.variable_out = HEADERS

QMAKE_EXTRA_COMPILERS += golibs
QMAKE_EXTRA_COMPILERS += gohdr
