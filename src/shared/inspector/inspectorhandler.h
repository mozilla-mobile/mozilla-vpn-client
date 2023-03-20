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

  static QString getObjectClass(const QObject* target);
  static QJsonObject getViewTree();
  static QJsonObject serialize(QQuickItem* item);
  static void itemsPicked(const QList<QQuickItem*>& objects);

  void recv(const QByteArray& buffer);
  virtual void send(const QByteArray& buffer) = 0;

  /**
   * @brief Set a constructor callback to inform the caller about this new
   * inspector handler.
   */
  static void setConstructorCallback(
      std::function<void(InspectorHandler* inspectorHandler)>&& callback);

  /**
   * @brief Register a new command.
   */
  static void registerCommand(
      const QString& commandName, const QString& commandDescription,
      int32_t arguments,
      std::function<QJsonObject(InspectorHandler*, const QList<QByteArray>&)>&&
          callback);

 protected:
  explicit InspectorHandler(QObject* parent);
  virtual ~InspectorHandler();

 private:
  void addonLoadCompleted();
  void logEntryAdded(const QByteArray& log);
  void networkRequestFinished(QNetworkReply* reply);
};

#endif  // INSPECTORHANDLER_H
