/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FEATURE_H
#define FEATURE_H

#include <QObject>
#include <QApplication>
#include "../translations/generated/l18nstrings.h"
#include "qmlengineholder.h"

class Feature : public QObject {
  Q_OBJECT

  Q_PROPERTY(QString id MEMBER m_id CONSTANT)
  Q_PROPERTY(QString name READ name CONSTANT)
  Q_PROPERTY(bool isMajorFeature MEMBER m_majorFeature CONSTANT)
  Q_PROPERTY(QString displayName READ displayName CONSTANT)
  Q_PROPERTY(QString shortDescription READ shortDescription CONSTANT)
  Q_PROPERTY(QString description READ description CONSTANT)
  Q_PROPERTY(QString imagePath MEMBER m_imagePath CONSTANT)
  Q_PROPERTY(QString iconPath MEMBER m_iconPath CONSTANT)
  Q_PROPERTY(bool isReleased MEMBER m_released CONSTANT)
  Q_PROPERTY(bool isNew MEMBER m_new CONSTANT)
  Q_PROPERTY(bool devModeWriteable MEMBER m_devModeWriteable CONSTANT)
  Q_PROPERTY(bool isSupported READ isSupported NOTIFY supportedChanged)

  // protected:
 public:
  Feature(QString id, L18nStrings::String name_id, bool isMajor,
          L18nStrings::String displayName_id, L18nStrings::String shortDesc_id,
          L18nStrings::String desc_id, QString imgPath, QString iconPath,
          QString releaseVersion, bool devModeWriteable = false);

 public:
  virtual ~Feature() = default;

  static QList<Feature*> getAll();

  // Returns a Pointer to the Feature with id, crashes client if
  // feature does not exist :)
  static const Feature* get(const QString& featureID);

  // Checks if the feature is released
  // or force enabled by the DevMode
  // returns the features checkSupportCallback otherwise.
  bool isSupported() const;

  // Returns true if it was enabled via DevMode
  bool isDevModeEnabled() const;

  QString name() const;
  QString displayName() const;
  QString description() const;
  QString shortDescription() const;

  // For use in QAbstractListModel
  static QHash<int, QByteArray> roleNames();
  QVariant data(int role) const;

  enum ModelRoles {
    RoleId,
    RoleName,
    RoleDisplayName,
    RoleDescription,
    RoleShortDescription,
    RoleImagePath,
    RoleIconPath,
    RoleReleased,
    RoleSupported,
    RoleNew,
    RoleMajor,
    RoleDevModeWriteable,
    RoleDevModeEnabled
  };

 signals:
  // Is send if the underlying factors for support changed
  // e.g Controller support level for features depending on this
  // or if the devmode enabled a feature
  void supportedChanged();
  void newChanged();

 protected:
  // Implemented by each feature.
  // Should check if the feature could be used, if released
  virtual bool checkSupportCallback() const = 0;

  // Unique Identifier of the Feature, used to Check
  // Capapbilities of the Daemon/Server or if is Force-Enabled in the Dev Menu
  const QString m_id;

  // Id for Human Readable Name of the Feature
  const L18nStrings::String m_name_id;

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

  // If true, the feature can be enabled in the Dev-Settings
  const bool m_devModeWriteable;

  // Determines if the feature should be available if possible
  // Otherwise it can only be reached via a dev-override
  bool m_released = false;
  // Is true if the Feature was released in this Version
  bool m_new = false;
};

#endif  // FEATURE_H
