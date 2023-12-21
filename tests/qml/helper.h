/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TESTHELPER_H
#define TESTHELPER_H

#include <QObject>
#include <QQmlEngine>
#include <QtQuickTest>

#include "i18nstrings.h"
#include "localizer.h"
#include "mozillavpn.h"
#include "settings/settinggroup.h"
#include "settingsholder.h"
#include "theme.h"

class TestHelper final : public QObject {
  Q_OBJECT

 public:
  static TestHelper* instance();
  Q_PROPERTY(bool mainWindowLoadedCalled READ mainWindowLoadedCalled)

  bool mainWindowLoadedCalled() const;
  void setMainWindowLoadedCalled(bool val);

 public slots:
  // For info on the slots we can use
  // https://doc.qt.io/qt-5/qtquicktest-index.html#executing-c-before-qml-tests
  void qmlEngineAvailable(QQmlEngine* engine);

 private:
  TestHelper();
  ~TestHelper() = default;

  SettingsHolder m_settingsHolder;
  Localizer m_localizer;

  bool m_mainWindowLoadedCalled = false;

  I18nStrings* m_i18nstrings = nullptr;
  MozillaVPN* m_mozillavpn = nullptr;
  Theme* m_theme = nullptr;

  SettingGroup* m_testSettingGroup;
};

#endif  // TESTHELPER_H
