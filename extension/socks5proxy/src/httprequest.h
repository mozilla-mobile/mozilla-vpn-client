/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <QMap>
#include <QString>

class QIODevice;

class HttpRequest {
 public:
  HttpRequest() = default;
  HttpRequest(const QString& line);

  bool isValid() const;
  static HttpRequest peek(QIODevice* socket);

  bool addHeader(const QString& line);
  const QString& header(const QString& name) const;

  QString m_method;
  QString m_uri;
  int m_verMajor = -1;
  int m_verMinor = -1;

 private:
  QMap<QString, QString> m_headers;
  static const QRegularExpression m_regex;
};

#endif  // HTTPREQUEST_H
