/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef APPIMAGEPROVIDER_H
#define APPIMAGEPROVIDER_H

#include <QObject>
#include <QQuickImageProvider>

// Helper class to support Qt5 and Qt6
class AppImageProvider : public QQuickImageProvider
#if QT_VERSION < 0x060000
    ,
                         public QObject
#endif
{
 public:
  AppImageProvider(QObject* parent, QQmlImageProviderBase::ImageType type,
                   QQmlImageProviderBase::Flags flags = Flags())
      : QQuickImageProvider(type, flags)
#if QT_VERSION < 0x060000
        ,
        QObject(parent)
#endif
  {
#if QT_VERSION >= 0x060000
    setParent(parent);
#endif
  }

  virtual ~AppImageProvider() = default;
};

#endif  // APPIMAGEPROVIDER_H
