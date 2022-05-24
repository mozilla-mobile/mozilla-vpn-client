# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

qt_add_executable(lottie_tests)

target_link_libraries(lottie_tests PRIVATE
    Qt6::Core
    Qt6::Test
    Qt6::Qml
    Qt6::Quick
)

target_sources(lottie_tests PRIVATE
    ../../lib/lottieprivate.cpp
    ../../lib/lottieprivate.h
    ../../lib/lottieprivatedocument.cpp
    ../../lib/lottieprivatedocument.h
    ../../lib/lottieprivatenavigator.cpp
    ../../lib/lottieprivatenavigator.h
    ../../lib/lottieprivatewindow.cpp
    ../../lib/lottieprivatewindow.h
    ../../lib/lottiestatus.h
    helper.h
    main.cpp
    testdocument.cpp
    testdocument.h
    testnavigator.cpp
    testnavigator.h
    testwindow.cpp
    testwindow.h
)

add_test(NAME lottie_tests COMMAND lottie_tests)
