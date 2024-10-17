# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# If DESTDIR is set, we are probably not installing on a real system
# and this is just for packaging. We should do nothing here.
if(DEFINED ENV{DESTDIR})
  return()
endif()

# If we are not root - do nothing.
execute_process(
    COMMAND id -u
    OUTPUT_VARIABLE CURRENT_UID
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(NOT "${CURRENT_UID}" STREQUAL "0")
  return()
endif()

# If systemctl exists then reload the service files and restart the daemon.
find_program(SYSTEMCTL_EXECUTABLE systemctl)
if(SYSTEMCTL_EXECUTABLE)
    execute_process(
        COMMAND ${SYSTEMCTL_EXECUTABLE} daemon-reload
        COMMAND ${SYSTEMCTL_EXECUTABLE} restart mozillavpn
    )
endif()

# If we can, run setcap on the installed VPN binary.
find_program(SETCAP_EXECUTABLE setcap)
if(SETCAP_EXECUTABLE)
    execute_process(
        COMMAND ${SETCAP_EXECUTABLE} cap_net_raw+ep ${CMAKE_INSTALL_PREFIX}/bin/mozillavpn
    )
endif()

