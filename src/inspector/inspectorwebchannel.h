/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef INSPECTOR_WEBCHANNEL_H

#define INSPECTOR_WEBCHANNEL_H

#include <QObject>
#include <QWebChannel>
#include <QHostAddress>
#include <QWebSocket>

/**
 * @brief - Exposes the Client via Webchannels 
 * 
 */
class InspectorWebChannel : public QObject {

  Q_OBJECT
  Q_DISABLE_COPY_MOVE(InspectorWebChannel)

 public:
  ~InspectorWebChannel();
  static InspectorWebChannel* instance();

    // Attach the Websocket connection to the WebChannel handler.
  void attach(QWebSocket* target);


 private:
    explicit InspectorWebChannel(QObject* parent);


    QWebChannel m_channel; 
};


#endif //INSPECTOR_WEBCHANNEL_H