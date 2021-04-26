/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "linuxappimageprovider.h"
#include "logger.h"
#include "leakdetector.h"

#include <QSettings>
#include <QIcon>

constexpr const char* DESKTOP_ENTRY_LOCATION = "/usr/share/applications/";
constexpr const char* PIXMAP_FALLBACK_PATH = "/usr/share/pixmaps/";

namespace {
Logger logger(LOG_CONTROLLER, "LinuxAppImageProvider");
}

LinuxAppImageProvider::LinuxAppImageProvider(QObject* parent)
    : QQuickImageProvider(QQuickImageProvider::Image,
                          QQmlImageProviderBase::ForceAsynchronousImageLoading),
      QObject(parent) {
  MVPN_COUNT_CTOR(LinuxAppImageProvider);
  QIcon::setFallbackSearchPaths(QIcon::fallbackSearchPaths() << PIXMAP_FALLBACK_PATH);
}

LinuxAppImageProvider::~LinuxAppImageProvider() {
  MVPN_COUNT_DTOR(LinuxAppImageProvider);
}

// from QQuickImageProvider
QImage LinuxAppImageProvider::requestImage(const QString& id, QSize* size,
                                             const QSize& requestedSize) {

  QSettings entry(QString(DESKTOP_ENTRY_LOCATION) + "/" + id, QSettings::IniFormat);
  entry.beginGroup("Desktop Entry");
  QString name = entry.value("Icon").toString();

  QIcon icon = QIcon::fromTheme(name);
  QPixmap pixmap = icon.pixmap(requestedSize);
  size->setHeight(pixmap.height());
  size->setWidth(pixmap.width());
  logger.log() << "Loaded icon" << icon.name() << "size:" << pixmap.width() << "x" << pixmap.height();

  return pixmap.toImage();
}
