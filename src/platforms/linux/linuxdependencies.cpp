/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "linuxdependencies.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>

constexpr const char *WG_QUICK = "wg-quick";

namespace {

void showAlert(const QString &message)
{
    QMessageBox alert;
    alert.setText(message);
    alert.exec();
}

bool findInPath(const char *what)
{
    char *path = getenv("PATH");
    Q_ASSERT(path);

    QStringList parts = QString(path).split(":");
    for (QStringList::ConstIterator i = parts.begin(); i != parts.end(); ++i) {
        QDir pathDir(*i);
        QFileInfo file(pathDir.filePath(what));
        if (file.exists()) {
            qDebug() << what << "found" << file.filePath();
            return true;
        }
    }

    return false;
}

} // namespace

// static
bool LinuxDependencies::checkDependencies()
{
    char *path = getenv("PATH");
    if (!path) {
        showAlert("No PATH env found.");
        return false;
    }

    if (!findInPath(WG_QUICK)) {
        showAlert("Unable to locate wg-quick");
        return false;
    }

    return true;
}
