/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FEATURETYPES_H
#define FEATURETYPES_H

#include <variant>

namespace Feature {


/**
* A Feature is a flag that can be used to gate functionality.
* This is a constant value that is determined at compile time and cannot be overridden.
* This allows for constexpr checks. i.e 
* if constexpr (isEnabled(f)) { ... }
*
* They should be used if possible. If the flag needs to be determined at runtime, use RuntimeFeature instead.
*/
struct ConstantFeature {
  const char* id;
  const char* name;
  bool supported;
};

constexpr bool isEnabled(const ConstantFeature& f) { return f.supported; }

/**
* A RuntimeFeature is a flag that can be used to gate functionality.
* This value is determined at runtime and cannot be overridden.
*
* They should be used if the value cannot be determined at compile time but also doesn't need to be overridden. If the flag needs to be overridden, use OverridableFeature instead.
*/
struct RuntimeFeature {
  const char* id;
  const char* name;
  bool (*evaluator)();
};
inline bool isEnabled(const RuntimeFeature& f) { return f.evaluator(); }

/**
* An OverridableFeature is a flag that can be used to gate functionality.
* This value is determined at runtime but can be overridden.
* Possible sources for overrides include:
* - Remote configuration /api/v1/features (Guardian API)
* - Local override from the Developer menu in the app
* Overrides are stored in SettingsHolder and take precedence over the default value.
*/
struct OverridableFeature {
  const char* id;
  const char* name;
  bool (*evaluator)();
  bool (*canFlipOn)() = +[] { return true; };
  bool (*canFlipOff)() = +[] { return true; };
};

/**
* Toggle an OverridableFeature on or off. This will update the override settings in SettingsHolder.
* The new value will be returned on the next call to isEnabled for the feature.
* @param f The feature to toggle.
* @param enabled Whether the feature should be enabled or disabled.
*/
void toggle(const OverridableFeature& f, bool enabled);

/**
* Returns true if the feature is enabled. 
* @param f The feature to check.
* @return Whether the feature is enabled.
*/
bool isEnabled(const OverridableFeature& f);
/**
* A type that can hold any feature type.
*/
using AnyFeature = std::variant<const ConstantFeature*,
                                const RuntimeFeature*,
                                const OverridableFeature*>;

/**
 * Visitor helper for std::visit on AnyFeature. Usage:
 *   std::visit(AnyFeatureVisitor{
 *                  [](const ConstantFeature* c)    { ... },
 *                  [](const RuntimeFeature* r)     { ... },
 *                  [](const OverridableFeature* o) { ... },
 *              }, feature);
 */
template <class... Ts>
struct AnyFeatureVisitor : Ts... {
  using Ts::operator()...;
};
template <class... Ts>
AnyFeatureVisitor(Ts...) -> AnyFeatureVisitor<Ts...>;
/**
* Returns true if the feature is enabled. 
* @param f The feature to check.
* @return Whether the feature is enabled.
*/
bool isEnabled(const AnyFeature& f);

inline const char* getId(const AnyFeature& f) {
  return std::visit([](auto* feat) { return feat->id; }, f);
}

constexpr AnyFeature ref(const ConstantFeature& f) { return &f; }
constexpr AnyFeature ref(const RuntimeFeature& f) { return &f; }
constexpr AnyFeature ref(const OverridableFeature& f) { return &f; }

}  // namespace Feature

#endif  // FEATURETYPES_H
