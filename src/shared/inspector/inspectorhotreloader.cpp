//
// Created by Basti on 02/03/2023.
//

#include "inspectorhotreloader.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "networkmanager.h"

namespace {
Logger logger("hotttt");

}

QUrl InspectorHotreloader::intercept(
    const QUrl& url, QQmlAbstractUrlInterceptor::DataType type) {
  if (type != DataType::QmlFile) {
    return url;
  }
  logger.debug() << "Requested: " << url.fileName();
  if (m_announced_files.contains(url.fileName())) {
    logger.debug() << "Redirect! : "
                   << m_announced_files[url.fileName()].toString();
    return m_announced_files[url.fileName()];
  }
  logger.debug() << "Not redirect: " << url.toString();
  return url;
}

void InspectorHotreloader::annonceReplacedFile(const QUrl& path) {
  logger.debug() << "Announced redirect! : " << path.fileName() << " as ->"
                 << path.toString();
  m_target->clearComponentCache();
  m_announced_files.insert(path.fileName(), path);

  if (path.scheme() == "file" || path.scheme() == "qrc") {
    // is's a file, just load it!
    Navigator::instance()->reloadCurrentScreen();
    return;
  }
  if (path.scheme() != "http") {
    logger.error() << "Unexpected File Scheme in: " << path.toString();
  }
  // Okay so let's have a request with the nm so the file is in the cache once
  // we hit it!
  QNetworkAccessManager* nm =
      NetworkManager::instance()->networkAccessManager();
  auto req = QNetworkRequest(path);
  auto reply = nm->get(req);
  QObject::connect(reply, &QNetworkReply::finished, [path] {
    logger.debug() << "Fetched & Cached: " << path.toString();
    Navigator::instance()->reloadCurrentScreen();
  });
}

void InspectorHotreloader::resetAllFiles() {
  logger.debug() << "Resetting hot reloaded files";
  m_target->clearComponentCache();
  m_announced_files.clear();
  Navigator::instance()->reloadCurrentScreen();
}
