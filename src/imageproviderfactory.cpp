/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "imageproviderfactory.h"

#ifdef MZ_LINUX
#  include "platforms/linux/linuxappimageprovider.h"
#endif

#ifdef MZ_ANDROID
#  include "platforms/android/androidappimageprovider.h"
#endif

#ifdef MZ_WINDOWS
#  include "platforms/windows/windowsappimageprovider.h"
#endif

QQuickImageProvider* ImageProviderFactory::create(QObject* parent) {
#ifdef MZ_LINUX
  return new LinuxAppImageProvider(parent);
#endif

#ifdef MZ_ANDROID
  return new AndroidAppImageProvider(parent);
#endif
#ifdef MZ_WINDOWS
  return new WindowsAppImageProvider(parent);
#endif
  Q_UNUSED(parent);
  return nullptr;
}
