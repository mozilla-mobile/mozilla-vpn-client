/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addonapi.h"

#include <QCoreApplication>
#include <QQmlEngine>

#include "addon.h"
#include "env.h"
#include "frontend/navigator.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/featuremodel.h"
#include "qmlengineholder.h"
#include "settingsholder.h"
#include "urlopener.h"

namespace {
Logger logger("AddonApi");
std::function<void(AddonApi*)> s_constructorCallback;
}  // namespace

AddonApi::AddonApi(Addon* addon)
    : QQmlPropertyMap(this, addon), m_addon(addon) {
  logger.debug() << "Create API for" << addon->id();
  MZ_COUNT_CTOR(AddonApi);

  initialize();
}

AddonApi::~AddonApi() { MZ_COUNT_DTOR(AddonApi); }

void AddonApi::initialize() {
  QJSEngine* engine = QmlEngineHolder::instance()->engine();

  {
    QQmlEngine::setObjectOwnership(m_addon, QQmlEngine::CppOwnership);

    QJSValue value = engine->newQObject(m_addon);
    value.setPrototype(engine->newQMetaObject(m_addon->metaObject()));

    insert("addon", QVariant::fromValue(value));
  }

  insert("env", QVariant::fromValue(Env::instance()));

  {
    QObject* obj = FeatureModel::instance();
    QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);

    QJSValue value = engine->newQObject(obj);
    value.setPrototype(engine->newQMetaObject(&FeatureModel::staticMetaObject));

    insert("featureList", QVariant::fromValue(value));
  }

  {
    QObject* obj = Navigator::instance();
    QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);

    QJSValue value = engine->newQObject(obj);
    value.setPrototype(engine->newQMetaObject(&Navigator::staticMetaObject));

    insert("navigator", QVariant::fromValue(value));
  }

  {
    QObject* obj = SettingsHolder::instance();
    QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);

    QJSValue value = engine->newQObject(obj);
    value.setPrototype(
        engine->newQMetaObject(&SettingsHolder::staticMetaObject));

    insert("settings", QVariant::fromValue(value));
  }

  {
    QObject* obj = UrlOpener::instance();
    QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);

    QJSValue value = engine->newQObject(obj);
    value.setPrototype(engine->newQMetaObject(&UrlOpener::staticMetaObject));

    insert("urlOpener", QVariant::fromValue(value));
  }

  if (s_constructorCallback) {
    s_constructorCallback(this);
  }

  m_timer.setSingleShot(true);
}

void AddonApi::setTimedCallback(int interval, const QJSValue& callback) {
  if (!callback.isCallable()) {
    logger.debug() << "No callback received";
    return;
  }

  //disconnect a potential previous timer
  QObject::disconnect(&m_timer, nullptr, nullptr, nullptr);

  connect(&m_timer, &QTimer::timeout, this, [callback]() { callback.call(); });

  m_timer.start(interval);
}

void AddonApi::log(const QString& message) { logger.debug() << message; }

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

// static
void AddonApi::setConstructorCallback(
    std::function<void(AddonApi*)>&& callback) {
  s_constructorCallback = std::move(callback);
}

AddonApiCallbackWrapper::AddonApiCallbackWrapper(QObject* parent,
                                                 const QJSValue& callback)
    : QObject(parent), m_callback(callback) {}

void AddonApiCallbackWrapper::run() {
  logger.debug() << "Callback execution";
  m_callback.call();
}
