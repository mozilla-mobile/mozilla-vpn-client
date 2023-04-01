/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef RESOURCELOADER_H
#define RESOURCELOADER_H

#include <QList>
#include <QObject>

class QQmlAbstractUrlInterceptor;

class ResourceLoader final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(ResourceLoader)

 public:
  static ResourceLoader* instance();

  ~ResourceLoader();

  QString loadFile(const QString& fileName);
  QString loadDir(const QString& dirName);

  void addUrlInterceptor(QQmlAbstractUrlInterceptor* interceptor);
  void removeUrlInterceptor(QQmlAbstractUrlInterceptor* interceptor);

 private:
  ResourceLoader(QObject* parent);

 signals:
  void cacheFlushNeeded();

 private:
  QList<QQmlAbstractUrlInterceptor*> m_interceptors;
};

#endif  // RESOURCELOADER_H
