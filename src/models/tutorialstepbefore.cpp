/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tutorialstepbefore.h"
#include "inspector/inspectorutils.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QMetaMethod>

namespace {
Logger logger(LOG_MAIN, "TutorialStepBefore");
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

class TutorialStepBeforeVpnLocationSet final : public TutorialStepBefore {
 public:
  static TutorialStepBefore* create(QObject* parent, const QJsonObject& obj) {
    QString exitCountryCode = obj["exitCountryCode"].toString();
    if (exitCountryCode.isEmpty()) {
      logger.warning()
          << "Empty exitCountryCode for 'before' step vpn_location_set";
      return nullptr;
    }

    QString exitCity = obj["exitCity"].toString();
    if (exitCity.isEmpty()) {
      logger.warning() << "Empty exitCity for 'before' step vpn_location_set";
      return nullptr;
    }

    QString entryCountryCode = obj["entryCountryCode"].toString();
    QString entryCity = obj["entryCity"].toString();

    return new TutorialStepBeforeVpnLocationSet(
        parent, exitCountryCode, exitCity, entryCountryCode, entryCity);
  };

  TutorialStepBeforeVpnLocationSet(QObject* parent,
                                   const QString& exitCountryCode,
                                   const QString& exitCity,
                                   const QString& entryCountryCode,
                                   const QString& entryCity)
      : TutorialStepBefore(parent),
        m_exitCountryCode(exitCountryCode),
        m_exitCity(exitCity),
        m_entryCountryCode(entryCountryCode),
        m_entryCity(entryCity) {
    MVPN_COUNT_CTOR(TutorialStepBeforeVpnLocationSet);
  }

  ~TutorialStepBeforeVpnLocationSet() {
    MVPN_COUNT_DTOR(TutorialStepBeforeVpnLocationSet);
  }

  bool run() override {
    Controller* controller = MozillaVPN::instance()->controller();
    Q_ASSERT(controller);

    controller->changeServer(m_exitCountryCode, m_exitCity, m_entryCountryCode,
                             m_entryCity);
    return true;
  }

 private:
  const QString m_exitCountryCode;
  const QString m_exitCity;
  const QString m_entryCountryCode;
  const QString m_entryCity;
};

// static
QList<TutorialStepBefore*> TutorialStepBefore::create(
    QObject* parent, const QString& elementForTooltip, const QJsonValue& json) {
  QList<TutorialStepBefore*> list;

  for (QJsonValue value : json.toArray()) {
    QJsonObject obj = value.toObject();

    TutorialStepBefore* tsb = nullptr;
    QString opValue = obj["op"].toString();
    if (opValue == "property_set") {
      tsb = TutorialStepBeforePropertySet::create(parent, obj);
    } else if (opValue == "property_get") {
      tsb = TutorialStepBeforePropertyGet::create(parent, obj);
    } else if (opValue == "vpn_location_set") {
      tsb = TutorialStepBeforeVpnLocationSet::create(parent, obj);
    } else {
      logger.warning() << "Invalid 'before' operation:" << opValue;
      return QList<TutorialStepBefore*>();
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
