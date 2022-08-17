/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "navigator.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/feature.h"
#include "mozillavpn.h"
#include "qmlengineholder.h"

#include <QCoreApplication>

namespace {
Navigator* s_instance = nullptr;

struct ScreenData {
  Navigator::Screen m_screen;
  QString m_qmlComponentUrl;
  QVector<MozillaVPN::State> m_requiredState;
  int8_t (*m_priorityGetter)(Navigator::Screen* requiredScreen);
  QQmlComponent* m_qmlComponent = nullptr;
} s_screens[] = {
    {
        Navigator::Screen::ScreenInitialize,
        "qrc:/ui/screens/ScreenInitialize.qml",
        QVector<MozillaVPN::State>{MozillaVPN::StateInitialize},
        [](Navigator::Screen*) -> int8_t {
          return Feature::get(Feature::Feature_mobileOnboarding)->isSupported()
                     ? -1
                     : 0;
        },
    },
    {
        Navigator::Screen::ScreenInitialize,
        "qrc:/ui/screens/ScreenInitializeMobileOnboarding.qml",
        QVector<MozillaVPN::State>{MozillaVPN::StateInitialize},
        [](Navigator::Screen*) -> int8_t {
          return Feature::get(Feature::Feature_mobileOnboarding)->isSupported()
                     ? 0
                     : -1;
        },
    },
    {
        Navigator::Screen::ScreenAuthenticationInApp,
        "qrc:/ui/screens/ScreenAuthenticationInApp.qml",
        QVector<MozillaVPN::State>{MozillaVPN::StateAuthenticating},
        [](Navigator::Screen*) -> int8_t {
          return Feature::get(Feature::Feature_inAppAuthentication)
                         ->isSupported()
                     ? 0
                     : -1;
        },
    },
    {
        Navigator::Screen::ScreenAuthenticating,
        "qrc:/ui/screens/ScreenAuthenticating.qml",
        QVector<MozillaVPN::State>{MozillaVPN::StateAuthenticating},
        [](Navigator::Screen*) -> int8_t {
          return Feature::get(Feature::Feature_inAppAuthentication)
                         ->isSupported()
                     ? -1
                     : 0;
        },
    },
    {
        Navigator::Screen::ScreenPostAuthentication,
        "qrc:/ui/screens/ScreenPostAuthentication.qml",
        QVector<MozillaVPN::State>{MozillaVPN::StatePostAuthentication},
        [](Navigator::Screen*) -> int8_t { return 0; },
    },
    {
        Navigator::Screen::ScreenSubscriptionNeeded,
        "qrc:/ui/screens/ScreenSubscriptionNeeded.qml",
        QVector<MozillaVPN::State>{MozillaVPN::StateSubscriptionNeeded},
        [](Navigator::Screen*) -> int8_t { return 0; },
    },
    {
        Navigator::Screen::ScreenSubscriptionInProgress,
        "qrc:/ui/screens/ScreenSubscriptionInProgress.qml",
        QVector<MozillaVPN::State>{MozillaVPN::StateSubscriptionInProgress},
        [](Navigator::Screen*) -> int8_t { return 0; },
    },
    {
        Navigator::Screen::ScreenSubscriptionBlocked,
        "qrc:/ui/screens/ScreenSubscriptionBlocked.qml",
        QVector<MozillaVPN::State>{MozillaVPN::StateSubscriptionBlocked},
        [](Navigator::Screen*) -> int8_t { return 0; },
    },
    {
        Navigator::Screen::ScreenSubscriptionNotValidated,
        "qrc:/ui/screens/ScreenSubscriptionNotValidated.qml",
        QVector<MozillaVPN::State>{MozillaVPN::StateSubscriptionNotValidated},
        [](Navigator::Screen*) -> int8_t { return 0; },
    },
    {
        Navigator::Screen::ScreenTelemetryPolicy,
        "qrc:/ui/screens/ScreenTelemetryPolicy.qml",
        QVector<MozillaVPN::State>{MozillaVPN::StateTelemetryPolicy},
        [](Navigator::Screen*) -> int8_t { return 0; },
    },
    {
        Navigator::Screen::ScreenUpdateRequired,
        "qrc:/ui/screens/ScreenUpdateRequired.qml",
        QVector<MozillaVPN::State>{MozillaVPN::StateUpdateRequired},
        [](Navigator::Screen*) -> int8_t { return 0; },
    },
    {
        Navigator::Screen::ScreenHome,
        "qrc:/ui/screens/ScreenHome.qml",
        QVector<MozillaVPN::State>{MozillaVPN::StateMain},
        [](Navigator::Screen*) -> int8_t { return 99; },
    },
    {
        Navigator::Screen::ScreenMessaging,
        "qrc:/ui/screens/ScreenMessaging.qml",
        QVector<MozillaVPN::State>{MozillaVPN::StateMain},
        [](Navigator::Screen*) -> int8_t { return 0; },
    },
    {
        Navigator::Screen::ScreenSettings,
        "qrc:/ui/screens/ScreenSettings.qml",
        QVector<MozillaVPN::State>{MozillaVPN::StateMain},
        [](Navigator::Screen*) -> int8_t { return 0; },
    },
    {
        Navigator::Screen::ScreenContactUs,
        "qrc:/ui/screens/ScreenContactUs.qml",
        QVector<MozillaVPN::State>{},
        [](Navigator::Screen*) -> int8_t { return 0; },
    },
    {
        Navigator::Screen::ScreenGetHelp,
        "qrc:/ui/screens/ScreenGetHelp.qml",
        QVector<MozillaVPN::State>{},
        [](Navigator::Screen*) -> int8_t { return 0; },
    },
    {
        Navigator::Screen::ScreenViewLogs,
        "qrc:/ui/screens/ScreenViewLogs.qml",
        QVector<MozillaVPN::State>{},
        [](Navigator::Screen*) -> int8_t { return 0; },
    },
    {Navigator::Screen::ScreenSubscriptionGenericError,
     "qrc:/ui/screens/ScreenSubscriptionGenericError.qml",
     QVector<MozillaVPN::State>{MozillaVPN::StateSubscriptionNeeded,
                                MozillaVPN::StateSubscriptionInProgress},
     [](Navigator::Screen* requiredScreen) -> int8_t {
       return (requiredScreen &&
               *requiredScreen ==
                   Navigator::Screen::ScreenSubscriptionGenericError)
                  ? 99
                  : -1;
     }},
    {Navigator::Screen::ScreenNoSubscriptionFoundError,
     "qrc:/ui/screens/ScreenNoSubscriptionFoundError.qml",
     QVector<MozillaVPN::State>{MozillaVPN::StateSubscriptionNeeded,
                                MozillaVPN::StateSubscriptionInProgress},
     [](Navigator::Screen* requiredScreen) -> int8_t {
       return (requiredScreen &&
               *requiredScreen ==
                   Navigator::Screen::ScreenNoSubscriptionFoundError)
                  ? 99
                  : -1;
     }},
    {Navigator::Screen::ScreenSubscriptionExpiredError,
     "qrc:/ui/screens/ScreenSubscriptionExpiredError.qml",
     QVector<MozillaVPN::State>{MozillaVPN::StateSubscriptionNeeded,
                                MozillaVPN::StateSubscriptionInProgress},
     [](Navigator::Screen* requiredScreen) -> int8_t {
       return requiredScreen &&
                      *requiredScreen ==
                          Navigator::Screen::ScreenSubscriptionExpiredError
                  ? 99
                  : -1;
     }},
    {Navigator::Screen::ScreenSubscriptionInUseError,
     "qrc:/ui/screens/ScreenSubscriptionInUseError.qml",
     QVector<MozillaVPN::State>{MozillaVPN::StateSubscriptionNeeded,
                                MozillaVPN::StateSubscriptionInProgress},
     [](Navigator::Screen* requiredScreen) -> int8_t {
       return requiredScreen &&
                      *requiredScreen ==
                          Navigator::Screen::ScreenSubscriptionInUseError
                  ? 99
                  : -1;
     }},
};

bool computeScreen(const ScreenData& screen,
                   Navigator::Screen* requestedScreen) {
  if (screen.m_priorityGetter(requestedScreen) < 0) {
    return false;
  }

  if (!screen.m_requiredState.isEmpty() &&
      !screen.m_requiredState.contains(MozillaVPN::instance()->state())) {
    return false;
  }

  return true;
}

QList<ScreenData*> computeScreens(Navigator::Screen* requestedScreen) {
  QList<ScreenData*> screens;

  for (ScreenData& screen : s_screens) {
    if (computeScreen(screen, requestedScreen)) {
      screens.append(&screen);
    }
  }

  Q_ASSERT(!screens.isEmpty());
  return screens;
}

void maybeGenerateComponent(Navigator* navigator, ScreenData* screen) {
  if (!screen->m_qmlComponent) {
    QQmlComponent* qmlComponent = new QQmlComponent(
        QmlEngineHolder::instance()->engine(), screen->m_qmlComponentUrl,
        QQmlComponent::Asynchronous, navigator);

    Q_ASSERT(!qmlComponent->isError());
    screen->m_qmlComponent = qmlComponent;
  }
}

};  // namespace

// static
Navigator* Navigator::instance() {
  if (!s_instance) {
    s_instance = new Navigator(qApp);
  }
  return s_instance;
}

Navigator::Navigator(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(Navigator);

  connect(MozillaVPN::instance(), &MozillaVPN::stateChanged, this,
          &Navigator::computeComponent);

  connect(Feature::get(Feature::Feature_inAppAuthentication),
          &Feature::supportedChanged, this, &Navigator::computeComponent);

  connect(Feature::get(Feature::Feature_mobileOnboarding),
          &Feature::supportedChanged, this, &Navigator::computeComponent);

  connect(ErrorHandler::instance(), &ErrorHandler::subscriptionGeneric, this,
          [this]() { requestScreen(ScreenSubscriptionGenericError); });

  connect(ErrorHandler::instance(), &ErrorHandler::noSubscriptionFound, this,
          [this]() { requestScreen(ScreenNoSubscriptionFoundError); });

  connect(ErrorHandler::instance(), &ErrorHandler::subscriptionExpired, this,
          [this]() { requestScreen(ScreenSubscriptionExpiredError); });

  connect(ErrorHandler::instance(), &ErrorHandler::subscriptionInUse, this,
          [this]() { requestScreen(ScreenSubscriptionInUseError); });

  computeComponent();
}

Navigator::~Navigator() { MVPN_COUNT_DTOR(Navigator); }

void Navigator::computeComponent() {
  QList<ScreenData*> screens = computeScreens(nullptr);
  Q_ASSERT(!screens.isEmpty());

  // Maybe the current component is in the list of possible screen.
  if (m_currentComponent) {
    for (ScreenData* screen : screens) {
      if (screen->m_qmlComponent == m_currentComponent) {
        return;
      }
    }
  }

  ScreenData* topPriorityScreen = nullptr;
  for (ScreenData* screen : screens) {
    int8_t priority = screen->m_priorityGetter(nullptr);
    Q_ASSERT(priority >= 0);

    if (!topPriorityScreen ||
        topPriorityScreen->m_priorityGetter(nullptr) < priority) {
      topPriorityScreen = screen;
    }
  }
  Q_ASSERT(topPriorityScreen);

  if (m_currentComponent &&
      topPriorityScreen->m_qmlComponent == m_currentComponent) {
    return;
  }

  maybeGenerateComponent(this, topPriorityScreen);
  loadScreen(topPriorityScreen->m_screen, topPriorityScreen->m_qmlComponent);
}

void Navigator::requestScreen(Navigator::Screen requestedScreen) {
  QList<ScreenData*> screens = computeScreens(&requestedScreen);
  Q_ASSERT(!screens.isEmpty());

  for (ScreenData* screen : screens) {
    if (screen->m_screen == requestedScreen) {
      maybeGenerateComponent(this, screen);
      loadScreen(screen->m_screen, screen->m_qmlComponent);
      return;
    }
  }
}

void Navigator::requestPreviousScreen() {
  if (m_screenHistory.length() <= 1) {
    return;
  }

  m_screenHistory.takeLast();
  requestScreen(m_screenHistory.takeLast());
}

void Navigator::loadScreen(Screen screen, QQmlComponent* component) {
  m_screenHistory.append(screen);
  m_currentComponent = component;
  emit currentComponentChanged();
}
