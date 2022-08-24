/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDONPROPERTY_H
#define ADDONPROPERTY_H

#include <QObject>

#define ADDON_PROPERTY(name, member, signal)                            \
  Q_PROPERTY(QString name READ addonPropertyGetter##name NOTIFY signal) \
  QString addonPropertyGetter##name() { return member.get(); }

class AddonProperty final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AddonProperty)

  Q_PROPERTY(QString value READ get CONSTANT)

 public:
  AddonProperty();
  ~AddonProperty();

  void initialize(const QString& id, const QString& fallback);

  QString get() const;

 private:
  bool m_initialized = false;

  QString m_id;
  QString m_fallback;
};

#endif  // ADDONPROPERTY_H
