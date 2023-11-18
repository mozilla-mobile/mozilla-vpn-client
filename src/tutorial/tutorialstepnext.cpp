/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tutorialstepnext.h"

#include <QJsonObject>
#include <QJsonValue>
#include <QMetaMethod>

#include "addons/addontutorial.h"
#include "leakdetector.h"
#include "logger.h"

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

struct TutorialStepNextData {
  QString m_name;
  bool (*m_emitterValidator)(const QString&);
  QObject* (*m_emitter)(const QString&);
};

QList<TutorialStepNextData> s_tutorialStepNextList{TutorialStepNextData{
    "query_emitter", [](const QString&) -> bool { return true; },
    nullptr}};

}  // namespace

// static
TutorialStepNext* TutorialStepNext::create(AddonTutorial* parent,
                                           const QJsonValue& json) {
  QJsonObject obj = json.toObject();
  if (obj["op"] != "signal") {
    logger.warning() << "Only 'signal' tutorial next are supported";
    return nullptr;
  }

  QString emitterName;
  QObject* (*emitterCallback)(const QString& objectName) = nullptr;

  for (const TutorialStepNextData& data : s_tutorialStepNextList) {
    if (obj.contains(data.m_name)) {
      emitterName = obj[data.m_name].toString();
      emitterCallback = data.m_emitter;

      if (!data.m_emitterValidator(obj[data.m_name].toString())) {
        logger.warning() << "The emitter" << obj[data.m_name].toString()
                         << "is not valid";
        return nullptr;
      }

      break;
    }
  }

  if (!emitterCallback) {
    logger.warning() << "No emitter defined";
    return nullptr;
  }

  QString signal = obj["signal"].toString();
  if (signal.isEmpty()) {
    logger.warning() << "No signal property";
    return nullptr;
  }

  return new TutorialStepNext(parent, emitterName, emitterCallback, signal);
}

TutorialStepNext::TutorialStepNext(AddonTutorial* parent,
                                   const QString& emitterName,
                                   QObject* (*emitterCallback)(const QString&),
                                   const QString& signal)
    : QObject(parent),
      m_addonTutorial(parent),
      m_emitterName(emitterName),
      m_emitterCallback(emitterCallback),
      m_signal(signal) {
  MZ_COUNT_CTOR(TutorialStepNext);
}

TutorialStepNext::~TutorialStepNext() { MZ_COUNT_DTOR(TutorialStepNext); }

void TutorialStepNext::startOrStop(bool start) {
  QObject* obj = m_emitterCallback(m_emitterName);

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
void TutorialStepNext::registerEmitter(
    const QString& name, bool (*emitterValidator)(const QString& objectName),
    QObject* (*emitter)(const QString& objectName)) {
  s_tutorialStepNextList.append(
      TutorialStepNextData{name, emitterValidator, emitter});
}
