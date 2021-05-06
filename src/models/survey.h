/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SURVEY_H
#define SURVEY_H

#include <QString>

class QJsonValue;

class Survey final {
 public:
  Survey();
  Survey(const Survey& other);
  Survey& operator=(const Survey& other);
  ~Survey();

  [[nodiscard]] bool fromJson(const QJsonValue& json);

  const QString& id() const { return m_id; }
  const QString& url() const { return m_url; }
  uint32_t triggerTime() const { return m_triggerTime; }

  bool isTriggerable() const;

 private:
  QString m_id;
  QString m_url;
  uint32_t m_triggerTime;
};

#endif  // SURVEY_H
