/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "resourceloader.h"

#include <QCoreApplication>
#include <QQmlAbstractUrlInterceptor>
#include <QUrl>

#include "utilities/leakdetector.h"

// static
ResourceLoader* ResourceLoader::instance() {
  static ResourceLoader* s_instance = nullptr;
  if (!s_instance) {
    s_instance = new ResourceLoader(qApp);
  }
  return s_instance;
}

ResourceLoader::ResourceLoader(QObject* parent) : QObject(parent) {
  MZ_COUNT_CTOR(ResourceLoader);
}

ResourceLoader::~ResourceLoader() { MZ_COUNT_DTOR(ResourceLoader); }

QString ResourceLoader::loadFile(const QString& fileName) {
  Q_ASSERT(fileName[0] == ':');

  QUrl url(QString("qrc%1").arg(fileName));

  for (QQmlAbstractUrlInterceptor* interceptor : m_interceptors) {
    QUrl newUrl =
        interceptor->intercept(url, QQmlAbstractUrlInterceptor::QmlFile);
    if (newUrl != url) {
      return QString(":%1").arg(newUrl.path());
    }
  }

  return fileName;
}

QString ResourceLoader::loadDir(const QString& fileName) {
  Q_ASSERT(fileName[0] == ':');

  QString dirName(QString("qrc%1").arg(fileName));
  if (!dirName.endsWith('/')) {
    dirName.append('/');
  }

  QUrl url(dirName);

  for (QQmlAbstractUrlInterceptor* interceptor : m_interceptors) {
    QUrl newUrl =
        interceptor->intercept(url, QQmlAbstractUrlInterceptor::QmlFile);
    if (newUrl != url) {
      return QString(":%1").arg(newUrl.path());
    }
  }

  return fileName;
}

void ResourceLoader::addUrlInterceptor(
    QQmlAbstractUrlInterceptor* interceptor) {
  Q_ASSERT(!m_interceptors.contains(interceptor));
  m_interceptors.append(interceptor);
  emit cacheFlushNeeded();
}

void ResourceLoader::removeUrlInterceptor(
    QQmlAbstractUrlInterceptor* interceptor) {
  Q_ASSERT(m_interceptors.contains(interceptor));
  m_interceptors.removeOne(interceptor);
  emit cacheFlushNeeded();
}
