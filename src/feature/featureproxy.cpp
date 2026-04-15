/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "featureproxy.h"

using namespace Feature;

void FeatureProxy::init(const AnyFeature* feature) { m_feature = feature; }

QString FeatureProxy::id() const {
  return std::visit([](auto* f) { return QString(f->id); }, *m_feature);
}

QString FeatureProxy::name() const {
  return std::visit([](auto* f) { return QString(f->name); }, *m_feature);
}

bool FeatureProxy::isSupported() const { return isEnabled(*m_feature); }

bool FeatureProxy::isToggleable() const {
  return std::visit(
      overloaded{
          [](const ConstantFeature*) { return false; },
          [](const RuntimeFeature*) { return false; },
          [](const OverridableFeature* f) -> bool {
            bool onByDefault = f->evaluator();
            return (onByDefault && f->canFlipOff()) ||
                   (!onByDefault && f->canFlipOn());
          },
      },
      *m_feature);
}

bool FeatureProxy::isOverridable() const {
  return std::holds_alternative<const OverridableFeature*>(*m_feature);
}
