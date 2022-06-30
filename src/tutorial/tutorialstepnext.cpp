/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tutorialstepnext.h"
#include "inspector/inspectorutils.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "settingsholder.h"

#include <QJsonObject>
#include <QJsonValue>
#include <QMetaMethod>

namespace {
Logger logger(LOG_MAIN, "TutorialStepNext");

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
TutorialStepNext* TutorialStepNext::create(QObject* parent,
                                           const QJsonValue& json) {
  QJsonObject obj = json.toObject();
  if (obj["op"] != "signal") {
    logger.warning() << "Only 'signal' tutorial next are supported";
    return nullptr;
  }

  QString qmlEmitter = obj["qml_emitter"].toString();
  QString vpnEmitter = obj["vpn_emitter"].toString();
  if ((qmlEmitter.isEmpty() ? 0 : 1) + (vpnEmitter.isEmpty() ? 0 : 1) != 1) {
    logger.warning()
        << "Only 1 qml_emitter or 1 vpn_emitter. Not none, not both.";
    return nullptr;
  }

  QString signal = obj["signal"].toString();
  if (signal.isEmpty()) {
    logger.warning() << "No signal property";
    return nullptr;
  }

  EmitterType emitterType = QML;
  if (!vpnEmitter.isEmpty()) {
    if (vpnEmitter == "settingsHolder") {
      emitterType = SettingsHolder;
    } else if (vpnEmitter == "controller") {
      emitterType = Controller;
    } else {
      logger.warning()
          << "Only 'settingsHolder' and 'controller' are supported "
             "as vpn_emitter";
      return nullptr;
    }
  }

  return new TutorialStepNext(parent, emitterType, qmlEmitter, signal);
}

TutorialStepNext::TutorialStepNext(QObject* parent, EmitterType emitterType,
                                   const QString& emitter,
                                   const QString& signal)
    : QObject(parent),
      m_emitterType(emitterType),
      m_emitter(emitter),
      m_signal(signal) {
  MVPN_COUNT_CTOR(TutorialStepNext);
}

TutorialStepNext::~TutorialStepNext() { MVPN_COUNT_DTOR(TutorialStepNext); }

void TutorialStepNext::startOrStop(bool start) {
  QObject* obj = nullptr;
  switch (m_emitterType) {
    case SettingsHolder:
      obj = ::SettingsHolder::instance();
      break;
    case Controller:
      obj = MozillaVPN::instance()->controller();
      break;
    case QML:
      obj = InspectorUtils::findObject(m_emitter);
      break;
  }

  if (!obj) {
    logger.warning() << "Unable to find the correct object";
    if (start) {
      emit completed();
    }
    return;
  }

  QMetaMethod signalA = signalByName(obj->metaObject(), m_signal);
  if (!signalA.isValid()) {
    logger.warning() << "Unable to find the signal" << m_signal;
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

void TutorialStepNext::start() { startOrStop(true); }

void TutorialStepNext::stop() { startOrStop(false); }
