/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WINDOWSAPPIMAGEPROVIDER_H
#define WINDOWSAPPIMAGEPROVIDER_H

#include "appimageprovider.h"

class WindowsAppImageProvider final : public AppImageProvider {
 public:
  WindowsAppImageProvider(QObject* parent);
  ~WindowsAppImageProvider();
  QPixmap requestPixmap(const QString& id, QSize* size,
                        const QSize& requestedSize) override;
  static bool hasImage(const QString& path);
};

#endif  // WINDOWSAPPIMAGEPROVIDER_H
