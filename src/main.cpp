/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commandlineparser.h"
#include "localizer.h"
#include "logger.h"
#include "loghandler.h"
#include "settingsholder.h"

#include <QApplication>
#include <QIcon>

namespace {
Logger logger(LOG_MAIN, "main");
}

int main(int argc, char *argv[])
{
    // Our logging system.
    qInstallMessageHandler(LogHandler::messageQTHandler);

    logger.log() << "MozillaVPN" << APP_VERSION;

    CommandLineParser clp;
    return clp.parse(argc, argv);
}
