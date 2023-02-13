/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tutorialstepbefore.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QMetaMethod>

#include "addons/addontutorial.h"
#include "controller.h"
#include "inspector/inspectorutils.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/serverdata.h"
#include "mozillavpn.h"

namespace {
Logger logger("TutorialStepBefore");
}

class TutorialStepBeforePropertyGet final : public TutorialStepBefore {
 public:
  static TutorialStepBefore* create(AddonTutorial* parent,
                                    const QJsonObject& obj) {
    QString query = obj["query"].toString();
    if (query.isEmpty()) {
      logger.warning() << "Empty 'query' for 'before' step property_get";
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

    return new TutorialStepBeforePropertyGet(parent, query, property,
                                             valueVariant);
  };

  TutorialStepBeforePropertyGet(AddonTutorial* parent, const QString& query,
                                const QString& property, const QVariant& value)
      : TutorialStepBefore(parent),
        m_query(query),
        m_property(property),
        m_value(value) {
    MZ_COUNT_CTOR(TutorialStepBeforePropertyGet);
  }

  ~TutorialStepBeforePropertyGet() {
    MZ_COUNT_DTOR(TutorialStepBeforePropertyGet);
  }

  bool run() override {
    QObject* element = InspectorUtils::queryObject(m_query);
    if (!element) {
      return false;
    }

    QVariant property = element->property(qPrintable(m_property));
    if (!property.isValid()) {
      logger.warning() << "Invalid property" << m_property << " for query"
                       << m_query;
      return false;
    }

    return property == m_value;
  }

 private:
  const QString m_query;
  const QString m_property;
  const QVariant m_value;
};

class TutorialStepBeforePropertySet final : public TutorialStepBefore {
 public:
  static TutorialStepBefore* create(AddonTutorial* parent,
                                    const QJsonObject& obj) {
    QString query = obj["query"].toString();
    if (query.isEmpty()) {
      logger.warning() << "Empty 'query' for 'before' step property_set";
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

    return new TutorialStepBeforePropertySet(parent, query, property,
                                             valueVariant);
  };

  TutorialStepBeforePropertySet(AddonTutorial* parent, const QString& query,
                                const QString& property, const QVariant& value)
      : TutorialStepBefore(parent),
        m_query(query),
        m_property(property),
        m_value(value) {
    MZ_COUNT_CTOR(TutorialStepBeforePropertySet);
  }

  ~TutorialStepBeforePropertySet() {
    MZ_COUNT_DTOR(TutorialStepBeforePropertySet);
  }

  bool run() override {
    QObject* element = InspectorUtils::queryObject(m_query);
    if (!element) {
      return false;
    }

    element->setProperty(qPrintable(m_property), m_value);
    return true;
  }

 private:
  const QString m_query;
  const QString m_property;
  const QVariant m_value;
};

class TutorialStepBeforeVpnLocationSet final : public TutorialStepBefore {
 public:
  static TutorialStepBefore* create(AddonTutorial* parent,
                                    const QJsonObject& obj) {
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

  TutorialStepBeforeVpnLocationSet(AddonTutorial* parent,
                                   const QString& exitCountryCode,
                                   const QString& exitCity,
                                   const QString& entryCountryCode,
                                   const QString& entryCity)
      : TutorialStepBefore(parent),
        m_exitCountryCode(exitCountryCode),
        m_exitCity(exitCity),
        m_entryCountryCode(entryCountryCode),
        m_entryCity(entryCity) {
    MZ_COUNT_CTOR(TutorialStepBeforeVpnLocationSet);
  }

  ~TutorialStepBeforeVpnLocationSet() {
    MZ_COUNT_DTOR(TutorialStepBeforeVpnLocationSet);
  }

  bool run() override {
    MozillaVPN::instance()->serverData()->changeServer(
        m_exitCountryCode, m_exitCity, m_entryCountryCode, m_entryCity);
    return true;
  }

 private:
  const QString m_exitCountryCode;
  const QString m_exitCity;
  const QString m_entryCountryCode;
  const QString m_entryCity;
};

class TutorialStepBeforeVpnOff final : public TutorialStepBefore {
 public:
  static TutorialStepBefore* create(AddonTutorial* parent, const QJsonObject&) {
    return new TutorialStepBeforeVpnOff(parent);
  };

  TutorialStepBeforeVpnOff(AddonTutorial* parent) : TutorialStepBefore(parent) {
    MZ_COUNT_CTOR(TutorialStepBeforeVpnOff);
  }

  ~TutorialStepBeforeVpnOff() { MZ_COUNT_DTOR(TutorialStepBeforeVpnOff); }

  bool run() override {
    Controller* controller = MozillaVPN::instance()->controller();
    Q_ASSERT(controller);

    if (controller->state() == Controller::StateOff) {
      return true;
    }

    controller->deactivate();
    return false;
  }
};

// static
bool TutorialStepBefore::create(AddonTutorial* parent, const QJsonValue& json,
                                QList<TutorialStepBefore*>& list) {
  QJsonArray array = json.toArray();
  for (const QJsonValue& value : array) {
    QJsonObject obj = value.toObject();

    TutorialStepBefore* tsb = nullptr;
    QString opValue = obj["op"].toString();
    if (opValue == "property_set") {
      tsb = TutorialStepBeforePropertySet::create(parent, obj);
    } else if (opValue == "property_get") {
      tsb = TutorialStepBeforePropertyGet::create(parent, obj);
    } else if (opValue == "vpn_location_set") {
      tsb = TutorialStepBeforeVpnLocationSet::create(parent, obj);
    } else if (opValue == "vpn_off") {
      tsb = TutorialStepBeforeVpnOff::create(parent, obj);
    } else {
      logger.warning() << "Invalid 'before' operation:" << opValue;
      return false;
    }

    if (!tsb) {
      logger.warning() << "Unable to create a step 'before' object";
      return false;
    }

    list.append(tsb);
  }

  return true;
}

TutorialStepBefore::TutorialStepBefore(AddonTutorial* parent)
    : QObject(parent), m_parent(parent) {
  MZ_COUNT_CTOR(TutorialStepBefore);
}

TutorialStepBefore::~TutorialStepBefore() { MZ_COUNT_DTOR(TutorialStepBefore); }
