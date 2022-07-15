/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDON_H
#define ADDON_H

#include <QObject>
#include <QTranslator>

class AddonConditionWatcher;
class QJsonObject;

class Addon : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Addon)

  Q_PROPERTY(QString id READ id CONSTANT)
  Q_PROPERTY(QString name MEMBER m_name CONSTANT)
  Q_PROPERTY(QString type READ type CONSTANT)

 public:
  static Addon* create(QObject* parent, const QString& manifestFileName);

  static bool evaluateConditions(const QJsonObject& conditions);

  virtual ~Addon();

  const QString& id() const { return m_id; }
  const QString& type() const { return m_type; }

  void retranslate();

  virtual bool enabled() const;

 signals:
  void conditionChanged(bool enabled);

 protected:
  Addon(QObject* parent, const QString& manifestFileName, const QString& id,
        const QString& name, const QString& type);

  virtual void enable();
  virtual void disable();

 private:
  void maybeCreateConditionWatchers(const QJsonObject& conditions);

 private:
  const QString m_manifestFileName;
  const QString m_id;
  const QString m_name;
  const QString m_type;

  QTranslator m_translator;

  AddonConditionWatcher* m_conditionWatcher = nullptr;
};

#endif  // ADDON_H
