/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "command.h"
#include "localizer.h"
#include "mozillavpn.h"
#include "settingsholder.h"
#include "simplenetworkmanager.h"

#include <QApplication>
#include <QIcon>
#include <QTextStream>

QVector<Command *> Command::s_commands;

Command::Command(const QString &name, const QString &description)
    : m_name(name), m_description(description)
{
    s_commands.append(this);
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

    return true;
}

int Command::runCommandLineApp(std::function<int()> &&a_callback)
{
    std::function<int()> callback = std::move(a_callback);

    constexpr const char *APP_NAME = "mozillavpn";
    int argc = 1;
    QCoreApplication app(argc, (char **) &APP_NAME);

    QCoreApplication::setApplicationName("Mozilla VPN");
    QCoreApplication::setApplicationVersion(APP_VERSION);

    SettingsHolder settingsHolder;
    Localizer::createInstance(&settingsHolder);
    SimpleNetworkManager snm;

    return callback();
}

int Command::runGuiApp(std::function<int()> &&a_callback)
{
    std::function<int()> callback = std::move(a_callback);

    constexpr const char *APP_NAME = "mozillavpn";
    int argc = 1;
    QApplication app(argc, (char **) &APP_NAME);

    QCoreApplication::setApplicationName("Mozilla VPN");
    QCoreApplication::setApplicationVersion(APP_VERSION);

    SettingsHolder settingsHolder;
    Localizer::createInstance(&settingsHolder);
    SimpleNetworkManager snm;

    QIcon icon("://ui/resources/logo-dock.png");
    app.setWindowIcon(icon);

    return callback();
}

int Command::runQmlApp(std::function<int()> &&a_callback)
{
    std::function<int()> callback = std::move(a_callback);

    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    constexpr const char *APP_NAME = "mozillavpn";
    int argc = 1;
    QApplication app(argc, (char **) &APP_NAME);

    QCoreApplication::setApplicationName("Mozilla VPN");
    QCoreApplication::setApplicationVersion(APP_VERSION);

    SettingsHolder settingsHolder;
    Localizer::createInstance(&settingsHolder);

    QIcon icon("://ui/resources/logo-dock.png");
    app.setWindowIcon(icon);

    return callback();
}
