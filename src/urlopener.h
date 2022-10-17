/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef URLOPENER_H
#define URLOPENER_H

#include <QObject>

class QUrl;

class UrlOpener final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(UrlOpener)

  Q_PROPERTY(
      QString lastUrl READ lastUrl WRITE setLastUrl NOTIFY lastUrlChanged)

 public:
  enum LinkType {
    LinkAccount,
    LinkContact,
    LinkForgotPassword,
    LinkLeaveReview,
    LinkHelpSupport,
    LinkTermsOfService,
    LinkPrivacyNotice,
    LinkUpdate,
    LinkInspector,
    LinkSubscriptionBlocked,
    LinkSplitTunnelHelp,
    LinkCaptivePortal,
    LinkRelayPremium,
    LinkSubscriptionIapApple,
    LinkSubscriptionFxa,
    LinkSubscriptionIapGoogle,
    LinkUpgradeToBundle,
  };
  Q_ENUM(LinkType)

  static UrlOpener* instance();
  ~UrlOpener();

  Q_INVOKABLE void openLink(UrlOpener::LinkType linkType);
  Q_INVOKABLE void openUrl(const QString& linkUrl);

  void open(QUrl url, bool addEmailAddress = false);

  const QString& lastUrl() const { return m_lastUrl; }
  void setLastUrl(const QString& url) {
    m_lastUrl = url;
    emit lastUrlChanged();
  }

  static QUrl replaceUrlParams(const QUrl& originalUrl);

 signals:
  void lastUrlChanged();

 private:
  explicit UrlOpener(QObject* parent);

 private:
  QString m_lastUrl;
};

#endif  // URLOPENER_H
