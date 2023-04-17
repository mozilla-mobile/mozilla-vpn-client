/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "inspectorhotreloader.h"


#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDir>


#include "logger.h"
#include "frontend/navigatorreloader.h"
#include "frontend/navigator.h"
#include "tasks/function/taskfunction.h"

#include "networkrequest.h"


namespace {
Logger logger("QMLHotReload");
}

InspectorHotreloader::InspectorHotreloader(QQmlEngine* target): m_target(target) {
    m_target->addUrlInterceptor(this);
    new NavigatorReloader(qApp);
      QDir dataDir(
      QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
      m_qml_folder = dataDir.absoluteFilePath("hot_reload");

      QDir dir(m_qml_folder);
      if (!dir.exists()){
        dir.mkpath(".");
      }
    // On shutdown cleanup all things.
    QObject::connect(qApp, &QCoreApplication::aboutToQuit, [this] {
      QDir dir(m_qml_folder);
       if (dir.exists()){
        dir.removeRecursively();
       }
    });
  
  }

QUrl InspectorHotreloader::intercept(
    const QUrl& url, QQmlAbstractUrlInterceptor::DataType type) {
  logger.debug() << "Requested: " << url.fileName();
  if (m_announced_files.contains(url.fileName())) {
    logger.debug() << "Redirect! : "
                   << m_announced_files[url.fileName()].toString();
    return m_announced_files[url.fileName()];
  }
  return url;
}

void InspectorHotreloader::annonceReplacedFile(const QUrl& path) {
  logger.debug() << "Announced redirect! : " << path.fileName() << " as ->"
                 << path.toString();

  if( path.scheme() != "http" &&
      path.scheme() != "file" &&
      path.scheme() != "qrc"){
      logger.error() << "Unexpected File Scheme in: " << path.toString();
      return;
  }
  if (path.scheme() == "file" || path.scheme() == "qrc") {
    // If it's a file, just load it!
    m_target->clearComponentCache();
    m_announced_files.insert(path.fileName(), path);
    Navigator::instance()->reloadCurrentScreen();
    return;
  }
  // If it is a remote file, download to a temp file and then come back to 
  // announce it!
  fetchAndAnnounce(path);
}

void InspectorHotreloader::fetchAndAnnounce(const QUrl& path) {
  if( path.scheme() != "http"){
      logger.error() << "Unexpected File Scheme in: " << path.toString();
      return;
  }
  TaskFunction* dummy_task = new TaskFunction([]() {});
  NetworkRequest* request = new NetworkRequest(dummy_task, 200);
  request->get(path.toString());
  
  QObject::connect(request, &NetworkRequest::requestFailed,[](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.error() << "Get qml content failed" << error;
  });

  QObject::connect(request, &NetworkRequest::requestCompleted,
          [this,path,dummy_task](const QByteArray& data) {
        dummy_task->deleteLater();
        auto temp_path = QString("%1/%2").arg(
            m_qml_folder,path.fileName());
        logger.warning() << "writing" << temp_path;
        auto temp_file = new QFile(temp_path);
        temp_file->open(QIODevice::WriteOnly);
        if (!temp_file->write(data)) {
          logger.warning() << "Unable to write to file:" << temp_file->fileName();
          return;
        }
        if (!temp_file->flush()) {
          logger.warning() << "Unable to flush to file:" << temp_file->fileName();
          return;
        }
        temp_file->close();
        QFileInfo info(temp_path);
        annonceReplacedFile(QUrl::fromLocalFile(info.absoluteFilePath()));          
  });
}

void InspectorHotreloader::resetAllFiles() {
  logger.debug() << "Resetting hot reloaded files";

  QDir dir(m_qml_folder);
  if (dir.exists()){
    logger.debug() << "Removing hot reloaded files from disk";
    dir.removeRecursively();
  }
  m_target->clearComponentCache();
  m_announced_files.clear();
  Navigator::instance()->reloadCurrentScreen();
}
