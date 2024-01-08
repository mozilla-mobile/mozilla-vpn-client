/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FEATUREMODEL_H
#define FEATUREMODEL_H

#include <QAbstractListModel>
#include <QObject>
#include <QPair>

class Feature;

class FeatureModel final : public QAbstractListModel {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(FeatureModel)

 private:
  FeatureModel() = default;

 public:
  enum ModelRoles {
    FeatureRole = Qt::UserRole + 1,
  };

  static FeatureModel* instance();

  void updateFeatureList(const QByteArray& data);

  // QAbstractListModel methods
  QHash<int, QByteArray> roleNames() const override;
  int rowCount(const QModelIndex&) const override;
  QVariant data(const QModelIndex& index, int role) const override;

  Q_INVOKABLE void toggle(const QString& feature);
  Q_INVOKABLE QObject* get(const QString& feature);

 private:
  /**
   * @brief Parses an object with a list of features to enable / disable.
   *
   * The expected format of the object is:
   *
   * ```json
   * {
   *   "featureToEnable": true,
   *   "featureToDisable": false,
   * }
   * ```
   *
   * Features _not_ in the object are ignored by this function.
   *
   * @param features
   * @return QPair<QStringList, QStringList> Returns the a list of the feature
   * to enabled and disable respectively.
   */
  static QPair<QStringList, QStringList> parseFeatures(
      const QJsonValue& features);

  /**
   * @brief Parses an object with a list of experimental features to enable /
   * disable and the values of related experimental feature settings.
   *
   * The expected format of the object is:
   *
   * ```json
   * {
   *   "experimentalFeatureToEnable": {
   *      "aFeatureSetting": "aha!"
   *   },
   *   "anotherExperimentalFeatureToEnable": {
   *      "anotherFeatureSetting": 42
   *   },
   * }
   * ```
   *
   * Experimental features _not_ in the object are disabled.
   *
   *
   * @param features
   * @return QPair<QStringList, QStringList> Returns the a list of the feature
   * to enabled and disable respectively.
   */
  static QPair<QStringList, QStringList> parseExperimentalFeatures(
      const QJsonValue& experimentalFeatures);
};

#endif  // FEATUREMODEL_H
