# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# link_whole_archive(<consumer> <dep>): force every .o in STATIC `dep`
# into `consumer` so translation-unit static initializers (e.g.
# Command::RegistrationProxy<…> in commands/command*.cpp,
# DBusMetatypeRegistrationProxy in platforms/linux/linuxutils.cpp) are
# not discarded by the linker.
#
# Can't use CMake's $<LINK_LIBRARY:WHOLE_ARCHIVE,…> (3.24+): it forbids
# the same target appearing with and without the feature, and
# libMozillavpn is transitively linked from mozillavpn-uiplugin without
# it.

function(link_whole_archive consumer dep)
    if(MSVC)
        # /WHOLEARCHIVE:<name> base-name matching fails under lld-link;
        # pass the resolved archive path instead.
        target_link_libraries(${consumer} PRIVATE ${dep})
        target_link_options(${consumer} PRIVATE
            "/WHOLEARCHIVE:$<TARGET_FILE:${dep}>"
        )
    elseif(APPLE)
        target_link_libraries(${consumer} PRIVATE ${dep})
        target_link_libraries(${consumer} PRIVATE
            "-Wl,-force_load,$<TARGET_FILE:${dep}>"
        )
    else()
        target_link_libraries(${consumer} PRIVATE
            -Wl,--whole-archive
            ${dep}
            -Wl,--no-whole-archive
        )
    endif()
endfunction()
