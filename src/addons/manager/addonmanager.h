/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDONMANAGER_H
#define ADDONMANAGER_H

#include <QAbstractListModel>
#include <QJSValue>
#include <QMap>

#include "addonindex.h"
#include "addons/addon.h"  // required for the signal

class AddonManager final : public QAbstractListModel {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AddonManager)

  Q_PROPERTY(
      bool loadCompleted MEMBER m_loadCompleted NOTIFY loadCompletedChanged)
  Q_PROPERTY(int count READ count NOTIFY countChanged)

 public:
  static QString addonServerAddress();

  static bool signatureVerificationNeeded();

  /**
   * This is mainly for testing. All the addons are unloaded and the cache is
   * reset.
   */
  void reset();

  /**
   * Force a fetch of the addon index manifest instead of waiting for the
   * normal scheduling.
   */
  void fetch() { refreshAddons(); }

  Q_INVOKABLE Addon* pick(QJSValue filterCallback) const;

  Q_INVOKABLE QJSValue reduce(QJSValue callback, QJSValue initialValue) const;

  Q_INVOKABLE void reinstateMessages() const;

  enum ModelRoles {
    AddonRole = Qt::UserRole + 1,
  };

  static AddonManager* instance();

  ~AddonManager();

  void storeAndLoadAddon(const QByteArray& addonData, const QString& addonId,
                         const QByteArray& sha256);

  void retranslate();

  void forEach(std::function<void(Addon* addon)>&& callback);

  /**
   * The operation of updating the index is completed. `status` indicates the
   * result of the operation. If `status` is true, the operation succeeded and
   * `index` and `indexSignature` can be used.
   */
  void updateIndex(bool status, const QByteArray& index = QByteArray(),
                   const QByteArray& indexSignature = QByteArray());

  int count() const;

 private:
  explicit AddonManager(QObject* parent);

  void initialize();

  void updateAddonsList(bool status, QList<AddonData> addons);

  void refreshAddons();

  bool validateAndLoad(const QString& addonId, const QByteArray& sha256,
                       bool checkSha256 = true);

  static void removeAddon(const QString& addonId);

  static QString mountPath(const QString& addonId);

  bool loadManifest(const QString& addonManifestFileName);

  void loadCompleted();

  void unload(const QString& addonId);

  // QAbstractListModel methods

  QHash<int, QByteArray> roleNames() const override;

  int rowCount(const QModelIndex&) const override;

  QVariant data(const QModelIndex& index, int role) const override;

 signals:
  void loadCompletedChanged();
  void countChanged();

  void addonCreated(Addon* addon);

 private:
  QMap<QString, AddonData> m_addons;

  bool m_loadCompleted = false;

  AddonIndex m_addonIndex;
  AddonDirectory m_addonDirectory;
};

#endif  // ADDONMANAGER_H
