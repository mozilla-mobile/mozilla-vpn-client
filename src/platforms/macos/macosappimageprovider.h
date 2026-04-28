/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MACOSAPPIMAGEPROVIDER_H
#define MACOSAPPIMAGEPROVIDER_H

#include "appimageprovider.h"

class MacOSAppImageProvider final : public AppImageProvider {
 public:
  MacOSAppImageProvider(QObject* parent);
  ~MacOSAppImageProvider();
  QImage requestImage(const QString& id, QSize* size,
                      const QSize& requestedSize) override;
};

#endif  // MACOSAPPIMAGEPROVIDER_H
