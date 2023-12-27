# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

qt_add_qml_module(mozillavpn-ui
    VERSION 1.0
    URI Mozilla.VPN.ui
    STATIC
    OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Mozilla/VPN/ui
    QML_FILES
        composer/MZComposerView.qml
    SOURCES
        composer/composer.cpp
        composer/composer.h
        composer/composerblock.cpp
        composer/composerblock.h
        composer/composerblockbutton.cpp
        composer/composerblockbutton.h
        composer/composerblockorderedlist.cpp
        composer/composerblockorderedlist.h
        composer/composerblocktext.cpp
        composer/composerblocktext.h
        composer/composerblocktitle.cpp
        composer/composerblocktitle.h
        composer/composerblockunorderedlist.cpp
        composer/composerblockunorderedlist.h
)

target_link_libraries(mozillavpn-ui PRIVATE
    Qt6::Quick
    Qt6::Widgets
)

target_include_directories(mozillavpn-ui PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_include_directories(mozillavpn-ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/composer
)
