/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WEBEXTENSIONADAPTER_H
#define WEBEXTENSIONADAPTER_H

#include <QList>

#include "webextension/baseadapter.h"

class ServerCountryModel;

/**
 * @brief This Class exposes the API available for
 * a Connected WebExtension
 *
 * All available commands are defined in m_commands
 */
class WebExtensionAdapter : public WebExtension::BaseAdapter {
  Q_OBJECT
 public:
  WebExtensionAdapter(QObject* parent);
  ~WebExtensionAdapter();

 private:
  void writeState();
  QJsonObject serializeStatus();
  void serializeServerCountry(ServerCountryModel* model, QJsonObject& obj);
};

#endif  // WEBEXTENSIONADAPTER_H
