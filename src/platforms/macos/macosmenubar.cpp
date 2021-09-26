/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macosmenubar.h"
#include "core.h"
#include "leakdetector.h"
#include "logger.h"
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

  Core* core = Core::instance();

  m_menuBar = new QMenuBar(nullptr);

  //% "File"
  QMenu* fileMenu = m_menuBar->addMenu(qtTrId("menubar.file.title"));

  // Do not use qtTrId here!
  QAction* quit =
      fileMenu->addAction("quit", core->controller(), &Controller::quit);
  quit->setMenuRole(QAction::QuitRole);

  // Do not use qtTrId here!
  m_aboutAction = fileMenu->addAction("about.vpn", core, &Core::requestAbout);
  m_aboutAction->setMenuRole(QAction::AboutRole);
  m_aboutAction->setVisible(core->state() == Core::StateMain);

  // Do not use qtTrId here!
  m_preferencesAction =
      fileMenu->addAction("preferences", core, &Core::requestSettings);
  m_preferencesAction->setMenuRole(QAction::PreferencesRole);
  m_preferencesAction->setVisible(core->state() == Core::StateMain);

  m_closeAction = fileMenu->addAction("close", []() {
    QmlEngineHolder::instance()->hideWindow();
#ifdef MVPN_MACOS
    MacOSUtils::hideDockIcon();
#endif
  });
  m_closeAction->setShortcut(QKeySequence::Close);

  m_helpMenu = m_menuBar->addMenu("");

  retranslate();
};

void MacOSMenuBar::controllerStateChanged() {
  Core* core = Core::instance();
  m_preferencesAction->setVisible(core->state() == Core::StateMain);
  m_aboutAction->setVisible(core->state() == Core::StateMain);
}

void MacOSMenuBar::retranslate() {
  logger.debug() << "Retranslate";

  //% "Close"
  m_closeAction->setText(qtTrId("menubar.file.close"));

  //% "Help"
  m_helpMenu->setTitle(qtTrId("menubar.help.title"));
  for (QAction* action : m_helpMenu->actions()) {
    m_helpMenu->removeAction(action);
  }

  Core* core = Core::instance();
  core->helpModel()->forEach([&](const char* nameId, int id) {
    m_helpMenu->addAction(qtTrId(nameId),
                          [help = core->helpModel(), id]() { help->open(id); });
  });
}
