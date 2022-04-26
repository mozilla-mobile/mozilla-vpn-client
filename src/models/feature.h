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

  Q_PROPERTY(QString id MEMBER m_id CONSTANT)
  Q_PROPERTY(QString name MEMBER m_name CONSTANT)
  Q_PROPERTY(bool isMajor READ isMajor CONSTANT)
  Q_PROPERTY(QString displayName READ displayName CONSTANT)
  Q_PROPERTY(QString shortDescription READ shortDescription CONSTANT)
  Q_PROPERTY(QString description READ description CONSTANT)
  Q_PROPERTY(QString imagePath READ imagePath CONSTANT)
  Q_PROPERTY(QString iconPath READ iconPath CONSTANT)
  Q_PROPERTY(QString linkUrl READ linkUrl CONSTANT)
  Q_PROPERTY(bool isReleased MEMBER m_released CONSTANT)
  Q_PROPERTY(bool isNew READ isNew CONSTANT)
  Q_PROPERTY(bool devModeWriteable MEMBER m_devModeWriteable CONSTANT)
  Q_PROPERTY(bool isSupported READ isSupported NOTIFY supportedChanged)

  // protected:
 public:
  Feature(const QString& id, const QString& name, bool isMajor,
          L18nStrings::String displayName_id, L18nStrings::String shortDesc_id,
          L18nStrings::String desc_id, const QString& imgPath,
          const QString& iconPath, const QString& linkUrl,
          const QString& releaseVersion, bool devModeWriteable,
          const QStringList& otherFeatureDependencies);

 public:
  virtual ~Feature() = default;

  static QList<Feature*> getAll();

  // Returns a Pointer to the Feature with id, crashes client if
  // feature does not exist :)
  static const Feature* get(const QString& featureID);

  // Similar to the previous get, but it doesn't crash. This is meant to be
  // used only to enable the features via REST API.
  static const Feature* getOrNull(const QString& featureID);

  // Checks if the feature is released
  // or force enabled by the DevMode
  // returns the features checkSupportCallback otherwise.
  bool isSupported() const;

  // Returns true if it was enabled via DevMode
  Q_INVOKABLE bool isDevModeEnabled() const;

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

 signals:
  // Is send if the underlying factors for support changed
  // e.g Controller support level for features depending on this
  // or if the devmode enabled a feature
  void supportedChanged();

 protected:
  // Implemented by each feature.
  // Should check if the feature could be used, if released
  virtual bool checkSupportCallback() const = 0;

  void maybeChangeDevMode(bool newDevModeEnabled);

  // Unique Identifier of the Feature, used to Check
  // Capapbilities of the Daemon/Server or if is Force-Enabled in the Dev Menu
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

  // If true, the feature can be enabled in the Dev-Settings
  const bool m_devModeWriteable;

  // List of other features to be supported in order to support this one.
  const QStringList m_featureDependencies;

  // If true, the feature is enabled in the Dev-Settings. If
  // `m_devModeWriteable` is false, this will always be false.
  bool m_devModeEnabled = false;

  // Determines if the feature should be available if possible
  // Otherwise it can only be reached via a dev-override
  bool m_released = false;
  // Is true if the Feature was released in this Version
  bool m_new = false;
};

#endif  // FEATURE_H
