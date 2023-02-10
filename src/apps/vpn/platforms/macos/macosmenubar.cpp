/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macosmenubar.h"

#include "externalophandler.h"
#include "frontend/navigator.h"
#include "i18nstrings.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "qmlengineholder.h"
#ifdef MZ_MACOS
#  include "platforms/macos/macosutils.h"
#endif

#include <QAction>
#include <QMenu>
#include <QMenuBar>

namespace {
Logger logger("MacOSManuBar");
MacOSMenuBar* s_instance = nullptr;
}  // namespace

MacOSMenuBar::MacOSMenuBar() {
  MZ_COUNT_CTOR(MacOSMenuBar);

  Q_ASSERT(!s_instance);
  s_instance = this;
}

MacOSMenuBar::~MacOSMenuBar() {
  MZ_COUNT_DTOR(MacOSMenuBar);

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
    (void)ExternalOpHandler::instance()->request(ExternalOpHandler::OpAbout);
  });
  m_aboutAction->setMenuRole(QAction::AboutRole);
  m_aboutAction->setVisible(vpn->state() == MozillaVPN::StateMain);

  m_closeAction = fileMenu->addAction("close", []() {
    QmlEngineHolder::instance()->hideWindow();
#ifdef MZ_MACOS
    MacOSUtils::hideDockIcon();
#endif
  });
  m_closeAction->setShortcut(QKeySequence::Close);

  retranslate();
};

void MacOSMenuBar::controllerStateChanged() {
  MozillaVPN* vpn = MozillaVPN::instance();
  m_aboutAction->setVisible(vpn->state() == MozillaVPN::StateMain);
}

void MacOSMenuBar::retranslate() {
  logger.debug() << "Retranslate";

  //% "Close"
  m_closeAction->setText(qtTrId("menubar.file.close"));
}
