/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADJUSTPROXYPACKAGEHANDLER_H
#define ADJUSTPROXYPACKAGEHANDLER_H

#include <QByteArray>
#include <QUrl>
#include <QUrlQuery>

class AdjustProxyPackageHandler final {
  Q_DISABLE_COPY_MOVE(AdjustProxyPackageHandler)

 public:
  enum ProcessingState {
    NotStarted,
    FirstLineDone,
    HeadersDone,
    ParametersDone,
    ProcessingDone,
    InvalidRequest
  };

  AdjustProxyPackageHandler();
  ~AdjustProxyPackageHandler();

  void processData(const QByteArray& input);
  ProcessingState getProcessingState() { return m_state; }
  bool isProcessingDone() { return m_state == ProcessingState::ProcessingDone; }
  bool isInvalidRequest() { return m_state == ProcessingState::InvalidRequest; }

  const QString& getMethod() const { return m_method; }
  const QString& getPath() const { return m_path; }
  const QString getQueryParameters() const {
    return m_queryParameters.toString();
  }
  const QString getBodyParameters() const {
    return m_bodyParameters.toString();
  }
  const QStringList& getUnknownParameters() const {
    return m_unknownParameters;
  }
  const QList<QPair<QString, QString>>& getHeaders() const { return m_headers; }

 private:
  bool processFirstLine();
  bool processHeaders();
  bool processParameters();
  void filterParameters();

 public:
  ProcessingState m_state = ProcessingState::NotStarted;
  QByteArray m_buffer;
  uint32_t m_contentLength = 0;
  QString m_method;
  QUrl m_route;
  QString m_path;
  QList<QPair<QString, QString>> m_headers;
  QUrlQuery m_queryParameters;
  QUrlQuery m_bodyParameters;
  QStringList m_unknownParameters;
};

#endif  // ADJUSTPROXYPACKAGEHANDLER_H