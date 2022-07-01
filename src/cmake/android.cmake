# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


target_link_libraries(mozillavpn PRIVATE
    Qt6::Xml)

target_link_libraries(
    mozillavpn PRIVATE
    -ljnigraphics)

target_sources(mozillavpn PRIVATE 
    platforms/android/androidcontroller.cpp
    platforms/android/androidglean.cpp
    platforms/android/androidiaphandler.cpp
    platforms/android/androidnetworkwatcher.cpp
    platforms/android/androidnotificationhandler.cpp
    platforms/android/androidutils.cpp
    platforms/android/androidvpnactivity.cpp
    platforms/android/androiddatamigration.cpp
    platforms/android/androidappimageprovider.cpp
    platforms/android/androidapplistprovider.cpp
    platforms/android/androidsharedprefs.cpp
    tasks/purchase/taskpurchase.cpp
    platforms/android/androidcontroller.h
    platforms/android/androidglean.h
    platforms/android/androidiaphandler.h
    platforms/android/androidnetworkwatcher.h
    platforms/android/androidnotificationhandler.h
    platforms/android/androidutils.h
    platforms/android/androidvpnactivity.h
    platforms/android/androiddatamigration.h
    platforms/android/androidappimageprovider.h
    platforms/android/androidapplistprovider.h
    platforms/android/androidsharedprefs.h
    tasks/purchase/taskpurchase.h
    platforms/linux/linuxpingsender.cpp 
    platforms/linux/linuxcryptosettings.cpp
    platforms/linux/linuxpingsender.h
)