/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowsapplistprovider.h"
#include "leakdetector.h"
#include "logger.h"
#include <QSettings>
#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QStandardPaths>
#include <Windows.h>

namespace {
Logger logger(LOG_WINDOWS, "AppListProvider");
}


WindowsAppListProvider::WindowsAppListProvider(QObject* parent)
    : AppListProvider(parent) {
  MVPN_COUNT_CTOR(WindowsAppListProvider);
}

WindowsAppListProvider::~WindowsAppListProvider() {
  MVPN_COUNT_DTOR(WindowsAppListProvider);
}

/**
 * @brief Assembles an Application list through reading all links in common locations
 */
void WindowsAppListProvider::getApplicationList() {
  QMap<QString, QString> appList;

  readLinkFiles("C:\\ProgramData\\Microsoft\\Windows\\Start Menu\\Programs", &appList);
  readLinkFiles(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "\\..\\Microsoft\\Windows\\Start Menu\\Programs", &appList);
  readLinkFiles(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation), &appList);

  emit newAppList(appList);
}
/**
 * @brief Reads all .lnk's in a Dir, filters them and Puts them into a QMap
 * @param path - Directory Path to read
 * @param out - QMap which valid links should be put into
 */
void WindowsAppListProvider::readLinkFiles(QString path,QMap<QString,QString>* out){
    logger.log() << "Read -> " <<path;
    QDirIterator it(path, QStringList() << "*.lnk", QDir::Files, QDirIterator::Subdirectories);
    int x =out->count();
    while (it.hasNext()) {
        QFileInfo link(it.next());
        QFileInfo target(link.symLinkTarget());

        if(target.completeSuffix() != "exe"){
            //1:  We only care for .exe
            logger.log() << "Skip -> " <<link.baseName() << "  " << target.absoluteFilePath();
            continue;
        }
        if(target.path().toUpper().startsWith("C:/WINDOWS/")){
            //2: Don't include windows links like cmd/ps
            logger.log() << "Skip -> " <<link.baseName() << "  " << target.absoluteFilePath();
            continue;
        }
        if(target.path().toLower().contains("uninstall") || link.baseName().toLower().contains("uninstall") ){
            //3: Don't include obvious uninstallers
            logger.log() << "Skip -> " <<link.baseName() << "  " << target.absoluteFilePath();
            continue;
        }
        logger.log() << "Add -> " <<link.baseName() << "  " << target.absoluteFilePath();
        out->insert(target.absoluteFilePath(),link.baseName());
    }
    logger.log() << " Added: " << out->count() - x;

}
