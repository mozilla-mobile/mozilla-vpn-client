/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tutorialvpn.h"

#include <QJsonObject>

#include "controller.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/serverdata.h"
#include "mozillavpn.h"
#include "settingsholder.h"
#include "tutorial/tutorialstepbefore.h"
#include "tutorial/tutorialstepnext.h"

namespace {

Logger logger("TutorialVPN");

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
    MozillaVPN::instance()->serverData()->changeServerForTutorial(
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

};  // namespace

// static
void TutorialVPN::registerTutorialSteps() {
  TutorialStepBefore::registerTutorialStepBefore(
      "vpn_location_set", TutorialStepBeforeVpnLocationSet::create);
  TutorialStepBefore::registerTutorialStepBefore(
      "vpn_off", TutorialStepBeforeVpnOff::create);

  TutorialStepNext::registerEmitter(
      "vpn_emitter",
      [](const QString& objectName) -> bool {
        return objectName == "controller" || objectName == "settingsHolder";
      },
      [](const QString& objectName) -> QObject* {
        if (objectName == "settingsHolder") {
          return SettingsHolder::instance();
        }

        if (objectName == "controller") {
          return MozillaVPN::instance()->controller();
        }

        qFatal("Invalid objectName");
        return nullptr;
      });
}
