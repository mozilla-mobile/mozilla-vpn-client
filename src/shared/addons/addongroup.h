/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDONGROUP_H
#define ADDONGROUP_H

#include "addon.h"

class AddonGroup final : public Addon {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AddonGroup)

 public:
  AddonGroup(QObject* parent, const QList<Addon*>& addons,
             const QString& manifestFileName, const QString& id,
             const QString& name);

  ~AddonGroup();

  void retranslate() override;

  const QList<Addon*>& addons() const { return m_addons; }

  Addon* as(Type) override;

  Q_INVOKABLE void enable(Type type);
  Q_INVOKABLE void disable(Type type);

 private:
  void enable() override;
  void disable() override;

  void setState(AddonState* state) override;

  Addon* asInternal(Type);

 private:
  QList<Addon*> m_addons;
  int m_retranslationCompletedCount = 0;
};

#endif  // ADDONGROUP_H
