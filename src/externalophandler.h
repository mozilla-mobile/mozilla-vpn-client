/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef EXTERNALOPHANDLER_H
#define EXTERNALOPHANDLER_H

#include <QMap>
#include <QObject>

class ExternalOpHandler final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(ExternalOpHandler)

 public:
  enum Op : int {
    OpCloseEvent = 0,

    OpCustom = 1000,
  };
  Q_ENUM(Op);

  class Blocker {
   public:
    virtual bool maybeBlockRequest(int op) = 0;
  };

  void registerExternalOperation(int op, void (*callback)());

  static ExternalOpHandler* instance();

  [[nodiscard]] bool request(int op);

  void registerBlocker(Blocker* blocker);
  void unregisterBlocker(Blocker* blocker);

 private:
  explicit ExternalOpHandler(QObject* parent);
  ~ExternalOpHandler();

 private:
  QList<Blocker*> m_blockers;
  QMap<int, void (*)()> m_ops;
};

#endif  // EXTERNALOPHANDLER_H
