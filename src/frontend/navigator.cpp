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
  QString (*m_qmlComponentGetter)();
  QVector<MozillaVPN::State> m_requiredState;
  QVector<MozillaVPN::State> m_excludedState;
  int8_t (*m_priorityGetter)();
  QQmlComponent* m_qmlComponent = nullptr;
} s_screens[] = {
    {
        Navigator::Screen::ScreenInitialize,
        []() -> QString { return "qrc:/ui/screens/ScreenInitialize.qml"; },
        QVector<MozillaVPN::State>{MozillaVPN::StateInitialize},
        QVector<MozillaVPN::State>{},
        []() -> int8_t { return 0; },
    },
    {
        Navigator::Screen::ScreenAuthenticationInApp,
        []() -> QString {
          return "qrc:/ui/screens/ScreenAuthenticationInApp.qml";
        },
        QVector<MozillaVPN::State>{MozillaVPN::StateAuthenticating},
        QVector<MozillaVPN::State>{},
        []() -> int8_t {
          return Feature::get(Feature::Feature_inAppAuthentication)
                         ->isSupported()
                     ? 0
                     : -1;
        },
    },
    {
        Navigator::Screen::ScreenAuthenticating,
        []() -> QString { return "qrc:/ui/screens/ScreenAuthenticating.qml"; },
        QVector<MozillaVPN::State>{MozillaVPN::StateAuthenticating},
        QVector<MozillaVPN::State>{},
        []() -> int8_t {
          return Feature::get(Feature::Feature_inAppAuthentication)
                         ->isSupported()
                     ? -1
                     : 0;
        },
    },
    {
        Navigator::Screen::ScreenPostAuthentication,
        []() -> QString {
          return "qrc:/ui/screens/ScreenPostAuthentication.qml";
        },
        QVector<MozillaVPN::State>{MozillaVPN::StatePostAuthentication},
        QVector<MozillaVPN::State>{},
        []() -> int8_t { return 0; },
    },
    {
        Navigator::Screen::ScreenSubscriptionNeeded,
        []() -> QString {
          return "qrc:/ui/screens/ScreenSubscriptionNeeded.qml";
        },
        QVector<MozillaVPN::State>{MozillaVPN::StateSubscriptionNeeded},
        QVector<MozillaVPN::State>{},
        []() -> int8_t { return 0; },
    },
    {
        Navigator::Screen::ScreenSubscriptionInProgress,
        []() -> QString {
          return "qrc:/ui/screens/ScreenSubscriptionInProgress.qml";
        },
        QVector<MozillaVPN::State>{MozillaVPN::StateSubscriptionInProgress},
        QVector<MozillaVPN::State>{},
        []() -> int8_t { return 0; },
    },
    {
        Navigator::Screen::ScreenSubscriptionBlocked,
        []() -> QString {
          return "qrc:/ui/screens/ScreenSubscriptionBlocked.qml";
        },
        QVector<MozillaVPN::State>{MozillaVPN::StateSubscriptionBlocked},
        QVector<MozillaVPN::State>{},
        []() -> int8_t { return 0; },
    },
    {
        Navigator::Screen::ScreenSubscriptionNotValidated,
        []() -> QString {
          return "qrc:/ui/screens/ScreenSubscriptionNotValidated.qml";
        },
        QVector<MozillaVPN::State>{MozillaVPN::StateSubscriptionNotValidated},
        QVector<MozillaVPN::State>{},
        []() -> int8_t { return 0; },
    },
    {
        Navigator::Screen::ScreenTelemetryPolicy,
        []() -> QString { return "qrc:/ui/screens/ScreenTelemetryPolicy.qml"; },
        QVector<MozillaVPN::State>{MozillaVPN::StateTelemetryPolicy},
        QVector<MozillaVPN::State>{},
        []() -> int8_t { return 0; },
    },
    {
        Navigator::Screen::ScreenUpdateRequired,
        []() -> QString { return "qrc:/ui/screens/ScreenUpdateRequired.qml"; },
        QVector<MozillaVPN::State>{MozillaVPN::StateUpdateRequired},
        QVector<MozillaVPN::State>{},
        []() -> int8_t { return 0; },
    },
    {
        Navigator::Screen::ScreenHome,
        []() -> QString { return "qrc:/ui/screens/ScreenHome.qml"; },
        QVector<MozillaVPN::State>{MozillaVPN::StateMain},
        QVector<MozillaVPN::State>{},
        []() -> int8_t { return 99; },
    },
    {
        Navigator::Screen::ScreenMessaging,
        []() -> QString { return "qrc:/ui/screens/ScreenMessaging.qml"; },
        QVector<MozillaVPN::State>{MozillaVPN::StateMain},
        QVector<MozillaVPN::State>{},
        []() -> int8_t { return 0; },
    },
    {
        Navigator::Screen::ScreenSettings,
        []() -> QString { return "qrc:/ui/screens/ScreenSettings.qml"; },
        QVector<MozillaVPN::State>{MozillaVPN::StateMain},
        QVector<MozillaVPN::State>{},
        []() -> int8_t { return 0; },
    },
    {
        Navigator::Screen::ScreenContactUs,
        []() -> QString { return "qrc:/ui/screens/ScreenContactUs.qml"; },
        QVector<MozillaVPN::State>{},
        QVector<MozillaVPN::State>{},
        []() -> int8_t { return 0; },
    },
    {
        Navigator::Screen::ScreenGetHelp,
        []() -> QString { return "qrc:/ui/screens/ScreenGetHelp.qml"; },
        QVector<MozillaVPN::State>{},
        QVector<MozillaVPN::State>{},
        []() -> int8_t { return 0; },
    },
    {
        Navigator::Screen::ScreenViewLogs,
        []() -> QString { return "qrc:/ui/screens/ScreenViewLogs.qml"; },
        QVector<MozillaVPN::State>{},
        QVector<MozillaVPN::State>{},
        []() -> int8_t { return 0; },
    },
};

bool computeScreen(const ScreenData& screen) {
  if (!screen.m_requiredState.isEmpty() &&
      !screen.m_requiredState.contains(MozillaVPN::instance()->state())) {
    return false;
  }

  if (!screen.m_excludedState.isEmpty() &&
      screen.m_excludedState.contains(MozillaVPN::instance()->state())) {
    return false;
  }

  return true;
}

QList<ScreenData*> computeScreens() {
  QList<ScreenData*> screens;

  for (ScreenData& screen : s_screens) {
    if (computeScreen(screen)) {
      screens.append(&screen);
    }
  }
  Q_ASSERT(!screens.isEmpty());
  return screens;
}

void maybeGenerateComponent(Navigator* navigator, ScreenData* screen) {
  if (!screen->m_qmlComponent) {
    QQmlComponent* qmlComponent = new QQmlComponent(
        QmlEngineHolder::instance()->engine(), screen->m_qmlComponentGetter(),
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

  computeComponent();
}

Navigator::~Navigator() { MVPN_COUNT_DTOR(Navigator); }

void Navigator::computeComponent() {
  QList<ScreenData*> screens = computeScreens();
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
    int8_t priority = screen->m_priorityGetter();
    if (priority < 0) continue;

    if (!topPriorityScreen ||
        topPriorityScreen->m_priorityGetter() < priority) {
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
  QList<ScreenData*> screens = computeScreens();
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
