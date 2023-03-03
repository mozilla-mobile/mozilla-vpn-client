/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "navigator.h"

#include <QCoreApplication>
#include <QQuickItem>

#include "app.h"
#include "errorhandler.h"
#include "externalophandler.h"
#include "feature.h"
#include "glean/generated/metrics.h"
#include "gleandeprecated.h"
#include "leakdetector.h"
#include "logger.h"
#include "loglevel.h"
#include "qmlengineholder.h"
#include "telemetry/gleansample.h"

namespace {
Navigator* s_instance = nullptr;
Logger logger("Navigator");

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
  int m_screen;

  // Default load policy for this screen
  Navigator::LoadPolicy m_loadPolicy;

  // The URL of the component.
  QString m_qmlComponentUrl;

  // The list of acceptable App states. Empty means any state is OK.
  QVector<int> m_requiredState;

  // This function ptr is used to obtain the priority for this screen. If it
  // returns -1, the screen is removed from the computation. Otherwise, if we
  // do not need to show a particular screen (see requestScreen() method), the
  // top priority screen is shown.
  int8_t (*m_priorityGetter)(int* requestedScreen) = nullptr;

  // On android/wasm, when the user clicks the back-button, the app uses this
  // function to decide the screen policy. If the function returns false, the
  // app will quit.
  bool (*m_quitBlocked)() = nullptr;

  // The cache of the QML component.
  QQmlComponent* m_qmlComponent = nullptr;

  // List of stack views, or views registered by this screen.
  QList<Layer> m_layers;

  ScreenData(int screen, Navigator::LoadPolicy loadPolicy,
             const QString& qmlComponentUrl, const QVector<int>& requiredState,
             int8_t (*priorityGetter)(int* requestedScreen),
             bool (*quitBlocked)())
      : m_screen(screen),
        m_loadPolicy(loadPolicy),
        m_qmlComponentUrl(qmlComponentUrl),
        m_requiredState(requiredState),
        m_priorityGetter(priorityGetter),
        m_quitBlocked(quitBlocked) {}
};

// The list of screens.
QList<ScreenData> s_screens;

bool computeScreen(const ScreenData& screen, int* requestedScreen) {
  if (screen.m_priorityGetter(requestedScreen) < 0) {
    return false;
  }

  if (!screen.m_requiredState.isEmpty() &&
      !screen.m_requiredState.contains(App::instance()->state())) {
    return false;
  }

  return true;
}

QList<ScreenData*> computeScreens(int* requestedScreen) {
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
  MZ_COUNT_CTOR(Navigator);
}

Navigator::~Navigator() { MZ_COUNT_DTOR(Navigator); }

void Navigator::initialize() {
  connect(App::instance(), &App::stateChanged, this,
          &Navigator::computeComponent);

  connect(Feature::get(Feature::Feature_inAppAuthentication),
          &Feature::supportedChanged, this, &Navigator::computeComponent);

  computeComponent();
}

void Navigator::computeComponent() {
  logger.debug() << "Compute component";

  QList<ScreenData*> screens = computeScreens(nullptr);
  Q_ASSERT(!screens.isEmpty());

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
  loadScreen(topPriorityScreen->m_screen, topPriorityScreen->m_loadPolicy,
             topPriorityScreen->m_qmlComponent, ForceReloadAll);
}

void Navigator::requestScreenFromBottomBar(
    int requestedScreen, Navigator::LoadingFlags loadingFlags) {
  QString screenStr;

  if (requestedScreen <= ScreenCustom) {
    screenStr = QVariant::fromValue(requestedScreen).toString();
  } else {
    screenStr = QString::number(requestedScreen);
  }

  // Exists so we can add glean metric for screen changes only from bottom bar
  mozilla::glean::sample::bottom_navigation_bar_click.record(
      mozilla::glean::sample::BottomNavigationBarClickExtra{
          ._barButton = QVariant::fromValue(requestedScreen).toString()});
  emit GleanDeprecated::instance()->recordGleanEventWithExtraKeys(
      GleanSample::bottomNavigationBarClick,
      {{"bar_button", QVariant::fromValue(requestedScreen).toString()}});

  requestScreen(requestedScreen, loadingFlags);
}

void Navigator::requestScreen(int requestedScreen,
                              Navigator::LoadingFlags loadingFlags) {
  logger.debug() << "Screen request:" << requestedScreen;

  if (!m_reloaders.isEmpty() && loadingFlags == NoFlags) {
    loadingFlags = ForceReload;
  }

  QList<ScreenData*> screens = computeScreens(&requestedScreen);
  Q_ASSERT(!screens.isEmpty());

  for (ScreenData* screen : screens) {
    if (screen->m_screen == requestedScreen) {
      maybeGenerateComponent(this, screen);

      if (screen->m_qmlComponent == m_currentComponent &&
          loadingFlags == NoFlags) {
        logger.debug() << "Already in the right screen";
        return;
      }

      loadScreen(screen->m_screen, screen->m_loadPolicy, screen->m_qmlComponent,
                 loadingFlags);
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

void Navigator::loadScreen(int screen, LoadPolicy loadPolicy,
                           QQmlComponent* component,
                           LoadingFlags loadingFlags) {
  logger.debug() << "Loading screen" << screen;

  if (!m_reloaders.isEmpty() && loadingFlags == NoFlags) {
    loadingFlags = ForceReload;
  }

  if (m_screenHistory.isEmpty() || screen != m_currentScreen) {
    m_screenHistory.append(screen);
    m_currentScreen = screen;
  }

  m_currentLoadPolicy = loadPolicy;
  m_currentComponent = component;
  m_currentLoadingFlags = loadingFlags;

  emit currentComponentChanged();
}

void Navigator::addStackView(int requestedScreen, const QVariant& stackView) {
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

void Navigator::addView(int requestedScreen, const QVariant& view) {
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

#ifdef MZ_DEBUG
  bool found = false;
#endif

  for (ScreenData& screen : s_screens) {
    for (int i = 0; i < screen.m_layers.length(); ++i) {
      if (screen.m_layers.at(i).m_layer == item) {
        screen.m_layers.removeAt(i);
#ifdef MZ_DEBUG
        found = true;
#endif
        break;
      }
    }
  }

#ifdef MZ_DEBUG
  Q_ASSERT(found);
#endif
}

bool Navigator::eventHandled() {
  logger.debug() << "Close event handled";

  if (!ExternalOpHandler::instance()->request(
          ExternalOpHandler::OpCloseEvent)) {
    // Something is blocking the close event handler
    return true;
  }

#if defined(MZ_ANDROID) || defined(MZ_WASM)
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
#elif defined(MZ_IOS)
  return false;
#elif defined(MZ_LINUX) || defined(MZ_MACOS) || defined(MZ_WINDOWS) || \
    defined(MZ_DUMMY) || defined(UNIT_TEST)
  logger.error() << "We should not be here! Why "
                    "CloseEventHandler::eventHandled() is called on desktop?!?";
  return true;
#else
#  error Unsupported platform
#endif
}

void Navigator::registerReloader(NavigatorReloader* reloader) {
  Q_ASSERT(!m_reloaders.contains(reloader));
  m_reloaders.append(reloader);
}

void Navigator::unregisterReloader(NavigatorReloader* reloader) {
  Q_ASSERT(m_reloaders.contains(reloader));
  m_reloaders.removeOne(reloader);
}

// static
void Navigator::registerScreen(int screenId, LoadPolicy loadPolicy,
                               const QString& qmlComponentUrl,
                               const QVector<int>& requiresAppState,
                               int8_t (*priorityGetter)(int*),
                               bool (*quitBlocked)()) {
  s_screens.append(ScreenData(screenId, loadPolicy, qmlComponentUrl,
                              requiresAppState, priorityGetter, quitBlocked));
}

void Navigator::reloadCurrentScreen() {
  auto old_screen = m_currentScreen;

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
        return;
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
      return;
    }

    continue;
  }
  requestScreen(old_screen, ForceReloadAll);
}
