/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "urlopener.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QUrlQuery>

#include "constants.h"
#include "env.h"
#include "leakdetector.h"
#include "logger.h"
#include "settingsholder.h"

namespace {
Logger logger("UrlOpener");
UrlOpener* s_instance = nullptr;
}  // namespace

// static
UrlOpener* UrlOpener::instance() {
  if (!s_instance) {
    s_instance = new UrlOpener(qApp);
  }
  return s_instance;
}

UrlOpener::UrlOpener(QObject* parent) : QObject(parent) {
  MZ_COUNT_CTOR(UrlOpener);

  registerUrlLabel("account", []() -> QString {
    QUrl url(Constants::fxaUrl());
    QUrlQuery query(url.query());
    query.addQueryItem("email", SettingsHolder::instance()->userEmail());
    url.setQuery(query);
    return url.toString();
  });

  UrlOpener::registerUrlLabel("forgotPassword", []() -> QString {
    return QString("%1/reset_password").arg(Constants::fxaUrl());
  });
}

UrlOpener::~UrlOpener() { MZ_COUNT_DTOR(UrlOpener); }

void UrlOpener::openUrlLabel(const QString& label) {
  logger.debug() << "Opening url label: " << label;
  Q_ASSERT(m_urlLabels.contains(label));
  openUrl(m_urlLabels[label]());
}

void UrlOpener::openUrl(const QString& linkUrl) {
  logger.debug() << "Opening url: " << linkUrl;
  openUrl(QUrl(linkUrl));
}

void UrlOpener::openUrl(const QUrl& a_url) {
  QUrl url = replaceUrlParams(a_url);
  logger.debug() << "Opening url 2: " << url.toDisplayString();

  if (!Constants::inProduction()) {
    setLastUrl(url.toString());
    logger.debug() << "m_stealUrls" << m_stealUrls;
    if (m_stealUrls) {
      return;
    }
  }

  QDesktopServices::openUrl(url);
}

// static
QUrl UrlOpener::replaceUrlParams(const QUrl& originalUrl) {
  if (!originalUrl.isValid()) {
    logger.error() << "Invalid survey URL";
    return originalUrl;
  }

  QUrl url(originalUrl);
  QUrlQuery currentQuery(url.query());
  QUrlQuery newQuery;

  // We support both __VPN_* and __MZ_* for legacy addons.
  for (QPair<QString, QString>& item : currentQuery.queryItems()) {
    if (item.second == "__VPN_VERSION__" || item.second == "__MZ_VERSION__") {
      newQuery.addQueryItem(item.first, Env::versionString());
    } else if (item.second == "__VPN_BUILDNUMBER__" ||
               item.second == "__MZ_BUILDNUMBER__") {
      newQuery.addQueryItem(item.first, Env::buildNumber());
    } else if (item.second == "__VPN_OS__" || item.second == "__MZ_OS__") {
      newQuery.addQueryItem(item.first, Env::osVersion());
    } else if (item.second == "__VPN_PLATFORM__" ||
               item.second == "__MZ_PLATFORM__") {
      newQuery.addQueryItem(item.first, Env::platform());
    } else if (item.second == "__VPN_ARCH__" || item.second == "__MZ_ARCH__") {
      newQuery.addQueryItem(item.first, Env::architecture());
    } else {
      newQuery.addQueryItem(item.first, item.second);
    }
  }

  url.setQuery(newQuery);
  return url;
}

void UrlOpener::registerUrlLabel(const QString& urlLabel,
                                 const std::function<QString()>&& callback) {
  m_urlLabels.insert(urlLabel, std::move(callback));
}

void UrlOpener::setStealUrls() { m_stealUrls = true; }
