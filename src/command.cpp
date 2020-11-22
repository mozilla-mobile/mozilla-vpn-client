/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "command.h"
#include "commandlineparser.h"
#include "leakdetector.h"
#include "localizer.h"
#include "mozillavpn.h"
#include "settingsholder.h"
#include "simplenetworkmanager.h"

#include <QApplication>
#include <QIcon>
#include <QTextStream>

QVector<std::function<Command *(QObject *)>> Command::s_commandCreators;

Command::Command(QObject *parent, const QString &name, const QString &description)
    : QObject(parent), m_name(name), m_description(description)
{
    MVPN_COUNT_CTOR(Command);
}

Command::~Command()
{
    MVPN_COUNT_DTOR(Command);
}

bool Command::userAuthenticated()
{
    if (!SettingsHolder::instance()->hasToken()) {
        QTextStream stream(stdout);
        stream << "User status: not authenticated" << Qt::endl;
        return false;
    }

    return true;
}

bool Command::loadModels()
{
    MozillaVPN *vpn = MozillaVPN::instance();

    if (!vpn->deviceModel()->fromSettings() || !vpn->serverCountryModel()->fromSettings()
        || !vpn->user()->fromSettings() || !vpn->keys()->fromSettings()
        || !vpn->currentServer()->fromSettings() || !vpn->modelsInitialized()) {
        QTextStream stream(stdout);
        stream << "No cache available" << Qt::endl;
        return false;
    }

    if (!vpn->captivePortal()->fromSettings()) {
        // We do not care about these settings.
    }

    return true;
}

int Command::runCommandLineApp(std::function<int()> &&a_callback)
{
    std::function<int()> callback = std::move(a_callback);

    QCoreApplication app(CommandLineParser::argc(), CommandLineParser::argv());

    QCoreApplication::setApplicationName("Mozilla VPN");
    QCoreApplication::setApplicationVersion(APP_VERSION);

    SettingsHolder settingsHolder;
    Localizer localizer;
    SimpleNetworkManager snm;

    return callback();
}

int Command::runGuiApp(std::function<int()> &&a_callback)
{
    std::function<int()> callback = std::move(a_callback);

    QApplication app(CommandLineParser::argc(), CommandLineParser::argv());

    QCoreApplication::setApplicationName("Mozilla VPN");
    QCoreApplication::setApplicationVersion(APP_VERSION);

    SettingsHolder settingsHolder;
    Localizer localizer;
    SimpleNetworkManager snm;

    QIcon icon(":/ui/resources/logo-dock.png");
    app.setWindowIcon(icon);

    return callback();
}

int Command::runQmlApp(std::function<int()> &&a_callback)
{
    std::function<int()> callback = std::move(a_callback);

    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication app(CommandLineParser::argc(), CommandLineParser::argv());

    QCoreApplication::setApplicationName("Mozilla VPN");
    QCoreApplication::setApplicationVersion(APP_VERSION);

    SettingsHolder settingsHolder;
    Localizer localizer;

    QIcon icon(":/ui/resources/logo-dock.png");
    app.setWindowIcon(icon);

    return callback();
}

// static
QVector<Command *> Command::commands(QObject *parent)
{
    QVector<Command *> list;
    for (auto i = s_commandCreators.begin(); i != s_commandCreators.end(); ++i) {
        list.append((*i)(parent));
    }
    return list;
}
