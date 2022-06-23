/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SURVEYMODEL_H
#define SURVEYMODEL_H

#include "survey.h"

#include <QList>
#include <QObject>
#include <QTimer>

class SurveyModel final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(SurveyModel)

  Q_PROPERTY(bool hasSurvey READ hasSurvey NOTIFY hasSurveyChanged)

 public:
  SurveyModel();
  ~SurveyModel();

  [[nodiscard]] bool fromJson(const QByteArray& s);

  [[nodiscard]] bool fromSettings();

  void writeSettings();

  const QList<Survey>& surveys() const { return m_surveys; }

  bool hasSurvey() const { return !m_currentSurveyId.isEmpty(); }

  Q_INVOKABLE void openCurrentSurvey();
  Q_INVOKABLE void dismissCurrentSurvey();

 signals:
  void hasSurveyChanged();

 private:
  [[nodiscard]] bool fromJsonInternal(const QByteArray& s);

  void maybeShowSurvey();

  void showSurvey(const Survey& survey);

 private:
  QList<Survey> m_surveys;

  QTimer m_timer;
  QByteArray m_rawJson;

  QString m_currentSurveyId;
};

#endif  // SURVEYMODEL_H
