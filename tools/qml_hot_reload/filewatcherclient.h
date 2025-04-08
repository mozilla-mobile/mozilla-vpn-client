/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


#pragma once

#include <QObject>
#include <QFileSystemWatcher>
#include <QWebSocket>
#include <QTimer>
#include <QHash>
#include <QString>
#include <QUrl>
#include <QDir>
#include <QElapsedTimer>
#include <QSet> // Keep QSet

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
    QString m_watchRootPath; // Renamed for clarity - the root defined by CMake
    QDir m_watchRootDir;   // Renamed for clarity
    bool m_adbMode = false;
    bool m_isConnected = false;

    QHash<QString, QTimer*> m_debounceTimers;
    static const int DEBOUNCE_THRESHOLD_MS = 200;

    QSocketNotifier* m_stdinNotifier = nullptr;

    // Keep track of watched files/dirs using canonical paths
    QSet<QString> m_watchedFiles;
    QSet<QString> m_watchedDirs;

    // Removed: m_targetMode, m_targetQmlFiles, loadQmlFileList, watchSpecificFiles
};
