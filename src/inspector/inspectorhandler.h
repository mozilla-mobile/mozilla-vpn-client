/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef INSPECTORHANDLER_H
#define INSPECTORHANDLER_H

#include <QByteArray>
#include <QObject>

class QNetworkReply;
class QUrl;
class QQuickItem;

class InspectorHandler : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(InspectorHandler)

 public:
  static void initialize();

  static bool stealUrls();
  static bool mockFreeTrial();
  static QString appVersionForUpdate();
  static QString getObjectClass(const QObject* target);
  static QJsonObject getViewTree();
  static QJsonObject serialize(QQuickItem* item);
  static void itemsPicked(const QList<QQuickItem*>& objects);

  void recv(const QByteArray& buffer);
  virtual void send(const QByteArray& buffer) = 0;

 protected:
  explicit InspectorHandler(QObject* parent);
  virtual ~InspectorHandler();

 private:
  void logEntryAdded(const QByteArray& log);
  void notificationShown(const QString& title, const QString& message);
  void networkRequestFinished(QNetworkReply* reply);
};

#endif  // INSPECTORHANDLER_H
