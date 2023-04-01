/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDONREPLACER_H
#define ADDONREPLACER_H

#include <QMap>
#include <QQmlAbstractUrlInterceptor>

#include "addon.h"

class QUrl;
class QJsonObject;

class AddonReplacer final : public Addon, public QQmlAbstractUrlInterceptor {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AddonReplacer)

 public:
  static Addon* create(QObject* parent, const QString& manifestFileName,
                       const QString& id, const QString& name,
                       const QJsonObject& obj);

  ~AddonReplacer();

  // QQmlAbstractUrlInterceptor class
  QUrl intercept(const QUrl& url,
                 QQmlAbstractUrlInterceptor::DataType type) override;

 private:
  AddonReplacer(QObject* parent, const QString& manifestFileName,
                const QString& id, const QString& name);

  void enable() override;
  void disable() override;

 private:
  enum UrlType {
    eUrlFile,
    eUrlDirectory,
  };

  struct Replace {
    UrlType m_type;
    QString m_request;
    QString m_response;
  };

  QList<Replace> m_replaces;
};

#endif  // ADDONREPLACER_H
