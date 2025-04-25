# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# This File Prints a warning if someone tries to compile Qt with a version
# that we know the VPN will break with. 

if( ${Qt6_VERSION} VERSION_EQUAL 6.8.0)
    message(AUTHOR_WARNING 
        "*******************  WARNING  ********************** \n"
        "Qt 6.8.0 is known to be broken and will fail to load Mozila VPN \n"
        "Please consider upgrading to at least Qt 6.8.1 \n"
        "See: https://bugreports.qt.io/browse/QTBUG-129500 \n"
        "****************************************************"
    )
endif()
