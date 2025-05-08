/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef XDGAPPEARANCE_H
#define XDGAPPEARANCE_H

#include <QObject>

#include "xdgportal.h"

class QDBusPendingCallWatcher;

class XdgAppearance final : public XdgPortal {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(XdgAppearance)

  Q_PROPERTY(uint colorScheme READ colorScheme NOTIFY colorSchemeChanged)
  Q_PROPERTY(uint accentColor READ accentColor NOTIFY accentColorChanged)
  Q_PROPERTY(uint contrast READ contrast NOTIFY contrastChanged)

 public:
  explicit XdgAppearance(QObject* parent = nullptr);
  ~XdgAppearance();

  uint colorScheme();
  uint accentColor();
  uint contrast();

 signals:
  void colorSchemeChanged();
  void accentColorChanged();
  void contrastChanged();

 private slots:
  void xdgSettingChanged(const QString& ns, const QString& key,
                         const QDBusVariant& value);

 private:
  uint readValueUint(const QString& name);
};

#endif  // XDGAPPEARANCE_H
