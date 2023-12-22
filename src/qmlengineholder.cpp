/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "qmlengineholder.h"

#include <QNetworkAccessManager>
#include <QQmlApplicationEngine>
#include <QWindow>

#include "addons/manager/addonmanager.h"
#include "authenticationinapp/authenticationinapp.h"
#include "env.h"
#include "errorhandler.h"
#include "feature/featuremodel.h"
#include "frontend/navigationbarmodel.h"
#include "frontend/navigator.h"
#include "glean/generated/metrics.h"
#include "glean/generated/pings.h"
#include "glean/mzglean.h"
#include "leakdetector.h"
#include "localizer.h"
#include "loghandler.h"
#include "models/licensemodel.h"
#include "settingsholder.h"
#include "theme.h"
#include "urlopener.h"
#include "utils.h"

#ifdef SENTRY_ENABLED
#  include "sentry/sentryadapter.h"
#endif

namespace {
QmlEngineHolder* s_instance = nullptr;
}  // namespace

QmlEngineHolder::QmlEngineHolder(QQmlEngine* engine) : m_engine(engine) {
  MZ_COUNT_CTOR(QmlEngineHolder);

  Q_ASSERT(engine);
  Q_ASSERT(!s_instance);
  s_instance = this;

  qmlRegisterSingletonInstance("Mozilla.Shared", 1, 0, "GleanPings",
                               __DONOTUSE__GleanPings::instance());
  qmlRegisterSingletonInstance("Mozilla.Shared", 1, 0, "Glean",
                               __DONOTUSE__GleanMetrics::instance());
  qmlRegisterSingletonInstance("Mozilla.Shared", 1, 0, "MZAddonManager",
                               AddonManager::instance());
  qmlRegisterSingletonInstance("Mozilla.Shared", 1, 0, "MZAuthInApp",
                               AuthenticationInApp::instance());
#ifdef SENTRY_ENABLED
  qmlRegisterSingletonInstance("Mozilla.Shared", 1, 0, "MZCrashReporter",
                               SentryAdapter::instance());
#endif
  qmlRegisterSingletonInstance("Mozilla.Shared", 1, 0, "MZErrorHandler",
                               ErrorHandler::instance());
  qmlRegisterSingletonInstance("Mozilla.Shared", 1, 0, "MZEnv",
                               Env::instance());
  qmlRegisterSingletonInstance("Mozilla.Shared", 1, 0, "MZFeatureList",
                               FeatureModel::instance());
  qmlRegisterSingletonInstance("Mozilla.Shared", 1, 0, "MZLicenseModel",
                               LicenseModel::instance());
  qmlRegisterSingletonInstance("Mozilla.Shared", 1, 0, "MZLocalizer",
                               Localizer::instance());
  qmlRegisterSingletonInstance("Mozilla.Shared", 1, 0, "MZLog",
                               LogHandler::instance());
  qmlRegisterSingletonInstance("Mozilla.Shared", 1, 0, "MZNavigator",
                               Navigator::instance());
  qmlRegisterSingletonInstance("Mozilla.Shared", 1, 0, "MZNavigationBarModel",
                               NavigationBarModel::instance());
  qmlRegisterSingletonInstance("Mozilla.Shared", 1, 0, "MZSettings",
                               SettingsHolder::instance());
  qmlRegisterSingletonInstance("Mozilla.Shared", 1, 0, "MZUrlOpener",
                               UrlOpener::instance());
  qmlRegisterSingletonInstance("Mozilla.Shared", 1, 0, "MZUtils",
                               Utils::instance());

  Theme::instance()->initialize(engine);
  qmlRegisterSingletonInstance("Mozilla.Shared", 1, 0, "MZTheme",
                               Theme::instance());
}

QmlEngineHolder::~QmlEngineHolder() {
  MZ_COUNT_DTOR(QmlEngineHolder);

  Q_ASSERT(s_instance == this);
  s_instance = nullptr;
}

// static
QmlEngineHolder* QmlEngineHolder::instance() {
  Q_ASSERT(s_instance);
  return s_instance;
}

// static
bool QmlEngineHolder::exists() { return !!s_instance; }

QNetworkAccessManager* QmlEngineHolder::networkAccessManager() {
  return m_engine->networkAccessManager();
}

void QmlEngineHolder::clearCacheInternal() {
  QNetworkAccessManager* nam = networkAccessManager();
  Q_ASSERT(nam);

  nam->clearAccessCache();
  nam->clearConnectionCache();
}

QWindow* QmlEngineHolder::window() const {
  QQmlApplicationEngine* engine =
      qobject_cast<QQmlApplicationEngine*>(m_engine);
  if (!engine) return nullptr;

  Q_ASSERT(hasWindow());
  QObject* rootObject = engine->rootObjects().first();
  return qobject_cast<QWindow*>(rootObject);
}

bool QmlEngineHolder::hasWindow() const {
  return !qobject_cast<QQmlApplicationEngine*>(m_engine)
              ->rootObjects()
              .isEmpty();
}

void QmlEngineHolder::showWindow() {
  QWindow* w = window();
  Q_ASSERT(w);

  w->show();
  w->raise();
  w->requestActivate();
}

void QmlEngineHolder::hideWindow() {
  QWindow* w = window();
  Q_ASSERT(w);

  w->hide();
}
