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
    OpAbout,
    OpActivate,
    OpCloseEvent,
    OpDeactivate,
    OpGetHelp,
    OpNotificationClicked,
    OpSettings,
    OpQuit,
  };
  Q_ENUM(Op);

  class Blocker {
   public:
    virtual bool maybeBlockRequest(Op op) = 0;
  };

  static ExternalOpHandler* instance();

  void request(Op op);

  void registerBlocker(Blocker* blocker);
  void unregisterBlocker(Blocker* blocker);

 private:
  explicit ExternalOpHandler(QObject* parent);
  ~ExternalOpHandler();

 private:
  QList<Blocker*> m_blockers;
};

#endif  // EXTERNALOPHANDLER_H
