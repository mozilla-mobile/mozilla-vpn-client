/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tutorialstepbefore.h"
#include "inspector/inspectorutils.h"
#include "leakdetector.h"
#include "logger.h"
#include "moduleholder.h"
#include "mozillavpn.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QMetaMethod>

namespace {
Logger logger("TutorialStepBefore");
}

class TutorialStepBeforePropertyGet final : public TutorialStepBefore {
 public:
  static TutorialStepBefore* create(QObject* parent, const QJsonObject& obj) {
    QString element = obj["element"].toString();
    if (element.isEmpty()) {
      logger.warning() << "Empty 'element' for 'before' step property_get";
      return nullptr;
    }

    QString property = obj["property"].toString();
    if (property.isEmpty()) {
      logger.warning() << "Empty 'property' for 'before' step property_get";
      return nullptr;
    }

    QJsonValue value = obj["value"];
    if (value.type() != QJsonValue::Bool &&
        value.type() != QJsonValue::String &&
        value.type() != QJsonValue::Double) {
      logger.warning() << "Only bool, string and numbers are supported for the "
                          "'before' step property_get";
      return nullptr;
    }

    QVariant valueVariant = value.toVariant();
    if (!valueVariant.isValid()) {
      logger.warning() << "Invalid value for 'before' step property_get";
      return nullptr;
    }

    return new TutorialStepBeforePropertyGet(parent, element, property,
                                             valueVariant);
  };

  TutorialStepBeforePropertyGet(QObject* parent, const QString& element,
                                const QString& property, const QVariant& value)
      : TutorialStepBefore(parent),
        m_element(element),
        m_property(property),
        m_value(value) {
    MVPN_COUNT_CTOR(TutorialStepBeforePropertyGet);
  }

  ~TutorialStepBeforePropertyGet() {
    MVPN_COUNT_DTOR(TutorialStepBeforePropertyGet);
  }

  bool run() override {
    QObject* element = InspectorUtils::findObject(m_element);
    if (!element) {
      return false;
    }

    QVariant property = element->property(qPrintable(m_property));
    if (!property.isValid()) {
      logger.warning() << "Invalid property" << m_property << " for element"
                       << m_element;
      return false;
    }

    return property == m_value;
  }

 private:
  const QString m_element;
  const QString m_property;
  const QVariant m_value;
};

class TutorialStepBeforePropertySet final : public TutorialStepBefore {
 public:
  static TutorialStepBefore* create(QObject* parent, const QJsonObject& obj) {
    QString element = obj["element"].toString();
    if (element.isEmpty()) {
      logger.warning() << "Empty 'element' for 'before' step property_set";
      return nullptr;
    }

    QString property = obj["property"].toString();
    if (property.isEmpty()) {
      logger.warning() << "Empty 'property' for 'before' step property_set";
      return nullptr;
    }

    QJsonValue value = obj["value"];
    if (value.type() != QJsonValue::Bool &&
        value.type() != QJsonValue::String &&
        value.type() != QJsonValue::Double) {
      logger.warning() << "Only bool, string and numbers are supported for the "
                          "'before' step property_set";
      return nullptr;
    }

    QVariant valueVariant = value.toVariant();
    if (!valueVariant.isValid()) {
      logger.warning() << "Invalid value for 'before' step property_set";
      return nullptr;
    }

    return new TutorialStepBeforePropertySet(parent, element, property,
                                             valueVariant);
  };

  TutorialStepBeforePropertySet(QObject* parent, const QString& element,
                                const QString& property, const QVariant& value)
      : TutorialStepBefore(parent),
        m_element(element),
        m_property(property),
        m_value(value) {
    MVPN_COUNT_CTOR(TutorialStepBeforePropertySet);
  }

  ~TutorialStepBeforePropertySet() {
    MVPN_COUNT_DTOR(TutorialStepBeforePropertySet);
  }

  bool run() override {
    QObject* element = InspectorUtils::findObject(m_element);
    if (!element) {
      return false;
    }

    element->setProperty(qPrintable(m_property), m_value);
    return true;
  }

 private:
  const QString m_element;
  const QString m_property;
  const QVariant m_value;
};

// static
QList<TutorialStepBefore*> TutorialStepBefore::create(
    QObject* parent, const QString& elementForTooltip, const QJsonValue& json) {
  QList<TutorialStepBefore*> list;

  QJsonArray array = json.toArray();
  for (const QJsonValue& value : array) {
    QJsonObject obj = value.toObject();

    TutorialStepBefore* tsb = nullptr;
    QString opValue = obj["op"].toString();
    if (opValue == "property_set") {
      tsb = TutorialStepBeforePropertySet::create(parent, obj);
    } else if (opValue == "property_get") {
      tsb = TutorialStepBeforePropertyGet::create(parent, obj);
    } else {
      ModuleHolder::instance()->forEach(
          [tsb = &tsb, parent, opValue, obj](const QString&, Module* module) {
            Q_ASSERT(tsb && !*tsb);
            *tsb = module->maybeCreateTutorialStepBefore(parent, opValue, obj);
          });

      if (!tsb) {
        logger.warning() << "Invalid 'before' operation:" << opValue;
        return QList<TutorialStepBefore*>();
      }
    }

    if (!tsb) {
      logger.warning() << "Unable to create a step 'before' object";
      return QList<TutorialStepBefore*>();
    }

    list.append(tsb);
  }

  // The tooltip element must be visible.
  list.append(new TutorialStepBeforePropertyGet(parent, elementForTooltip,
                                                "visible", QVariant(true)));
  return list;
}

TutorialStepBefore::TutorialStepBefore(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(TutorialStepBefore);
}

TutorialStepBefore::~TutorialStepBefore() {
  MVPN_COUNT_DTOR(TutorialStepBefore);
}
