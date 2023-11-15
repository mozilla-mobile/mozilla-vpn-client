/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef INSPECTORSERVER_H
#  define INSPECTORSERVER_H

#include <QObject>
class Inspector;

namespace InspectorServer {


class Factory {
 public:
  /**
   * @brief Factory function that creates a InspectorServer appropriate 
   * for the current platform
   * 
   * Returns nullptr if it's not possible to construct a server. 
   * 
   * @param parent 
   * @return InspectorServer* 
   */
  static QObject* create(Inspector* parent);
};

}


#endif
