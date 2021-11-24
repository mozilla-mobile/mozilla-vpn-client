/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TESTHELPER_H
#define TESTHELPER_H

#include "closeeventhandler.h"
#include "l18nstrings.h"
#include "models/whatsnewmodel.h"
#include "mozillavpn.h"

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
  Q_PROPERTY(bool mainWindowLoadedCalled READ mainWindowLoadedCalled)
  Q_PROPERTY(bool stagingMode READ stagingMode WRITE setStagingMode)
  Q_PROPERTY(bool debugMode READ debugMode WRITE setDebugMode)

  bool mainWindowLoadedCalled() const;
  void setMainWindowLoadedCalled(bool val);
  bool stagingMode() const;
  void setStagingMode(bool val);
  bool debugMode() const;
  void setDebugMode(bool val);

 public slots:
  // For info on the slots we can use
  // https://doc.qt.io/qt-5/qtquicktest-index.html#executing-c-before-qml-tests
  void qmlEngineAvailable(QQmlEngine* engine);

 private:
  TestHelper();
  ~TestHelper() = default;

  bool m_mainWindowLoadedCalled = false;
  bool m_debugMode = true;
  bool m_stagingMode = true;

  CloseEventHandler* m_closeEventHandler = nullptr;
  L18nStrings* m_l18nstrings = nullptr;
  WhatsNewModel* m_whatsNewModel = nullptr;
};

#endif  // TESTHELPER_H
