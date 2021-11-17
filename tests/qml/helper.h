/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TESTHELPER_H
#define TESTHELPER_H

#include "closeeventhandler.h"
#include "models/whatsnewmodel.h"
#include "settingsholder.h"

#include <QObject>
#include <QQmlEngine>
#include <QtQuickTest>

class TestHelper final : public QObject {
  Q_OBJECT

 public:
  static TestHelper* instance();
  Q_INVOKABLE void triggerAboutToQuit() const;
  Q_INVOKABLE void triggerInitializeGlean() const;
  Q_INVOKABLE void triggerRecordGleanEvent(const QString& event) const;
  Q_INVOKABLE void triggerSendGleanPings() const;
  Q_INVOKABLE void triggerSetGleanSourceTags(const QStringList& tags) const;
  Q_INVOKABLE QString osVersion() const;
  Q_INVOKABLE QString architecture() const;
  Q_PROPERTY(bool mainWindowLoadedCalled READ mainWindowLoadedCalled)

  CloseEventHandler closeEventHandler;
  SettingsHolder settingsHolder;
  WhatsNewModel* whatsNewModel() { return m_whatsNewModel; }
  bool mainWindowLoadedCalled() const;
  void setMainWindowLoadedCalled(bool val);

 public slots:
  // For info on the slots we can use
  // https://doc.qt.io/qt-5/qtquicktest-index.html#executing-c-before-qml-tests
  void qmlEngineAvailable(QQmlEngine* engine);

 private:
  TestHelper();
  ~TestHelper() = default;
  WhatsNewModel* m_whatsNewModel = nullptr;
  bool m_mainWindowLoadedCalled = false;
};

#endif  // TESTHELPER_H