/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FEATUREPROXY_H
#define FEATUREPROXY_H

#include <QObject>

#include "featuretypes.h"

class FeatureProxy : public QObject {
  Q_OBJECT

  Q_PROPERTY(QString id READ id CONSTANT)
  Q_PROPERTY(QString name READ name CONSTANT)
  Q_PROPERTY(bool isSupported READ isSupported NOTIFY supportedChanged)
  Q_PROPERTY(bool isToggleable READ isToggleable CONSTANT)

 public:
  FeatureProxy() = default;

  void init(const Feature::AnyFeature* feature);

  QString id() const;
  QString name() const;
  bool isSupported() const;
  bool isToggleable() const;
  bool isOverridable() const;
  const Feature::AnyFeature* feature() const { return m_feature; }

 signals:
  void supportedChanged();

 private:
  const Feature::AnyFeature* m_feature = nullptr;
};

#endif  // FEATUREPROXY_H
