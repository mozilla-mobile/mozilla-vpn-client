/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef GLEANTEST_H
#define GLEANTEST_H

#include <QObject>

class GleanTest final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(GleanTest)

 public:
  GleanTest();
  ~GleanTest();

  static GleanTest* instance();

  const QByteArray& lastUrl() const { return m_lastUrl; }
  const QByteArray& lastData() const { return m_lastData; }

  void reset();

  Q_INVOKABLE void requestDone(const QByteArray& url, const QByteArray& data);

 private:
  QByteArray m_lastUrl;
  QByteArray m_lastData;
};

#endif  // GLEANTEST_H
