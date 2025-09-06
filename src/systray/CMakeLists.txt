# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

qt_add_qml_module(systray
    VERSION 1.0
    URI SysTray
    STATIC
    OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/SysTray
    QML_FILES
        SysTray.qml
        SysTrayCard.qml
    SOURCES
        systray.h
        systray.cpp
        systraymanager.h
        systraymanager.cpp
)

target_link_libraries(systray PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::Widgets
)

# Make sure we can find the common headers
target_include_directories(systray PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(mozillavpn PRIVATE systray systrayplugin)
