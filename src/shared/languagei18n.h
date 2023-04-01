/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LANGUAGEI18N_H
#define LANGUAGEI18N_H

#include <QHash>
#include <QList>
#include <QObject>
#include <QString>

class QJsonValue;

class LanguageI18N final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(LanguageI18N)

 public:
  static LanguageI18N* instance();

  ~LanguageI18N();

  bool languageExists(const QString& languageCode);

  QString translateLanguage(const QString& translationCode,
                            const QString& languageCode);

  int languageCompare(const QString& languageCodeA,
                      const QString& languageCodeB);

 private:
  explicit LanguageI18N(QObject* parent);

  void initialize();
  void addLanguage(const QJsonValue& value);

 private:
  QList<QString> m_languageList;
  QHash<QString, QString> m_items;
};

#endif  // LANGUAGEI18N_H
