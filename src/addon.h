/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDON_H
#define ADDON_H

#include <QObject>
#include <QTranslator>

class Addon final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Addon)

  Q_PROPERTY(QString id MEMBER m_id CONSTANT)
  Q_PROPERTY(QString name MEMBER m_name CONSTANT)
  Q_PROPERTY(QString qml READ qml CONSTANT)

 public:
  enum AddonType {
    AddonTypeDemo,
    AddonTypeGuide,
    AddonTypeI18n,
    AddonTypeTutorial,
  };

  Addon(QObject* parent, AddonType addonType, const QString& manifestFileName,
        const QString& id, const QString& name, const QString& qml);
  ~Addon();

  AddonType type() const { return m_addonType; }

  QString qml() const;

  void retranslate();

 private:
  const AddonType m_addonType;
  const QString m_manifestFileName;
  const QString m_id;
  const QString m_name;
  const QString m_qml;

  QTranslator m_translator;
};

#endif  // ADDON_H
