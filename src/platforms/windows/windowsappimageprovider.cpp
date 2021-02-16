/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "logger.h"
#include "leakdetector.h"
#include "windowsappimageprovider.h"
#include <QObject>
#include <QQuickImageProvider>
#include <QtWin>

#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImage>
#include <QPixmap>
#include <QScopedArrayPointer>
#include <QStringList>
#include <QSysInfo>

#include <iostream>

#include <shellapi.h>
#include <comdef.h>
#include <commctrl.h>
#include <objbase.h>
#include <commoncontrols.h>


namespace {
Logger logger(LOG_WINDOWS, "WindowsAppImageProvider");
}


WindowsAppImageProvider::WindowsAppImageProvider(QObject* parent)
    : QQuickImageProvider(QQuickImageProvider::Pixmap), QObject(parent) {
  MVPN_COUNT_CTOR(WindowsAppImageProvider);
}

WindowsAppImageProvider::~WindowsAppImageProvider() {
  MVPN_COUNT_DTOR(WindowsAppImageProvider);
}


QPixmap WindowsAppImageProvider::requestPixmap(const QString& path, QSize* size,
                                      const QSize& requestedSize){
    Q_UNUSED(requestedSize);
    logger.log() << "Request Image for " << path;

    const QString nativePath = QDir::toNativeSeparators(path);
    const auto *sourceFileC = reinterpret_cast<const wchar_t *>(nativePath.utf16());
    const auto iconCount = ExtractIconEx(sourceFileC, -1, nullptr, nullptr, 0);
    if (!iconCount) {
        logger.log() << path << " does not appear to contain icons.\n";
        return QPixmap();
    }
    QScopedArrayPointer<HICON> icons(new HICON[iconCount]);
    const auto extractedIconCount = ExtractIconEx(sourceFileC, 0, icons.data(), nullptr, iconCount);
    if (!extractedIconCount) {
        logger.log() << path << " Failed to extract icon";
        return QPixmap();
    }

    auto pixmap = QtWin::fromHICON(icons[0]);
    if (pixmap.isNull()) {
        logger.log() << path << " Failed to convert icon";
        return QPixmap();
    }
    if (size) *size = pixmap.size();
    return pixmap;
}
