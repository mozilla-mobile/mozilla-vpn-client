/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "dummyappimageprovider.h"
#include "logger.h"
#include "leakdetector.h"

namespace {
Logger logger(LOG_CONTROLLER, "DummyAppImageProvider");
}

DummyAppImageProvider::DummyAppImageProvider(QObject* parent)
    : QQuickImageProvider(QQuickImageProvider::Pixmap), QObject(parent) {
  MVPN_COUNT_CTOR(DummyAppImageProvider);
}

DummyAppImageProvider::~DummyAppImageProvider() {
  MVPN_COUNT_DTOR(DummyAppImageProvider);
}

// Returns an example image for any provided APPID (a red square)
QPixmap DummyAppImageProvider::requestPixmap(const QString& id, QSize* size,
                                             const QSize& requestedSize) {
  logger.debug() << "Request Image for " << id;
  int width = 50;
  int height = 50;

  if (size) *size = QSize(width, height);
  QPixmap pixmap(requestedSize.width() > 0 ? requestedSize.width() : width,
                 requestedSize.height() > 0 ? requestedSize.height() : height);
  pixmap.fill(QColor("red").rgba());
  return pixmap;
}
