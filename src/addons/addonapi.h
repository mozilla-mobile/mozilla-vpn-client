/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDONAPI_H
#define ADDONAPI_H

#include "settingsholder.h"

#include <QJSValue>
#include <QObject>

class AddonApi final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AddonApi)

  Q_PROPERTY(SettingsHolder* settings READ settings CONSTANT)

 public:
  static AddonApi* instance();
  ~AddonApi();

  Q_INVOKABLE void connectSignal(QObject* obj, const QString& signalName,
                                 const QJSValue& callback);

 private:
  explicit AddonApi(QObject* parent);

  SettingsHolder* settings() const { return SettingsHolder::instance(); }
};

class AddonApiCallbackWrapper final : public QObject {
  Q_OBJECT

 public:
  AddonApiCallbackWrapper(QObject* parent, const QJSValue& callback);

 public slots:
  void run();

 private:
  const QJSValue m_callback;
};

#endif  // ADDONAPI_H
