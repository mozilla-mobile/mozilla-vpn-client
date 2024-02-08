/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WEBEXTENSION_VPNADAPTER_H
#define WEBEXTENSION_VPNADAPTER_H

#include <QList>

#include "baseadapter.h"

class ServerCountryModel;

namespace WebExtension {

class VPNAdapter : public BaseAdapter {
  Q_OBJECT
 public:
  VPNAdapter(QObject* parent);
  ~VPNAdapter();

 private:
  void writeState();

  QJsonObject serializeStatus();
  void serializeServerCountry(ServerCountryModel* model, QJsonObject& obj);
};

}  // namespace WebExtension

#endif  // WEBEXTENSION_VPNADAPTER_H
