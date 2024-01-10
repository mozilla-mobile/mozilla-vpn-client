/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowsappimageprovider.h"

#include <Windows.h>
#include <shellapi.h>

#include <QDir>
#include <QObject>
#include <QPixmap>
#include <QQuickImageProvider>
#include <QScopedArrayPointer>
#include <QStringList>
#include <QSysInfo>

#include "leakdetector.h"
#include "logging/logger.h"
#include "platforms/windows/windowsutils.h"

namespace {
Logger logger("WindowsAppImageProvider");
}

WindowsAppImageProvider::WindowsAppImageProvider(QObject* parent)
    : AppImageProvider(parent, QQuickImageProvider::Pixmap) {
  MZ_COUNT_CTOR(WindowsAppImageProvider);
}

WindowsAppImageProvider::~WindowsAppImageProvider() {
  MZ_COUNT_DTOR(WindowsAppImageProvider);
}

QPixmap WindowsAppImageProvider::requestPixmap(const QString& path, QSize* size,
                                               const QSize& requestedSize) {
  Q_UNUSED(requestedSize);
  logger.debug() << "Request Image for " << path;

  const QString nativePath = QDir::toNativeSeparators(path);
  const auto* sourceFileC =
      reinterpret_cast<const wchar_t*>(nativePath.utf16());
  const UINT iconCount = ExtractIconEx(sourceFileC, -1, nullptr, nullptr, 0);
  if (!iconCount) {
    WindowsUtils::windowsLog(path + " does not appear to contain icons.");
    return QPixmap();
  }
  QScopedArrayPointer<HICON> icons(new HICON[iconCount]);
  const auto extractedIconCount =
      ExtractIconEx(sourceFileC, 0, icons.data(), nullptr, iconCount);
  if (!extractedIconCount) {
    WindowsUtils::windowsLog(path + " Failed to extract icon");
    return QPixmap();
  }

  auto pixmap = QPixmap::fromImage(QImage::fromHICON(icons[0]));
  if (pixmap.isNull()) {
    WindowsUtils::windowsLog(path + " Failed to convert icon");
    return QPixmap();
  }
  if (size) *size = pixmap.size();
  return pixmap;
}

/*
 * Returns true if the Path Contains an icon.
 */
bool WindowsAppImageProvider::hasImage(const QString& path) {
  const QString nativePath = QDir::toNativeSeparators(path);
  const auto* sourceFileC =
      reinterpret_cast<const wchar_t*>(nativePath.utf16());
  const UINT iconCount = ExtractIconEx(sourceFileC, -1, nullptr, nullptr, 0);
  return iconCount != 0;
}
