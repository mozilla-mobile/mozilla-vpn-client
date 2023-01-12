/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef URLOPENER_H
#define URLOPENER_H

#include <QMap>
#include <QObject>
#include <functional>

class QUrl;

class UrlOpener final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(UrlOpener)

  Q_PROPERTY(QString lastUrl READ lastUrl NOTIFY lastUrlChanged)

 public:
  static UrlOpener* instance();
  ~UrlOpener();

  Q_INVOKABLE void openUrlLabel(const QString& urlLabel);
  Q_INVOKABLE void openUrl(const QString& url);
  Q_INVOKABLE void openUrl(const QUrl& url);

  const QString& lastUrl() const { return m_lastUrl; }
  void setLastUrl(const QString& url) {
    m_lastUrl = url;
    emit lastUrlChanged();
  }

  static QUrl replaceUrlParams(const QUrl& originalUrl);

  /**
   * @brief do not open URLs but keep the last one in memory for testing
   */
  void setStealUrls();

  /**
   * @brief add a new label for a custom url
   */
  void registerUrlLabel(const QString& urlLabel,
                        const std::function<QString()>&& callback);

 private:
  explicit UrlOpener(QObject* parent);

 signals:
  void lastUrlChanged();

 private:
  QMap<QString, std::function<QString()>> m_urlLabels;
  QString m_lastUrl;
  bool m_stealUrls = false;
};

#endif  // URLOPENER_H
