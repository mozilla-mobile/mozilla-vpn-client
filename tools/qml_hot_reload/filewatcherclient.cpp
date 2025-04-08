/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "filewatcherclient.h"
#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QUrl>
#include <QNetworkRequest>
#include <QDirIterator>
#include <QTimer>

#ifndef PROJECT_SOURCE_DIR
    #define PROJECT_SOURCE_DIR ""
    #warning "PROJECT_SOURCE_DIR was not defined by CMake. Watching current working directory instead."
#endif

FileWatcherClient::FileWatcherClient(QCoreApplication* app, QObject *parent)
    : QObject(parent),
      m_app(app),
      m_wsUrl(QStringLiteral("ws://127.0.0.1:8765")),
      m_origin(QStringLiteral("https://mozilla-mobile.github.io")),
      m_watchRootPath(QStringLiteral(PROJECT_SOURCE_DIR))
{
    connect(&m_webSocket, &QWebSocket::connected, this, &FileWatcherClient::onConnected);
    connect(&m_webSocket, &QWebSocket::disconnected, this, &FileWatcherClient::onDisconnected);
    connect(&m_fileWatcher, &QFileSystemWatcher::fileChanged, this, &FileWatcherClient::onFileChanged);
    connect(m_app, &QCoreApplication::aboutToQuit, this, &FileWatcherClient::aboutToQuit);

    static_assert(PROJECT_SOURCE_DIR);
    qInfo() << "Connecting to Client at" << m_wsUrl.toString();
    QNetworkRequest request(m_wsUrl);
    request.setRawHeader("Origin", m_origin.toUtf8());
    m_webSocket.open(request);
}

FileWatcherClient::~FileWatcherClient()
{
    m_webSocket.close();
}

void FileWatcherClient::startWatching()
{
    qInfo() << "Performing initial scan of:" << m_watchRootPath;
    QDirIterator it(m_watchRootPath, QStringList() << "*.qml", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString filePath = it.next();
        m_fileWatcher.addPath(filePath);
    }
    qInfo() << "Initial scan complete. Watching" << m_fileWatcher.files().count() << "QML files.";
}

void FileWatcherClient::onConnected()
{
    qInfo() << "Connected to Client!";
    startWatching();
}

void FileWatcherClient::onDisconnected()
{
    qApp->exit(0);
    qWarning() << "Disconnected from Client.";
}

void FileWatcherClient::onFileChanged(const QString &path)
{
    qDebug() << "File changed:" << path;
    announceChange(path);
}

void FileWatcherClient::announceChange(const QString& path)
{
    QString urlString = QUrl::fromLocalFile(path).toString(QUrl::FullyEncoded);
    QString message = QString("live_reload %1").arg(urlString);
    qInfo() << "Announcing:" << urlString;
    m_webSocket.sendTextMessage(message);
}

void FileWatcherClient::aboutToQuit()
{
    m_webSocket.close();
}
