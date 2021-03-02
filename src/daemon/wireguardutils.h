/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WIREGUARDUTILS_H
#define WIREGUARDUTILS_H

#include <QCoreApplication>

constexpr const char* WG_INTERFACE = "moz0";

class WireguardUtils : public QObject {
 public:
  virtual bool interfaceExists() {
    qFatal("Have you forgotten to implement WireguardUtils::interfaceExists?");
    return false;
  };

  virtual QStringList currentInterfaces() {
    qFatal(
        "Have you forgotten to implement WireguardUtils::currentInterfaces?");
    return QStringList();
  };
};

#endif  // WIREGUARDUTILS_H
