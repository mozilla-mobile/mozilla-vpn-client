/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FEATURE_H
#define FEATURE_H

#include <QApplication>
#include <QObject>

#include "settings/settinggroup.h"

class Feature : public QObject {
  Q_OBJECT

 public:
#define FEATURE(id, name, flippableOn, flippableOff, otherFeatureDependencies, \
                callback)                                                      \
  static constexpr const char* Feature_##id = #id;
#include "featurelist.h"
#undef FEATURE

#define EXPERIMENTAL_FEATURE(id, name, ...) \
  static constexpr const char* ExperimentalFeature_##id = #id;
#include "experimentalfeaturelist.h"
#undef EXPERIMENTAL_FEATURE

  Q_PROPERTY(QString id MEMBER m_id CONSTANT)
  Q_PROPERTY(QString name MEMBER m_name CONSTANT)
  Q_PROPERTY(bool isToggleable READ isToggleable CONSTANT)
  Q_PROPERTY(bool isSupported READ isSupported NOTIFY supportedChanged)

#ifndef UNIT_TEST
 private:
#else
 public:
#endif
  Feature(const QString& id, const QString& name,
          std::function<bool()>&& flippableOn,
          std::function<bool()>&& flippableOff,
          const QStringList& otherFeatureDependencies,
          std::function<bool()>&& callback);
  ~Feature();

 public:
  static const QList<Feature*>& getAll();

  // Returns a Pointer to the Feature with id, crashes client if
  // feature does not exist :)
  static const Feature* get(const QString& featureID);

  // Similar to the previous get, but it doesn't crash. This is meant to be
  // used only to enable the features via REST API.
  static const Feature* getOrNull(const QString& featureID);

  // Checks if the feature is supported
  // or force enabled/disable via flip flags
  // returns the features checkSupportCallback otherwise.
  bool isSupported(bool ignoreCache = false) const;

  // Checks if the feature is supported ignoring the flip on/off
  bool isSupportedIgnoringFlip() const;

  bool isFlippableOn() const { return m_flippableOn(); }
  bool isFlippableOff() const { return m_flippableOff(); }

  bool isToggleable() const;

  QString id() const { return m_id; }

 signals:
  // This signal is emitted if the underlying factors for support changed e.g
  // Controller support level for features depending on this or if the feature
  // has been flipped on/off somehow.
  void supportedChanged();

 private:
  static void maybeInitialize();
  void maybeFlipOnOrOff();

  // Returns true if this feature is flipped on via settings
  bool isFlippedOn(bool ignoreCache = false) const;

  // Returns true if this feature is flipped off via settings
  bool isFlippedOff(bool ignoreCache = false) const;

 private:
  // Unique Identifier of the Feature, used to Check
  // Capapbilities of the Daemon/Server or if is Force-Enabled/Disabled in the
  // Dev Menu
  const QString m_id;

  // Human Readable Name of the Feature
  const QString m_name;

  // If this callback returns true, the feature can be enabled.
  std::function<bool()> m_flippableOn;
  // If this callback returns true, the feature can be disabled.
  std::function<bool()> m_flippableOff;

  // List of other features to be supported in order to support this one.
  const QStringList m_featureDependencies;

  // The callback to see if this feature is supported or not
  std::function<bool()> m_callback;

  // How to compute the feature support value.
  enum State {
    // Just use what the `checkSupportCallback` method returns
    DefaultValue,
    // This pref is forced to be true
    FlippedOn,
    // This pref is forced to be false
    FlippedOff,
  };
  State m_state = DefaultValue;

#ifdef UNIT_TEST
  friend class TestAddonIndex;
  friend class TestResourceLoader;
#endif
};

#endif  // FEATURE_H
