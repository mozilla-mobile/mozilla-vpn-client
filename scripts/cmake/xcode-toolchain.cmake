# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# We require a sysroot in order to find Apple's tooling.
if(NOT CMAKE_OSX_SYSROOT)
    return()
endif()

# A helper function to find and replace tooling.
function(_xcode_force_toolchain OUTPUT_VARIABLE TOOLNAME)
    execute_process(OUTPUT_VARIABLE XCODE_TOOLPATH OUTPUT_STRIP_TRAILING_WHITESPACE
        COMMAND xcrun --sdk ${CMAKE_OSX_SYSROOT} --find ${TOOLNAME})
    
    set(${OUTPUT_VARIABLE} ${XCODE_TOOLPATH} CACHE FILEPATH "Xcode-provided ${TOOLNAME} tool" FORCE)
endfunction(_xcode_force_toolchain)

# The tools that we need to get from Apple's SDK.
_xcode_force_toolchain(CMAKE_RANLIB "ranlib")
