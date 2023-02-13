/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TESTHELPER_H
#define TESTHELPER_H

#include <QObject>
#include <QQmlEngine>
#include <QtQuickTest>

#include "i18nstrings.h"
#include "mozillavpn.h"
#include "settingsholder.h"
#include "theme.h"

class TestHelper final : public QObject {
  Q_OBJECT

 public:
  static TestHelper* instance();
  Q_INVOKABLE void triggerAboutToQuit() const;
  Q_INVOKABLE void triggerInitializeGlean() const;
  Q_INVOKABLE void triggerRecordGleanEvent(const QString& event) const;
  Q_INVOKABLE void triggerRecordGleanEventWithExtraKeys(
      const QString& event, const QVariantMap& keys) const;
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

  SettingsHolder m_settingsHolder;
  bool m_mainWindowLoadedCalled = false;
  bool m_debugMode = true;
  bool m_stagingMode = true;

  I18nStrings* m_i18nstrings = nullptr;
  MozillaVPN* m_mozillavpn = nullptr;
  Theme* m_theme = nullptr;
};

#endif  // TESTHELPER_H
