/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


#pragma once

#include <QObject>
#include <QFileSystemWatcher>
#include <QWebSocket>
#include <QString>
#include <QUrl>
#include <QDir>

class QCoreApplication;
class QSocketNotifier;

class FileWatcherClient : public QObject
{
    Q_OBJECT

public:
    explicit FileWatcherClient(QCoreApplication* app, QObject *parent = nullptr);
    ~FileWatcherClient() override;

    void startWatching();

private slots:
    void onConnected();
    void onDisconnected();

    void onFileChanged(const QString &path);
    void aboutToQuit();

private:
    void announceChange(const QString& path);

    QCoreApplication* m_app = nullptr;
    QFileSystemWatcher m_fileWatcher;
    QWebSocket m_webSocket;
    QUrl m_wsUrl;
    QString m_origin;
    QString m_watchRootPath;
};
