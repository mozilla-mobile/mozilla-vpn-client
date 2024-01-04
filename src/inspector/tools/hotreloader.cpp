/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "hotreloader.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonObject>
#include <QQmlApplicationEngine>
#include <QStandardPaths>
#include <QWindow>

#include "../inspector.h"
#include "command.h"
#include "commandhandler.h"

namespace InspectorTools {

Hotreloader::Hotreloader(QObject* parent, QQmlEngine* target)
    : QObject(parent), m_target(target), m_intercecptor(this) {
  m_target->addUrlInterceptor(&m_intercecptor);
  QDir dataDir(
      QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
  m_qml_folder = dataDir.absoluteFilePath("hot_reload");

  QDir dir(m_qml_folder);
  if (!dir.exists()) {
    dir.mkpath(".");
  }
  // On shutdown cleanup all things.
  QObject::connect(qApp, &QCoreApplication::aboutToQuit, [this] {
    QDir dir(m_qml_folder);
    if (dir.exists()) {
      dir.removeRecursively();
    }
  });

  registerDevCommands();
}

void Hotreloader::annonceReplacedFile(const QUrl& path) {
  qDebug() << "Announced redirect! : " << path.fileName() << " as ->"
           << path.toString();

  if (path.scheme() != "http" && path.scheme() != "file" &&
      path.scheme() != "qrc") {
    qDebug() << "Unexpected File Scheme in: " << path.toString();
    return;
  }
  if (path.scheme() == "file" || path.scheme() == "qrc") {
    // If it's a file, just load it!
    m_target->clearComponentCache();
    m_announced_files.insert(path.fileName(), path);
    reloadWindow();
    return;
  }
  qDebug() << "Unexpected file format";
}

void Hotreloader::pushFile(const QString& fileName, const QByteArray& data) {
  auto temp_path = QString("%1/%2").arg(m_qml_folder, fileName);
  auto temp_file = new QFile(temp_path);
  temp_file->open(QIODevice::WriteOnly);
  if (!temp_file->write(data)) {
    qDebug() << "Unable to write to file:" << temp_file->fileName();
    return;
  }
  if (!temp_file->flush()) {
    qDebug() << "Unable to flush to file:" << temp_file->fileName();
    return;
  }
  temp_file->close();
  QFileInfo info(temp_path);
  annonceReplacedFile(QUrl::fromLocalFile(info.absoluteFilePath()));
}

void Hotreloader::resetAllFiles() {
  qDebug() << "Resetting hot reloaded files";

  QDir dir(m_qml_folder);
  if (dir.exists()) {
    qDebug() << "Removing hot reloaded files from disk";
    dir.removeRecursively();
  }
  m_target->clearComponentCache();
  m_announced_files.clear();
  reloadWindow();
}

void Hotreloader::reloadWindow() {
  QQmlApplicationEngine* engine = static_cast<QQmlApplicationEngine*>(m_target);
  // Here is the main QML file.
  if (!engine->rootObjects().isEmpty()) {
    qDebug() << "No Window to reload";
    return;
  }
  qDebug() << "Closing and full reloading window";

  int x = 0, y = 0;
  // We may have multiple closed windows still in here.
  // So best to go over them all.
  QList<QObject*> rootObjects = engine->rootObjects();
  for (auto* rootObject : rootObjects) {
    QWindow* maybeWindow = qobject_cast<QWindow*>(rootObject);
    if (maybeWindow) {
      if (maybeWindow->isVisible()) {
        x = maybeWindow->x();
        y = maybeWindow->y();
      }
      maybeWindow->close();
    }
  }
  const QUrl url(QStringLiteral("qrc:/ui/main.qml"));
  engine->load(url);
  auto newWindow = qobject_cast<QWindow*>(engine->rootObjects().last());
  if (newWindow) {
    if (x != 0 && y != 0) {
      newWindow->setX(x);
      newWindow->setY(y);
    }
  }
}

QUrl Hotreloader::HotReloadInterceptor::intercept(
    const QUrl& url, QQmlAbstractUrlInterceptor::DataType type) {
  if (m_parent.isNull()) {
    return url;
  }
  qDebug() << "Requested: " << url.fileName();
  if (m_parent->m_announced_files.contains(url.fileName())) {
    qDebug() << "Redirect! : "
             << m_parent->m_announced_files[url.fileName()].toString();
    return m_parent->m_announced_files[url.fileName()];
  }
  return url;
}

void Hotreloader::registerDevCommands() {
  Inspector* i = qobject_cast<Inspector*>(parent());
  if (i) {
    return;
  }

  i->registerCommand(
      InspectorTools::Command{"live_reload", "Live reload file X", 1,
                              [this](const QList<QByteArray>& args) {
                                auto url = QUrl(args.at(1));
                                annonceReplacedFile(url);
                                return QJsonObject();
                              }});
  i->registerCommand(
      InspectorTools::Command{"reload_window", "Reload the whole window", 0,
                              [this](const QList<QByteArray>& args) {
                                reloadWindow();
                                return QJsonObject();
                              }});

  i->registerCommand(InspectorTools::Command{
      "reset_live_reload", "Reset all hot reloaded files", 0,
      [this](const QList<QByteArray>& args) {
        resetAllFiles();
        return QJsonObject();
      }});
}

}  // namespace InspectorTools
