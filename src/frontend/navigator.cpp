/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "navigator.h"
#include "externalophandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/feature.h"
#include "mozillavpn.h"
#include "qmlengineholder.h"

#include <QCoreApplication>
#include <QQuickItem>

namespace {
Navigator* s_instance = nullptr;
Logger logger(LOG_MAIN, "Navigator");

struct Layer {
  enum Type {
    eStackView,
    eView,
  };

  Layer(QQuickItem* layer, Type type) : m_layer(layer), m_type(type) {}

  QQuickItem* m_layer;
  Type m_type;
};

// This struct contains the data for a single screen.
struct ScreenData {
  // The screen name.
  Navigator::Screen m_screen;

  // The URL of the component.
  QString m_qmlComponentUrl;

  // The list of acceptable VPN states. Empty means any state is OK.
  QVector<MozillaVPN::State> m_requiredState;

  // This function ptr is used to obtain the priority for this screen. If it
  // returns -1, the screen is removed from the computation. Otherwise, if we
  // do not need to show a particular screen (see requestScreen() method), the
  // top priority screen is shown.
  int8_t (*m_priorityGetter)(Navigator::Screen* requestedScreen) = nullptr;

  // On android/wasm, when the user clicks the back-button, the app uses this
  // function to decide the screen policy. If the function returns false, the
  // app will quit.
  bool (*m_quitBlocked)() = nullptr;

  // The cache of the QML component.
  QQmlComponent* m_qmlComponent = nullptr;

  // List of stack views, or views registered by this screen.
  QList<Layer> m_layers;

  ScreenData(Navigator::Screen screen, const QString& qmlComponentUrl,
             const QVector<MozillaVPN::State>& requiredState,
             int8_t (*priorityGetter)(Navigator::Screen* requestedScreen),
             bool (*quitBlocked)())
      : m_screen(screen),
        m_qmlComponentUrl(qmlComponentUrl),
        m_requiredState(requiredState),
        m_priorityGetter(priorityGetter),
        m_quitBlocked(quitBlocked) {}
};

// The list of screens.
ScreenData s_screens[] = {
    ScreenData(
        Navigator::Screen::ScreenInitialize,
        "qrc:/ui/screens/ScreenInitialize.qml",
        QVector<MozillaVPN::State>{MozillaVPN::StateInitialize},
        [](Navigator::Screen*) -> int8_t {
          return Feature::get(Feature::Feature_mobileOnboarding)->isSupported()
                     ? -1
                     : 0;
        },
        []() -> bool { return false; }),
    ScreenData(
        Navigator::Screen::ScreenInitialize,
        "qrc:/ui/screens/ScreenInitializeMobileOnboarding.qml",
        QVector<MozillaVPN::State>{MozillaVPN::StateInitialize},
        [](Navigator::Screen*) -> int8_t {
          return Feature::get(Feature::Feature_mobileOnboarding)->isSupported()
                     ? 0
                     : -1;
        },
        []() -> bool { return false; }),
    ScreenData(
        Navigator::Screen::ScreenAuthenticationInApp,
        "qrc:/ui/screens/ScreenAuthenticationInApp.qml",
        QVector<MozillaVPN::State>{MozillaVPN::StateAuthenticating},
        [](Navigator::Screen*) -> int8_t {
          return Feature::get(Feature::Feature_inAppAuthentication)
                         ->isSupported()
                     ? 0
                     : -1;
        },
        []() -> bool {
          MozillaVPN::instance()->cancelAuthentication();
          return true;
        }),
    ScreenData(
        Navigator::Screen::ScreenAuthenticating,
        "qrc:/ui/screens/ScreenAuthenticating.qml",
        QVector<MozillaVPN::State>{MozillaVPN::StateAuthenticating},
        [](Navigator::Screen*) -> int8_t {
          return Feature::get(Feature::Feature_inAppAuthentication)
                         ->isSupported()
                     ? -1
                     : 0;
        },
        []() -> bool {
          MozillaVPN::instance()->cancelAuthentication();
          return true;
        }),
    ScreenData(
        Navigator::Screen::ScreenBackendFailure,
        "qrc:/ui/screens/ScreenBackendFailure.qml",
        QVector<MozillaVPN::State>{MozillaVPN::StateBackendFailure},
        [](Navigator::Screen*) -> int8_t { return 0; },
        []() -> bool { return false; }),
    ScreenData(
        Navigator::Screen::ScreenBillingNotAvailable,
        "qrc:/ui/screens/ScreenBillingNotAvailable.qml",
        QVector<MozillaVPN::State>{MozillaVPN::StateBillingNotAvailable},
        [](Navigator::Screen*) -> int8_t { return 0; },
        []() -> bool { return false; }),
    ScreenData(
        Navigator::Screen::ScreenDeviceLimit,
        "qrc:/ui/screens/ScreenDeviceLimit.qml",
        QVector<MozillaVPN::State>{MozillaVPN::StateDeviceLimit},
        [](Navigator::Screen*) -> int8_t { return 0; },
        []() -> bool { return false; }),
    ScreenData(
        Navigator::Screen::ScreenPostAuthentication,
        "qrc:/ui/screens/ScreenPostAuthentication.qml",
        QVector<MozillaVPN::State>{MozillaVPN::StatePostAuthentication},
        [](Navigator::Screen*) -> int8_t { return 0; },
        []() -> bool { return false; }),
    ScreenData(
        Navigator::Screen::ScreenSubscriptionNeeded,
        "qrc:/ui/screens/ScreenSubscriptionNeeded.qml",
        QVector<MozillaVPN::State>{MozillaVPN::StateSubscriptionNeeded},
        [](Navigator::Screen*) -> int8_t { return 0; },
        []() -> bool { return false; }),
    ScreenData(
        Navigator::Screen::ScreenSubscriptionInProgress,
        "qrc:/ui/screens/ScreenSubscriptionInProgress.qml",
        QVector<MozillaVPN::State>{MozillaVPN::StateSubscriptionInProgress},
        [](Navigator::Screen*) -> int8_t { return 0; },
        []() -> bool { return false; }),
    ScreenData(
        Navigator::Screen::ScreenSubscriptionBlocked,
        "qrc:/ui/screens/ScreenSubscriptionBlocked.qml",
        QVector<MozillaVPN::State>{MozillaVPN::StateSubscriptionBlocked},
        [](Navigator::Screen*) -> int8_t { return 0; },
        []() -> bool { return false; }),
    ScreenData(
        Navigator::Screen::ScreenSubscriptionNotValidated,
        "qrc:/ui/screens/ScreenSubscriptionNotValidated.qml",
        QVector<MozillaVPN::State>{MozillaVPN::StateSubscriptionNotValidated},
        [](Navigator::Screen*) -> int8_t { return 0; },
        []() -> bool { return false; }),
    ScreenData(
        Navigator::Screen::ScreenTelemetryPolicy,
        "qrc:/ui/screens/ScreenTelemetryPolicy.qml",
        QVector<MozillaVPN::State>{MozillaVPN::StateTelemetryPolicy},
        [](Navigator::Screen*) -> int8_t { return 0; },
        []() -> bool { return false; }),
    ScreenData(
        Navigator::Screen::ScreenUpdateRequired,
        "qrc:/ui/screens/ScreenUpdateRequired.qml",
        QVector<MozillaVPN::State>{MozillaVPN::StateUpdateRequired},
        [](Navigator::Screen*) -> int8_t { return 0; },
        []() -> bool { return false; }),
    ScreenData(
        Navigator::Screen::ScreenHome, "qrc:/ui/screens/ScreenHome.qml",
        QVector<MozillaVPN::State>{MozillaVPN::StateMain},
        [](Navigator::Screen*) -> int8_t { return 99; },
        []() -> bool { return false; }),
    ScreenData(
        Navigator::Screen::ScreenMessaging,
        "qrc:/ui/screens/ScreenMessaging.qml",
        QVector<MozillaVPN::State>{MozillaVPN::StateMain},
        [](Navigator::Screen*) -> int8_t { return 0; },
        []() -> bool {
          Navigator::instance()->requestScreen(Navigator::ScreenHome);
          // TODO: reset the stack
          return true;
        }),
    ScreenData(
        Navigator::Screen::ScreenSettings, "qrc:/ui/screens/ScreenSettings.qml",
        QVector<MozillaVPN::State>{MozillaVPN::StateMain},
        [](Navigator::Screen*) -> int8_t { return 0; },
        []() -> bool {
          Navigator::instance()->requestScreen(Navigator::ScreenHome);
          // TODO: reset the stack
          return true;
        }),
    ScreenData(
        Navigator::Screen::ScreenGetHelp, "qrc:/ui/screens/ScreenGetHelp.qml",
        QVector<MozillaVPN::State>{},
        [](Navigator::Screen*) -> int8_t { return 0; },
        []() -> bool {
          Navigator::instance()->requestPreviousScreen();
          return true;
        }),
    ScreenData(
        Navigator::Screen::ScreenViewLogs, "qrc:/ui/screens/ScreenViewLogs.qml",
        QVector<MozillaVPN::State>{},
        [](Navigator::Screen*) -> int8_t { return 0; },
        []() -> bool {
          Navigator::instance()->requestPreviousScreen();
          return true;
        }),
    ScreenData(
        Navigator::Screen::ScreenSubscriptionGenericError,
        "qrc:/ui/screens/ScreenSubscriptionGenericError.qml",
        QVector<MozillaVPN::State>{MozillaVPN::StateSubscriptionNeeded,
                                   MozillaVPN::StateSubscriptionInProgress},
        [](Navigator::Screen* requestedScreen) -> int8_t {
          return (requestedScreen &&
                  *requestedScreen ==
                      Navigator::Screen::ScreenSubscriptionGenericError)
                     ? 99
                     : -1;
        },
        []() -> bool { return false; }),
    ScreenData(
        Navigator::Screen::ScreenNoSubscriptionFoundError,
        "qrc:/ui/screens/ScreenNoSubscriptionFoundError.qml",
        QVector<MozillaVPN::State>{MozillaVPN::StateSubscriptionNeeded,
                                   MozillaVPN::StateSubscriptionInProgress},
        [](Navigator::Screen* requestedScreen) -> int8_t {
          return (requestedScreen &&
                  *requestedScreen ==
                      Navigator::Screen::ScreenNoSubscriptionFoundError)
                     ? 99
                     : -1;
        },
        []() -> bool { return false; }),
    ScreenData(
        Navigator::Screen::ScreenSubscriptionExpiredError,
        "qrc:/ui/screens/ScreenSubscriptionExpiredError.qml",
        QVector<MozillaVPN::State>{MozillaVPN::StateSubscriptionNeeded,
                                   MozillaVPN::StateSubscriptionInProgress},
        [](Navigator::Screen* requestedScreen) -> int8_t {
          return requestedScreen &&
                         *requestedScreen ==
                             Navigator::Screen::ScreenSubscriptionExpiredError
                     ? 99
                     : -1;
        },
        []() -> bool { return false; }),
    ScreenData(
        Navigator::Screen::ScreenSubscriptionInUseError,
        "qrc:/ui/screens/ScreenSubscriptionInUseError.qml",
        QVector<MozillaVPN::State>{MozillaVPN::StateSubscriptionNeeded,
                                   MozillaVPN::StateSubscriptionInProgress},
        [](Navigator::Screen* requestedScreen) -> int8_t {
          return requestedScreen &&
                         *requestedScreen ==
                             Navigator::Screen::ScreenSubscriptionInUseError
                     ? 99
                     : -1;
        },
        []() -> bool { return false; }),
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
  logger.debug() << "Compute component";

  QList<ScreenData*> screens = computeScreens(nullptr);
  Q_ASSERT(!screens.isEmpty());

  // Maybe the current component is in the list of possible screen.
  if (m_currentComponent) {
    for (ScreenData* screen : screens) {
      if (screen->m_qmlComponent == m_currentComponent) {
        logger.debug() << "The current screen is still acceptable";
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

  maybeGenerateComponent(this, topPriorityScreen);
  loadScreen(topPriorityScreen->m_screen, topPriorityScreen->m_qmlComponent);
}

void Navigator::requestScreen(Navigator::Screen requestedScreen) {
  logger.debug() << "Screen request:" << requestedScreen;

  QList<ScreenData*> screens = computeScreens(&requestedScreen);
  Q_ASSERT(!screens.isEmpty());

  for (ScreenData* screen : screens) {
    if (screen->m_screen == requestedScreen) {
      maybeGenerateComponent(this, screen);
      loadScreen(screen->m_screen, screen->m_qmlComponent);
      return;
    }
  }

  logger.debug() << "Unable to show the requested screen";
}

void Navigator::requestPreviousScreen() {
  logger.debug() << "Previous screen request";

  if (m_screenHistory.length() <= 1) {
    logger.error() << "not enough screens!";
    return;
  }

  m_screenHistory.takeLast();
  requestScreen(m_screenHistory.takeLast());
}

void Navigator::loadScreen(Screen screen, QQmlComponent* component) {
  logger.debug() << "Loading screen" << screen;

  m_screenHistory.append(screen);
  m_currentComponent = component;
  emit currentComponentChanged();
}

void Navigator::addStackView(Screen requestedScreen,
                             const QVariant& stackView) {
  logger.debug() << "Add stack view for screen" << requestedScreen;

  QQuickItem* item = qobject_cast<QQuickItem*>(stackView.value<QObject*>());
  Q_ASSERT(item);

  QList<ScreenData*> screens = computeScreens(&requestedScreen);
  Q_ASSERT(!screens.isEmpty());

  for (ScreenData* screen : screens) {
    if (screen->m_screen == requestedScreen) {
      connect(item, &QObject::destroyed, this, &Navigator::removeItem);
      screen->m_layers.append(Layer(item, Layer::eStackView));
      return;
    }
  }

  logger.error() << "Unable to find the screen" << requestedScreen;
}

void Navigator::addView(Screen requestedScreen, const QVariant& view) {
  logger.debug() << "Add view for screen" << requestedScreen;

  QQuickItem* item = qobject_cast<QQuickItem*>(view.value<QObject*>());
  Q_ASSERT(item);

  QList<ScreenData*> screens = computeScreens(&requestedScreen);
  Q_ASSERT(!screens.isEmpty());

  for (ScreenData* screen : screens) {
    if (screen->m_screen == requestedScreen) {
      connect(item, &QObject::destroyed, this, &Navigator::removeItem);
      screen->m_layers.append(Layer(item, Layer::eView));
      return;
    }
  }

  logger.error() << "Unable to find the screen" << requestedScreen;
}

void Navigator::removeItem(QObject* item) {
  logger.debug() << "Remove item";
  Q_ASSERT(item);

#ifdef MVPN_DEBUG
  bool found = false;
#endif

  for (ScreenData& screen : s_screens) {
    for (int i = 0; i < screen.m_layers.length(); ++i) {
      if (screen.m_layers.at(i).m_layer == item) {
        screen.m_layers.removeAt(i);
#ifdef MVPN_DEBUG
        found = true;
#endif
        break;
      }
    }
  }

#ifdef MVPN_DEBUG
  Q_ASSERT(found);
#endif
}

bool Navigator::eventHandled() {
  logger.debug() << "Close event handled";

  ExternalOpHandler::instance()->request(ExternalOpHandler::OpCloseEvent);

#if defined(MVPN_ANDROID) || defined(MVPN_WASM)
  if (m_screenHistory.isEmpty()) {
    return false;
  }

  ScreenData* currentScreen = nullptr;
  for (ScreenData& screen : s_screens) {
    if (screen.m_screen == m_screenHistory.last()) {
      currentScreen = &screen;
      break;
    }
  }
  Q_ASSERT(currentScreen);

  for (int i = currentScreen->m_layers.length() - 1; i >= 0; --i) {
    const Layer& layer = currentScreen->m_layers.at(i);

    if (layer.m_type == Layer::eStackView) {
      QVariant property = layer.m_layer->property("depth");
      if (!property.isValid()) {
        logger.warning() << "Invalid depth property!!";
        continue;
      }

      int depth = property.toInt();
      if (depth > 1) {
        emit goBack(layer.m_layer);
        return true;
      }

      continue;
    }

    Q_ASSERT(layer.m_type == Layer::eView);
    QVariant property = layer.m_layer->property("visible");
    if (!property.isValid()) {
      logger.warning() << "Invalid visible property!!";
      continue;
    }

    bool visible = property.toBool();
    if (visible) {
      emit goBack(layer.m_layer);
      return true;
    }

    continue;
  }

  return currentScreen->m_quitBlocked();
#elif defined(MVPN_IOS)
  return false;
#elif defined(MVPN_LINUX) || defined(MVPN_MACOS) || defined(MVPN_WINDOWS) || \
    defined(MVPN_DUMMY)
  logger.error() << "We should not be here! Why "
                    "CloseEventHandler::eventHandled() is called on desktop?!?";
  return true;
#else
#  error Unsupported platform
#endif
}
