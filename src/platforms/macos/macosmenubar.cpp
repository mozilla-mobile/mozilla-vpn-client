/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macosmenubar.h"
#include "externalophandler.h"
#include "frontend/navigator.h"
#include "l18nstrings.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "qmlengineholder.h"
#ifdef MVPN_MACOS
#  include "platforms/macos/macosutils.h"
#endif

#include <QAction>
#include <QMenu>
#include <QMenuBar>

namespace {
Logger logger(LOG_MACOS, "MacOSManuBar");
MacOSMenuBar* s_instance = nullptr;
}  // namespace

MacOSMenuBar::MacOSMenuBar() {
  MVPN_COUNT_CTOR(MacOSMenuBar);

  Q_ASSERT(!s_instance);
  s_instance = this;
}

MacOSMenuBar::~MacOSMenuBar() {
  MVPN_COUNT_DTOR(MacOSMenuBar);

  Q_ASSERT(s_instance == this);
  s_instance = nullptr;
}

// static
MacOSMenuBar* MacOSMenuBar::instance() {
  Q_ASSERT(s_instance);
  return s_instance;
}

void MacOSMenuBar::initialize() {
  logger.debug() << "Creating menubar";

  MozillaVPN* vpn = MozillaVPN::instance();

  m_menuBar = new QMenuBar(nullptr);

  //% "File"
  QMenu* fileMenu = m_menuBar->addMenu(qtTrId("menubar.file.title"));

  // Do not use qtTrId here!
  QAction* quit =
      fileMenu->addAction("quit", vpn->controller(), &Controller::quit);
  quit->setMenuRole(QAction::QuitRole);

  // Do not use qtTrId here!
  m_aboutAction = fileMenu->addAction("about.vpn", []() {
    ExternalOpHandler::instance()->request(ExternalOpHandler::OpAbout);
  });
  m_aboutAction->setMenuRole(QAction::AboutRole);
  m_aboutAction->setVisible(vpn->state() == MozillaVPN::StateMain);

  // Do not use qtTrId here!
  m_preferencesAction = fileMenu->addAction("preferences", []() {
    ExternalOpHandler::instance()->request(ExternalOpHandler::OpSettings);
  });
  m_preferencesAction->setMenuRole(QAction::PreferencesRole);
  m_preferencesAction->setVisible(vpn->state() == MozillaVPN::StateMain);

  m_closeAction = fileMenu->addAction("close", []() {
    QmlEngineHolder::instance()->hideWindow();
#ifdef MVPN_MACOS
    MacOSUtils::hideDockIcon();
#endif
  });
  m_closeAction->setShortcut(QKeySequence::Close);

  m_helpAction = m_menuBar->addAction("", []() {
    ExternalOpHandler::instance()->request(ExternalOpHandler::OpGetHelp);
  });

  retranslate();
};

void MacOSMenuBar::controllerStateChanged() {
  MozillaVPN* vpn = MozillaVPN::instance();
  m_preferencesAction->setVisible(vpn->state() == MozillaVPN::StateMain);
  m_aboutAction->setVisible(vpn->state() == MozillaVPN::StateMain);
}

void MacOSMenuBar::retranslate() {
  logger.debug() << "Retranslate";

  //% "Close"
  m_closeAction->setText(qtTrId("menubar.file.close"));

  L18nStrings* l18nStrings = L18nStrings::instance();
  Q_ASSERT(l18nStrings);

  m_helpAction->setText(l18nStrings->t(L18nStrings::SystrayHelp));
}
