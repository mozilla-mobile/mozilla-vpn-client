/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FEATURE_H
#define FEATURE_H

#include "qmlengineholder.h"
#include "l18nstrings.h"

#include <QObject>
#include <QApplication>

class Feature : public QObject {
  Q_OBJECT

 public:
#define FEATURE(id, name, isMajor, displayNameId, shortDescId, descId,   \
                imgPath, iconPath, linkUrl, releaseVersion, flippableOn, \
                flippableOff, otherFeatureDependencies, callback)        \
  static constexpr const char* Feature_##id = #id;
#include "featureslist.h"
#undef FEATURE

  Q_PROPERTY(QString id MEMBER m_id CONSTANT)
  Q_PROPERTY(QString name MEMBER m_name CONSTANT)
  Q_PROPERTY(bool isMajor READ isMajor CONSTANT)
  Q_PROPERTY(QString displayName READ displayName CONSTANT)
  Q_PROPERTY(QString shortDescription READ shortDescription CONSTANT)
  Q_PROPERTY(QString description READ description CONSTANT)
  Q_PROPERTY(QString imagePath READ imagePath CONSTANT)
  Q_PROPERTY(QString iconPath READ iconPath CONSTANT)
  Q_PROPERTY(QString linkUrl READ linkUrl CONSTANT)
  Q_PROPERTY(QString releaseVersion READ releaseVersion CONSTANT)
  Q_PROPERTY(bool isReleased MEMBER m_released CONSTANT)
  Q_PROPERTY(bool isNew READ isNew CONSTANT)
  Q_PROPERTY(bool isToggleable READ isToggleable CONSTANT)
  Q_PROPERTY(bool isSupported READ isSupported NOTIFY supportedChanged)

#ifndef UNIT_TEST
 private:
#else
 public:
#endif
  Feature(const QString& id, const QString& name, bool isMajor,
          L18nStrings::String displayName_id, L18nStrings::String shortDesc_id,
          L18nStrings::String desc_id, const QString& imgPath,
          const QString& iconPath, const QString& linkUrl,
          const QString& releaseVersion, bool flippableOn, bool flippableOff,
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

  // Checks if the feature is released
  // or force enabled/disable via flip flags
  // returns the features checkSupportCallback otherwise.
  bool isSupported(bool ignoreCache = false) const;

  // Checks if the feature is released ignoring the flip on/off
  bool isSupportedIgnoringFlip() const;

  bool isFlippableOn() const { return m_flippableOn; }
  bool isFlippableOff() const { return m_flippableOff; }

  bool isToggleable() const;

  // Returns true if this is a newly introduced feature
  bool isNew() const { return m_new; }

  // Returns true if this is a feature we marked as major
  bool isMajor() const { return m_majorFeature; }

  QString id() const { return m_id; }
  QString displayName() const;
  QString description() const;
  QString shortDescription() const;
  QString imagePath() const { return m_imagePath; }
  QString iconPath() const { return m_iconPath; }
  QString linkUrl() const { return m_linkUrl; }
  QString releaseVersion() const { return m_releaseVersion; }

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

  // Major Features are shown at "Whats new"
  const bool m_majorFeature;

  // Id for Display Name, used in the whats new list
  const L18nStrings::String m_displayName_id;
  // Id for Short Description, used in the whats new list
  const L18nStrings::String m_shortDescription_id;
  // Id for Longer description, used in the whats new cards
  const L18nStrings::String m_description_id;
  // Path to Big image, used in whats new card
  const QString m_imagePath;
  // Path to Icon for small list view thing
  const QString m_iconPath;
  // Link URL to external information on the feature
  const QString m_linkUrl;
  // Version that the feature was released in
  const QString m_releaseVersion;

  // If true, the feature can be enabled.
  const bool m_flippableOn;
  // If true, the feature can be disabled.
  const bool m_flippableOff;

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

  // Determines if the feature should be available if possible
  // Otherwise it can only be reached via a dev-override
  bool m_released = false;
  // Is true if the Feature was released in this Version
  bool m_new = false;

#ifdef UNIT_TEST
  friend class TestAddonIndex;
#endif
};

#endif  // FEATURE_H
