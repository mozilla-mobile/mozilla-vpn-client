//
// Created by Basti on 02/03/2023.
//

#include "inspectorhotreloader.h"

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
  Navigator::instance()->reloadCurrentScreen();
}