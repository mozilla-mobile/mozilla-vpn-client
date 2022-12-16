/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDONAPI_H
#define ADDONAPI_H

#include <QJSValue>
#include <QObject>

#include "env.h"

class Addon;

/*
 * IMPORTANT!! If you add, change or remove this object, please update the
 * documentation in `docs/add-on-api.md`.
 */

class AddonApi final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AddonApi)

  Q_PROPERTY(QJSValue addon READ addon CONSTANT)
  Q_PROPERTY(QJSValue controller READ controller CONSTANT)
  Q_PROPERTY(const Env* env READ env CONSTANT)
  Q_PROPERTY(QJSValue featureList READ featureList CONSTANT)
  Q_PROPERTY(QJSValue navigator READ navigator CONSTANT)
  Q_PROPERTY(QJSValue settings READ settings CONSTANT)
  Q_PROPERTY(QJSValue subscriptionData READ subscriptionData CONSTANT)
  Q_PROPERTY(QJSValue urlOpener READ urlOpener CONSTANT)

 public:
  explicit AddonApi(Addon* addon);
  ~AddonApi();

  Q_INVOKABLE void connectSignal(QObject* obj, const QString& signalName,
                                 const QJSValue& callback);

 private:
  QJSValue addon() const;
  QJSValue controller() const;
  const Env* env() const { return &m_env; }
  QJSValue featureList() const;
  QJSValue navigator() const;
  QJSValue settings() const;
  QJSValue subscriptionData() const;
  QJSValue urlOpener() const;

 private:
  Addon* m_addon = nullptr;
  Env m_env;
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
