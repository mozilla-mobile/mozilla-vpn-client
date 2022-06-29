/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef EXTERNALOPHANDLER_H
#define EXTERNALOPHANDLER_H

#include <QObject>

class ExternalOpHandler final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(ExternalOpHandler)

 public:
  enum Op {
    OpActivate,
    OpContactUs,
    OpDeactivate,
    OpNotificationClicked,
    OpSettings,
    OpQuit,
    OpViewLogs,
  };

  static ExternalOpHandler* instance();

  void request(Op op);
  void requestOpActivate() { return request(OpActivate); }
  void requestOpDeactivate() { return request(OpDeactivate); }
  void requestOpSettings() { return request(OpSettings); }
  void requestOpQuit() { return request(OpQuit); }

 signals:
  void requestReceived(Op op);

 private:
  explicit ExternalOpHandler(QObject* parent);
  ~ExternalOpHandler();
};

#endif  // EXTERNALOPHANDLER_H
