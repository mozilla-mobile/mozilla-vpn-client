/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macosmenubar.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"

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
  logger.log() << "Creating menubar";

  MozillaVPN* vpn = MozillaVPN::instance();

  m_menuBar = new QMenuBar(nullptr);

  //% "File"
  QMenu* fileMenu = m_menuBar->addMenu(qtTrId("menubar.file.title"));

  // Do not use qtTrId here!
  QAction* quit =
      fileMenu->addAction("quit", vpn->controller(), &Controller::quit);
  quit->setMenuRole(QAction::QuitRole);

  // Do not use qtTrId here!
  m_aboutAction =
      fileMenu->addAction("about.vpn", vpn, &MozillaVPN::requestAbout);
  m_aboutAction->setMenuRole(QAction::AboutRole);
  m_aboutAction->setVisible(vpn->state() == MozillaVPN::StateMain);

  // Do not use qtTrId here!
  m_preferencesAction =
      fileMenu->addAction("preferences", vpn, &MozillaVPN::requestSettings);
  m_preferencesAction->setMenuRole(QAction::PreferencesRole);
  m_preferencesAction->setVisible(vpn->state() == MozillaVPN::StateMain);

  m_closeAction = fileMenu->addAction("", vpn->controller(), &Controller::quit);
  m_closeAction->setShortcut(QKeySequence::Close);

  m_helpMenu = m_menuBar->addMenu("");

  retranslate();
};

void MacOSMenuBar::controllerStateChanged() {
  MozillaVPN* vpn = MozillaVPN::instance();
  m_preferencesAction->setVisible(vpn->state() == MozillaVPN::StateMain);
  m_aboutAction->setVisible(vpn->state() == MozillaVPN::StateMain);
}

void MacOSMenuBar::retranslate() {
  logger.log() << "Retranslate";

  //% "Close"
  m_closeAction->setText(qtTrId("menubar.file.close"));

  //% "Help"
  m_helpMenu->setTitle(qtTrId("menubar.help.title"));
  for (QAction* action : m_helpMenu->actions()) {
    m_helpMenu->removeAction(action);
  }

  MozillaVPN* vpn = MozillaVPN::instance();
  vpn->helpModel()->forEach([&](const char* nameId, int id) {
    m_helpMenu->addAction(qtTrId(nameId),
                          [help = vpn->helpModel(), id]() { help->open(id); });
  });
}
