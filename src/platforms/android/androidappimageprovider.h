/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ANDROIDAPPIMAGEPROVIDER_H
#define ANDROIDAPPIMAGEPROVIDER_H

#include "appimageprovider.h"

#if QT_VERSION >= 0x060000
#  include <QJniObject>
#else
#  include <QAndroidJniObject>
#endif

#if QT_VERSION < 0x060000
typedef QAndroidJniObject QJniObject;
#endif

class AndroidAppImageProvider final : public AppImageProvider {
 public:
  AndroidAppImageProvider(QObject* parent);
  ~AndroidAppImageProvider();
  QImage requestImage(const QString& id, QSize* size,
                      const QSize& requestedSize) override;

  QJniObject createBitmap(int width, int height);
  QImage toImage(const QJniObject& bitmap);
  QImage toImage(const QJniObject& drawable, const QRect& bounds);
};

#endif  // ANDROIDAPPIMAGEPROVIDER_H
