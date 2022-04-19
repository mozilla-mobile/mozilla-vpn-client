# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(lottie STATIC)

find_package(Qt6 REQUIRED COMPONENTS Core Qml)
target_link_libraries(lottie PRIVATE Qt6::Core Qt6::Qml)
target_include_directories(lottie PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib)

target_sources(lottie PRIVATE
    lib/lottie.cpp
    lib/lottie.h
    lib/lottieprivate.cpp
    lib/lottieprivate.h
    lib/lottieprivatedocument.cpp
    lib/lottieprivatedocument.h
    lib/lottieprivatenavigator.cpp
    lib/lottieprivatenavigator.h
    lib/lottieprivatewindow.cpp
    lib/lottieprivatewindow.h
    lib/lottiestatus.h
    lib/lottie.qrc
)
