/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TESTHELPER_H
#define TESTHELPER_H

#include "../../src/closeeventhandler.h"
#include "../../src/models/whatsnewmodel.h"

#include <QObject>
#include <QQmlEngine>
#include <QtQuickTest>

class TestHelper final : public QObject {
  Q_OBJECT

 public:
  TestHelper();
  Q_INVOKABLE void triggerAboutToQuit();
  Q_INVOKABLE void triggerInitializeGlean();
  Q_INVOKABLE void triggerSetGleanSourceTags(const QStringList& tags);
  Q_INVOKABLE QString osVersion();
  Q_INVOKABLE QString architecture();

  CloseEventHandler closeEventHandler;
  WhatsNewModel* whatsNewModel() { return m_whatsNewModel; }

 public slots:
  // For info on the slots we can use
  // https://doc.qt.io/qt-5/qtquicktest-index.html#executing-c-before-qml-tests
  void qmlEngineAvailable(QQmlEngine* engine);

 private:
  WhatsNewModel* m_whatsNewModel = nullptr;
};

#endif  // TESTHELPER_H