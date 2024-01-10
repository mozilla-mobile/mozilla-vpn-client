/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addonreplacer.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QQmlEngine>
#include <QScopeGuard>

#include "leakdetector.h"
#include "logging/logger.h"
#include "qmlengineholder.h"
#include "resourceloader.h"

namespace {
Logger logger("AddonReplacer");

QUrl normalizeUrlScheme(const QUrl& url, const QString& response) {
  // QT is a strange framework. Sometimes the files are loaded using 'qrc:'
  // scheme, sometimes, just ':'. Let's normalize the final URL based on the
  // incoming one.

  if (url.scheme() == "qrc" && response.startsWith(':')) {
    return QString("qrc%1").arg(response);
  }
  return response;
}
}  // namespace

// static
Addon* AddonReplacer::create(QObject* parent, const QString& manifestFileName,
                             const QString& id, const QString& name,
                             const QJsonObject& obj) {
  QJsonObject replacerObj = obj["replacer"].toObject();

  QJsonArray urls = replacerObj["urls"].toArray();
  if (urls.isEmpty()) {
    logger.warning() << "Empty URLs";
    return nullptr;
  }

  AddonReplacer* replacer =
      new AddonReplacer(parent, manifestFileName, id, name);
  auto guard = qScopeGuard([&] { replacer->deleteLater(); });

  QFileInfo manifestFileInfo(manifestFileName);
  QDir addonPath = manifestFileInfo.dir();

  for (const QJsonValue& urlValue : urls) {
    QJsonObject urlObj = urlValue.toObject();

    QString urlRequest = urlObj["request"].toString();
    QString urlResponse = urlObj["response"].toString();
    QString urlResponseFile(addonPath.filePath(urlResponse));

    if (urlRequest.isEmpty() || urlResponse.isEmpty()) {
      logger.warning() << "Invalid URL content";
      return nullptr;
    }

    if (!urlRequest.startsWith("qrc:")) {
      logger.warning() << "Invalid URL content (QRC scheme only)";
      return nullptr;
    }

    QString type = urlObj["type"].toString();
    if (type.isEmpty() || type == "file") {
      if (!QFile::exists(urlResponseFile)) {
        logger.warning() << "The addon does not contain the file `"
                         << urlResponseFile << "' required for `" << urlRequest
                         << "'";
        return nullptr;
      }

      replacer->m_replaces.append({eUrlFile, urlRequest, urlResponseFile});
    } else if (type == "directory") {
      if (!QDir(urlResponseFile).exists()) {
        logger.warning() << "The addon does not contain the directory `"
                         << urlResponseFile << "' required for `" << urlRequest
                         << "'";
        return nullptr;
      }

      // To simplify the path comparison, let's add a final '/' in both
      // request/response strings.

      if (!urlRequest.endsWith('/')) {
        urlRequest.append('/');
      }

      if (!urlResponseFile.endsWith('/')) {
        urlResponseFile.append('/');
      }

      replacer->m_replaces.append({eUrlDirectory, urlRequest, urlResponseFile});
    } else {
      logger.warning() << "Invalid URL type:" << type;
      return nullptr;
    }
  }

  guard.dismiss();
  return replacer;
}

AddonReplacer::AddonReplacer(QObject* parent, const QString& manifestFileName,
                             const QString& id, const QString& name)
    : Addon(parent, manifestFileName, id, name, "replacer") {
  MZ_COUNT_CTOR(AddonReplacer);
}

AddonReplacer::~AddonReplacer() { MZ_COUNT_DTOR(AddonReplacer); }

void AddonReplacer::enable() {
  Addon::enable();
  QmlEngineHolder::instance()->engine()->addUrlInterceptor(this);
  ResourceLoader::instance()->addUrlInterceptor(this);
}

void AddonReplacer::disable() {
  Addon::disable();
  QmlEngineHolder::instance()->engine()->removeUrlInterceptor(this);
  ResourceLoader::instance()->removeUrlInterceptor(this);
}

QUrl AddonReplacer::intercept(const QUrl& url,
                              QQmlAbstractUrlInterceptor::DataType type) {
  if (type != DataType::QmlFile) {
    return url;
  }

  QString urlRequest = url.toString();

  for (const Replace& replace : m_replaces) {
    if (replace.m_type == eUrlFile) {
      if (replace.m_request != urlRequest) continue;
      return normalizeUrlScheme(url, replace.m_response);
    }

    Q_ASSERT(replace.m_type == eUrlDirectory);
    if (!urlRequest.startsWith(replace.m_request)) continue;

    QString newPath = urlRequest;
    newPath.replace(0, replace.m_request.length(), replace.m_response);
    return normalizeUrlScheme(url, newPath);
  }

  return url;
}
