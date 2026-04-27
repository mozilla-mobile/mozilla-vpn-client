/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FEATUREMODEL_H
#define FEATUREMODEL_H

#include <QAbstractListModel>
#include <QHash>
#include <array>

#include "featureproxy.h"
#include "features.h"

using namespace Feature;

// Features the FeatureModel exposes: anything QML/UI reads, plus anything the
// Guardian override API or dev menu needs to flip. Features outside this list
// are unreachable via get()/isEnabledById()/updateFeatureList().
inline const AnyFeature s_exposedFeatures[] = {
    ref(accountDeletion),
    ref(addonSignature),
    ref(alwaysPort53),
    ref(annualUpgrade),
    ref(appReview),
    ref(captivePortal),
    ref(checkConnectivityOnActivation),
    ref(customDNS),
    ref(enableUpdateServer),
    ref(factoryReset),
    ref(freeTrial),
    ref(inAppAccountCreate),
    ref(inAppAuthentication),
    ref(multiHop),
    ref(notificationControl),
    ref(recommendedServers),
    ref(replacerAddon),
    ref(serverUnavailableNotification),
    ref(shareLogs),
    ref(splitTunnel),
    ref(startOnBoot),
    ref(subscriptionManagement),
    ref(themeSelectionIncludesAutomatic),
    ref(unsecuredNetworkNotification),
    ref(webExtension),
    ref(webPurchase),
};

class FeatureModel final : public QAbstractListModel {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(FeatureModel)

 public:
  enum ModelRoles {
    FeatureRole = Qt::UserRole + 1,
  };

  static FeatureModel* instance();

  void updateFeatureList(const QByteArray& data);

  QHash<int, QByteArray> roleNames() const override;
  int rowCount(const QModelIndex&) const override;
  QVariant data(const QModelIndex& index, int role) const override;

  Q_INVOKABLE void toggle(const QString& feature);
  Q_INVOKABLE QObject* get(const QString& feature);

  bool isEnabledById(const QString& id) const;

#ifdef UNIT_TEST
  static void testCleanup();
#endif

 private:
  FeatureModel();
  void initialize();

  FeatureProxy* proxyForId(const QString& id);

  static QHash<QString, bool> parseFeatures(const QByteArray& data,
                                            bool acceptExperiments);

  bool m_initialized = false;
  std::array<FeatureProxy, std::size(s_exposedFeatures)> m_proxies;
};

#endif  // FEATUREMODEL_H
