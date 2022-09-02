/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addonapi.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "qmlengineholder.h"

#include <QCoreApplication>
#include <QQmlEngine>

namespace {
Logger logger(LOG_MAIN, "AddonApi");

AddonApi* s_instance = nullptr;
}  // namespace

// static
AddonApi* AddonApi::instance() {
  if (!s_instance) {
    s_instance = new AddonApi(qApp);
  }

  return s_instance;
}

AddonApi::AddonApi(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(AddonApi);
}

AddonApi::~AddonApi() { MVPN_COUNT_DTOR(AddonApi); }

void AddonApi::connectSignal(QObject* obj, const QString& signalName,
                             const QJSValue& callback) {
  if (!obj) {
    logger.debug() << "connectSignal called with a null object";
    return;
  }

  if (!callback.isCallable()) {
    logger.debug() << "No callback received";
    return;
  }

  const QMetaObject* metaObject = obj->metaObject();
  if (!metaObject) {
    logger.debug() << "Unable to find the metaobject of the object";
    return;
  }

  QMetaMethod signal;
  for (int i = 0; i < metaObject->methodCount(); ++i) {
    QMetaMethod method = metaObject->method(i);
    if (method.name() == signalName) {
      signal = method;
      break;
    }
  }

  if (!signal.isValid()) {
    logger.debug() << "Unable to find the signal" << signalName;
    return;
  }

  AddonApiCallbackWrapper* cw = new AddonApiCallbackWrapper(this, callback);

  QMetaMethod slot =
      cw->metaObject()->method(cw->metaObject()->indexOfSlot("run()"));
  Q_ASSERT(slot.isValid());

  connect(obj, signal, cw, slot);
}

void AddonApi::openURL(const QString& url) const {
  MozillaVPN::instance()->instance()->openLinkUrl(url);
}

AddonApiCallbackWrapper::AddonApiCallbackWrapper(QObject* parent,
                                                 const QJSValue& callback)
    : QObject(parent), m_callback(callback) {}

void AddonApiCallbackWrapper::run() {
  logger.debug() << "Callback execution";
  m_callback.call();
}

QJSValue AddonApi::settings() const {
  QJSEngine* engine = QmlEngineHolder::instance()->engine();
  QJSValue value = engine->newQObject(SettingsHolder::instance());
  value.setPrototype(engine->newQMetaObject(&SettingsHolder::staticMetaObject));
  return value;
}

QJSValue AddonApi::navigator() const {
  QJSEngine* engine = QmlEngineHolder::instance()->engine();
  QJSValue value = engine->newQObject(Navigator::instance());
  value.setPrototype(engine->newQMetaObject(&Navigator::staticMetaObject));
  return value;
}
