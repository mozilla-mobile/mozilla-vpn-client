/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tutorialnext.h"
#include "inspector/inspectorutils.h"
#include "leakdetector.h"
#include "logger.h"
#include "settingsholder.h"

#include <QJsonObject>
#include <QJsonValue>
#include <QMetaMethod>

namespace {
Logger logger(LOG_MAIN, "TutorialNext");

// We cannot use QMetaObject::indexOfSignal() because that wants the signal
// signature (with params) and we do not have them.
QMetaMethod signalByName(const QMetaObject* metaObject, const QString& name) {
  Q_ASSERT(metaObject);

  for (int i = 0; i < metaObject->methodCount(); ++i) {
    QMetaMethod method = metaObject->method(i);
    if (method.name() == name) {
      return method;
    }
  }

  return QMetaMethod();
}

}  // namespace

// static
TutorialNext* TutorialNext::create(QObject* parent, const QJsonValue& json) {
  QJsonObject obj = json.toObject();
  if (obj["op"] != "signal") {
    logger.error() << "Only 'signal' tutorial next are supported";
    return nullptr;
  }

  QString qmlEmitter = obj["qml_emitter"].toString();
  QString vpnEmitter = obj["vpn_emitter"].toString();
  if ((qmlEmitter.isEmpty() ? 0 : 1) + (vpnEmitter.isEmpty() ? 0 : 1) != 1) {
    logger.error()
        << "Only 1 qml_emitter or 1 vpn_emitter. Not none, not both.";
    return nullptr;
  }

  QString signal = obj["signal"].toString();
  if (signal.isEmpty()) {
    logger.error() << "No signal property";
    return nullptr;
  }

  EmitterType emitterType = QML;
  if (!vpnEmitter.isEmpty()) {
    if (vpnEmitter != "settingsHolder") {
      logger.error() << "Only 'settingsHolder' is supported as vpn_emitter";
      return nullptr;
    }

    emitterType = SettingsHolder;
  }

  return new TutorialNext(parent, emitterType, qmlEmitter, signal);
}

TutorialNext::TutorialNext(QObject* parent, EmitterType emitterType,
                           const QString& emitter, const QString& signal)
    : QObject(parent),
      m_emitterType(emitterType),
      m_emitter(emitter),
      m_signal(signal) {
  MVPN_COUNT_CTOR(TutorialNext);
}

TutorialNext::~TutorialNext() { MVPN_COUNT_DTOR(TutorialNext); }

void TutorialNext::startOrStop(bool start) {
  QObject* obj = nullptr;
  switch (m_emitterType) {
    case SettingsHolder:
      obj = ::SettingsHolder::instance();
      break;
    case QML:
      obj = InspectorUtils::findObject(m_emitter);
      break;
  }

  if (!obj) {
    logger.error() << "Unable to find the correct object";
    if (start) {
      emit completed();
    }
    return;
  }

  QMetaMethod signalA = signalByName(obj->metaObject(), m_signal);
  if (!signalA.isValid()) {
    logger.error() << "Unable to find the signal" << m_signal;
    if (start) {
      emit completed();
    }
    return;
  }

  QMetaMethod signalB = signalByName(metaObject(), "completed");
  Q_ASSERT(signalB.isValid());

  if (start) {
    connect(obj, signalA, this, signalB);
  } else {
    disconnect(obj, signalA, this, signalB);
  }
}

void TutorialNext::start() { startOrStop(true); }

void TutorialNext::stop() { startOrStop(false); }
