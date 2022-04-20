# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Dummy platform source files
target_sources(mozillavpn PRIVATE
    platforms/dummy/dummycontroller.cpp
    platforms/dummy/dummycryptosettings.cpp
    systemtraynotificationhandler.cpp
    tasks/authenticate/desktopauthenticationlistener.cpp
)

# Dummy platform header files
target_sources(mozillavpn PRIVATE
    platforms/dummy/dummycontroller.h
    systemtraynotificationhandler.h
    tasks/authenticate/desktopauthenticationlistener.h
)
