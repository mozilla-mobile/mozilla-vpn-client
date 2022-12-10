/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tutorialstepnext.h"

#include <QJsonObject>
#include <QJsonValue>
#include <QMetaMethod>

#include "inspector/inspectorutils.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "settingsholder.h"

namespace {
Logger logger("TutorialStepNext");

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

QMap<QString, QMap<QString, std::function<QObject*()>>> s_emitters;
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
  std::function<QObject*()> emitterGetter = nullptr;

  int count = qmlEmitter.isEmpty() ? 0 : 1;
  for (QMap<QString, QMap<QString, std::function<QObject*()>>>::const_iterator
           i = s_emitters.constBegin();
       i != s_emitters.constEnd(); ++i) {
    if (obj.contains(i.key())) {
      ++count;
      if (count > 1) {
        logger.warning() << "Only 1 emitter type is supported.";
        return nullptr;
      }

      QString emitterStr = obj[i.key()].toString();
      if (!i.value().contains(emitterStr)) {
        logger.warning() << "Emitter group" << i.key()
                         << "does not contain emitter" << emitterStr;
        return nullptr;
      }

      emitterGetter = i.value()[emitterStr];
      Q_ASSERT(emitterGetter);
    }
  }

  Q_ASSERT(count <= 1);
  if (count == 0) {
    logger.warning() << "No emitter found";
    return nullptr;
  }

  QString signal = obj["signal"].toString();
  if (signal.isEmpty()) {
    logger.warning() << "No signal property";
    return nullptr;
  }

  return new TutorialStepNext(parent, qmlEmitter, std::move(emitterGetter),
                              signal);
}

TutorialStepNext::TutorialStepNext(QObject* parent, const QString& qmlEmitter,
                                   std::function<QObject*()>&& emitterGetter,
                                   const QString& signal)
    : QObject(parent),
      m_qmlEmitter(qmlEmitter),
      m_emitterGetter(std::move(emitterGetter)),
      m_signal(signal) {
  MVPN_COUNT_CTOR(TutorialStepNext);
}

TutorialStepNext::~TutorialStepNext() { MVPN_COUNT_DTOR(TutorialStepNext); }

void TutorialStepNext::startOrStop(bool start) {
  QObject* obj = nullptr;
  if (!m_qmlEmitter.isEmpty()) {
    obj = InspectorUtils::findObject(m_qmlEmitter);
  } else {
    obj = m_emitterGetter();
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

// static
void TutorialStepNext::registerEmitter(const QString& group,
                                       const QString& emitter,
                                       std::function<QObject*()>&& getter) {
  Q_ASSERT(group != "qml_emitter");

  if (!s_emitters.contains(group)) {
    s_emitters.insert(group, QMap<QString, std::function<QObject*()>>());
  }

  Q_ASSERT(!s_emitters[group].contains(emitter));
  s_emitters[group].insert(emitter, std::move(getter));
}
