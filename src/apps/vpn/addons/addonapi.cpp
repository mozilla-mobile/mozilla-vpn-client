/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addonapi.h"

#include <QCoreApplication>
#include <QQmlEngine>

#include "addon.h"
#include "controller.h"
#include "frontend/navigator.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/featuremodel.h"
#include "models/subscriptiondata.h"
#include "mozillavpn.h"
#include "qmlengineholder.h"
#include "settingsholder.h"
#include "urlopener.h"

namespace {
Logger logger("AddonApi");
}

AddonApi::AddonApi(Addon* addon) : QObject(addon), m_addon(addon) {
  logger.debug() << "Create API for" << addon->id();
  MZ_COUNT_CTOR(AddonApi);
}

AddonApi::~AddonApi() { MZ_COUNT_DTOR(AddonApi); }

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

AddonApiCallbackWrapper::AddonApiCallbackWrapper(QObject* parent,
                                                 const QJSValue& callback)
    : QObject(parent), m_callback(callback) {}

void AddonApiCallbackWrapper::run() {
  logger.debug() << "Callback execution";
  m_callback.call();
}

QJSValue AddonApi::settings() const {
  QJSEngine* engine = QmlEngineHolder::instance()->engine();

  QObject* obj = SettingsHolder::instance();
  QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);

  QJSValue value = engine->newQObject(obj);
  value.setPrototype(engine->newQMetaObject(&SettingsHolder::staticMetaObject));
  return value;
}

QJSValue AddonApi::navigator() const {
  QJSEngine* engine = QmlEngineHolder::instance()->engine();

  QObject* obj = Navigator::instance();
  QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);

  QJSValue value = engine->newQObject(obj);
  value.setPrototype(engine->newQMetaObject(&Navigator::staticMetaObject));
  return value;
}

QJSValue AddonApi::urlOpener() const {
  QJSEngine* engine = QmlEngineHolder::instance()->engine();

  QObject* obj = UrlOpener::instance();
  QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);

  QJSValue value = engine->newQObject(obj);
  value.setPrototype(engine->newQMetaObject(&UrlOpener::staticMetaObject));
  return value;
}

QJSValue AddonApi::controller() const {
  QJSEngine* engine = QmlEngineHolder::instance()->engine();

  QObject* obj = MozillaVPN::instance()->controller();
  QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);

  QJSValue value = engine->newQObject(obj);
  value.setPrototype(engine->newQMetaObject(&Controller::staticMetaObject));
  return value;
}

QJSValue AddonApi::featureList() const {
  QJSEngine* engine = QmlEngineHolder::instance()->engine();

  QObject* obj = FeatureModel::instance();
  QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);

  QJSValue value = engine->newQObject(obj);
  value.setPrototype(engine->newQMetaObject(&FeatureModel::staticMetaObject));
  return value;
}

QJSValue AddonApi::addon() const {
  QJSEngine* engine = QmlEngineHolder::instance()->engine();

  QQmlEngine::setObjectOwnership(m_addon, QQmlEngine::CppOwnership);

  QJSValue value = engine->newQObject(m_addon);
  value.setPrototype(engine->newQMetaObject(&Addon::staticMetaObject));
  return value;
}

QJSValue AddonApi::subscriptionData() const {
  QJSEngine* engine = QmlEngineHolder::instance()->engine();

  QObject* obj = MozillaVPN::instance()->subscriptionData();
  QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);

  QJSValue value = engine->newQObject(obj);
  value.setPrototype(
      engine->newQMetaObject(&SubscriptionData::staticMetaObject));
  return value;
}

QJSValue AddonApi::vpn() const {
  QJSEngine* engine = QmlEngineHolder::instance()->engine();

  QObject* obj = MozillaVPN::instance();
  QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);

  QJSValue value = engine->newQObject(obj);
  value.setPrototype(engine->newQMetaObject(&MozillaVPN::staticMetaObject));
  return value;
}
