/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef HELPER_H
#define HELPER_H

#include "../../src/closeeventhandler.h"
#include "../../src/models/whatsnewmodel.h"
#include "../../src/settingsholder.h"

#include <QObject>
#include <QVector>
#include <QtTest/QtTest>

class TestHelper final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(TestHelper)

 public:
  ~TestHelper() = default;
  static TestHelper* instance();

 public:
  CloseEventHandler* closeEventHandler() {
    return &m_private->m_closeEventHandler;
  }
  WhatsNewModel* whatsNewModel() { return &m_private->m_whatsNewModel; }
  SettingsHolder* settingsHolder() { return &m_private->m_settings; }

  Q_INVOKABLE void triggerInitializeGlean();
  Q_INVOKABLE void triggerSetGleanSourceTags(const QStringList& tags);

 private:
  TestHelper();
  struct Private {
    CloseEventHandler m_closeEventHandler;
    SettingsHolder m_settings;
    WhatsNewModel m_whatsNewModel;
  };
  Private* m_private = nullptr;
};

#endif  // HELPER_H
