/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "filewatcherclient.h"
#include <QCoreApplication>
#include <csignal>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("HotReloader");
    QCoreApplication::setApplicationVersion("1.0");

    FileWatcherClient client(&app);
    return app.exec();
}
