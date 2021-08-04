/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "command.h"
#include "commandlineparser.h"
#include "constants.h"
#include "leakdetector.h"
#include "localizer.h"
#include "logger.h"
#include "loghandler.h"
#include "mozillavpn.h"
#include "settingsholder.h"
#include "simplenetworkmanager.h"

#ifdef MVPN_WINDOWS
#  include <Windows.h>
#endif

#include <QApplication>
#include <QIcon>
#include <QTextStream>

namespace {
Logger logger(LOG_MAIN, "Command");
}

QVector<std::function<Command*(QObject*)>> Command::s_commandCreators;

Command::Command(QObject* parent, const QString& name,
                 const QString& description)
    : QObject(parent), m_name(name), m_description(description) {
  MVPN_COUNT_CTOR(Command);
}

Command::~Command() { MVPN_COUNT_DTOR(Command); }

bool Command::userAuthenticated() {
  if (!SettingsHolder::instance()->hasToken()) {
    QTextStream stream(stdout);
    stream << "User status: not authenticated" << Qt::endl;
    return false;
  }

  return true;
}

bool Command::loadModels() {
  MozillaVPN* vpn = MozillaVPN::instance();

  // First the keys!
  if (!vpn->keys()->fromSettings()) {
    QTextStream stream(stdout);
    stream << "No cache available" << Qt::endl;
    return false;
  }

  if (!vpn->deviceModel()->fromSettings(vpn->keys()) ||
      !vpn->serverCountryModel()->fromSettings() ||
      !vpn->user()->fromSettings() || !vpn->currentServer()->fromSettings() ||
      !vpn->modelsInitialized()) {
    QTextStream stream(stdout);
    stream << "No cache available" << Qt::endl;
    return false;
  }

  if (!vpn->captivePortal()->fromSettings()) {
    // We do not care about these settings.
  }

  return true;
}

int Command::runCommandLineApp(std::function<int()>&& a_callback) {
  std::function<int()> callback = std::move(a_callback);

  // Our logging system.
  qInstallMessageHandler(LogHandler::messageQTHandler);
  logger.info() << "MozillaVPN" << APP_VERSION;
  logger.info() << "User-Agent:" << NetworkManager::userAgent();

  QCoreApplication app(CommandLineParser::argc(), CommandLineParser::argv());

  QCoreApplication::setApplicationName("Mozilla VPN");
  QCoreApplication::setApplicationVersion(APP_VERSION);

  SettingsHolder settingsHolder;
  Localizer localizer;
  SimpleNetworkManager snm;

  return callback();
}

int Command::runGuiApp(std::function<int()>&& a_callback) {
  std::function<int()> callback = std::move(a_callback);

  // Our logging system.
  qInstallMessageHandler(LogHandler::messageQTHandler);
  logger.debug() << "MozillaVPN" << APP_VERSION;
  logger.debug() << "User-Agent:" << NetworkManager::userAgent();

  QApplication app(CommandLineParser::argc(), CommandLineParser::argv());

  QCoreApplication::setApplicationName("Mozilla VPN");
  QCoreApplication::setApplicationVersion(APP_VERSION);

  SettingsHolder settingsHolder;
  Localizer localizer;
  SimpleNetworkManager snm;

  QIcon icon(Constants::LOGO_URL);
  app.setWindowIcon(icon);

  return callback();
}

int Command::runQmlApp(std::function<int()>&& a_callback) {
  std::function<int()> callback = std::move(a_callback);

  // Our logging system.
  qInstallMessageHandler(LogHandler::messageQTHandler);
  logger.debug() << "MozillaVPN" << APP_VERSION;
  logger.debug() << "User-Agent:" << NetworkManager::userAgent();

#ifdef MVPN_WINDOWS
  SetProcessDPIAware();
#endif

  QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

  QApplication app(CommandLineParser::argc(), CommandLineParser::argv());

  QCoreApplication::setApplicationName("Mozilla VPN");
  QCoreApplication::setApplicationVersion(APP_VERSION);

  SettingsHolder settingsHolder;
  Localizer localizer;

  QIcon icon(Constants::LOGO_URL);
  app.setWindowIcon(icon);

  return callback();
}

// static
QVector<Command*> Command::commands(QObject* parent) {
  QVector<Command*> list;
  for (auto i = s_commandCreators.begin(); i != s_commandCreators.end(); ++i) {
    list.append((*i)(parent));
  }
  return list;
}
