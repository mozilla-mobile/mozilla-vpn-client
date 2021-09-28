/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADJUSTPROXYCONNECTION_H
#define ADJUSTPROXYCONNECTION_H

#include <QByteArray>
#include <QObject>
#include <QHash>
#include <QUrl>
#include <QUrlQuery>

class QTcpSocket;

class AdjustProxyConnection final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AdjustProxyConnection)

 public:
  AdjustProxyConnection(QObject* parent, QTcpSocket* connection);
  ~AdjustProxyConnection();

 private:
  void readData();

  void processFirstLine();
  void processHeaders();
  void processParameters();
  void filterParametersAndForwardRequest();
  void forwardRequest();

 private:
  enum ProcessingState {
    NotStarted,
    FirstLineDone,
    HeadersDone,
    ParametersDone
  };

  QTcpSocket* m_connection = nullptr;

  ProcessingState m_state = ProcessingState::NotStarted;
  QByteArray m_buffer;
  QString m_method;
  QUrl m_route;
  QList<QPair<QString, QString>> m_headers;
  QUrlQuery m_parameters;
};

#endif  // ADJUSTPROXYCONNECTION_H
