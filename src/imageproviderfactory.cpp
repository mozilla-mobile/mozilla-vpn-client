/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "imageproviderfactory.h"

#ifdef MVPN_LINUX
#  include "platforms/linux/linuxappimageprovider.h"
#endif

#ifdef MVPN_ANDROID
#  include "platforms/android/androidappimageprovider.h"
#endif

#ifdef MVPN_WINDOWS
#  include "platforms/windows/windowsappimageprovider.h"
#endif

QQuickImageProvider * ImageProviderFactory::create(QObject *parent){
#ifdef MVPN_LINUX
    return new LinuxAppImageProvider(parent);
#endif

#ifdef MVPN_ANDROID
    return new AndroidAppImageProvider(parent);
#endif
#ifdef MVPN_WINDOWS
    return new WindowsAppImageProvider(parent);
#endif
    return nullptr;
}
