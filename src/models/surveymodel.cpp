/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "surveymodel.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "settingsholder.h"
#include "urlopener.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QUrl>

namespace {

Logger logger(LOG_MAIN, "SurveyModel");

bool sortCallback(const Survey& a, const Survey& b) {
  return a.triggerTime() < b.triggerTime();
}

}  // namespace

SurveyModel::SurveyModel() {
  MVPN_COUNT_CTOR(SurveyModel);

  connect(&m_timer, &QTimer::timeout, this, &SurveyModel::maybeShowSurvey);
  m_timer.start(Constants::surveyTimerMsec());
}

SurveyModel::~SurveyModel() { MVPN_COUNT_DTOR(SurveyModel); }

bool SurveyModel::fromJson(const QByteArray& s) {
  logger.debug() << "Surveys from json";

  if (!s.isEmpty() && m_rawJson == s) {
    logger.debug() << "Nothing has changed";
    return true;
  }

  if (!fromJsonInternal(s)) {
    return false;
  }

  m_rawJson = s;
  return true;
}

bool SurveyModel::fromSettings() {
  auto& settingsHolder = SettingsHolder::instance();

  logger.debug() << "Reading the survey list from settings";

  const QByteArray& json = settingsHolder.surveys();
  if (json.isEmpty() || !fromJsonInternal(json)) {
    return false;
  }

  m_rawJson = json;
  return true;
}

void SurveyModel::writeSettings() {
  SettingsHolder::instance().setSurveys(m_rawJson);
}

bool SurveyModel::fromJsonInternal(const QByteArray& json) {
  m_rawJson = "";
  m_surveys.clear();

  QJsonDocument doc = QJsonDocument::fromJson(json);
  if (!doc.isArray()) {
    return false;
  }

  QJsonArray surveysArray = doc.array();
  for (const QJsonValue surveyValue : surveysArray) {
    Survey survey;
    if (!survey.fromJson(surveyValue)) {
      return false;
    }
    m_surveys.append(survey);
  }

  // Sort by trigger time.
  std::sort(m_surveys.begin(), m_surveys.end(), sortCallback);
  return true;
}

void SurveyModel::maybeShowSurvey() {
  logger.debug() << "Checking surveys";

  if (!m_currentSurveyId.isEmpty()) {
    return;
  }

  for (const Survey& survey : m_surveys) {
    if (survey.isTriggerable()) {
      showSurvey(survey);
      break;
    }
  }
}

void SurveyModel::showSurvey(const Survey& survey) {
  m_currentSurveyId = survey.id();
  emit hasSurveyChanged();
}

void SurveyModel::openCurrentSurvey() {
  if (m_currentSurveyId.isEmpty()) {
    return;
  }

  for (const Survey& survey : m_surveys) {
    if (survey.id() == m_currentSurveyId) {
      dismissCurrentSurvey();
      UrlOpener::open(QUrl(survey.url()));
      break;
    }
  }
}

void SurveyModel::dismissCurrentSurvey() {
  auto& settingsHolder = SettingsHolder::instance();

  QStringList list = settingsHolder.consumedSurveys();
  list.append(m_currentSurveyId);
  settingsHolder.setConsumedSurveys(list);

  m_currentSurveyId.clear();
  emit hasSurveyChanged();
}
