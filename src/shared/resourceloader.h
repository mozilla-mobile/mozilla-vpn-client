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

  /**
   * @brief returns the path of the file the caller wants to load.
   */
  QString loadFile(const QString& fileName);

  /**
   * @brief returns the path of the directory the caller wants to load.
   */
  QString loadDir(const QString& dirName);

  /**
   * @brief add a new URL interceptor. Ownership does not change.
   */
  void addUrlInterceptor(QQmlAbstractUrlInterceptor* interceptor);

  /**
   * @brief remove an existing URL interceptor. Ownership does not change.
   */
  void removeUrlInterceptor(QQmlAbstractUrlInterceptor* interceptor);

 private:
  ResourceLoader(QObject* parent);

 signals:
  void cacheFlushNeeded();

 private:
  QList<QQmlAbstractUrlInterceptor*> m_interceptors;
};

#endif  // RESOURCELOADER_H
