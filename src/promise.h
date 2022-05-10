/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PROMISE_H
#define PROMISE_H

#include <QObject>
#include <QVariant>

#include <any>
#include <functional>
#include <iostream>

class Promise final : public QObject {
  Q_OBJECT

 public:
  class Resolver final : public QObject {
   public:
    Resolver(Promise* promise) : m_promise(promise) {}

    template <typename... A>
    void resolve(A&&... args) {
      QVariantList list;
      (
          [&](auto& input) {
            list.append(QVariant::fromValue((input)));
            ;
          }(args),
          ...);

      m_promise->resolve(list);
    }

    template <typename... A>
    void reject(A&&... args) {
      QVariantList list;
      (
          [&](auto& input) {
            list.append(QVariant::fromValue((input)));
            ;
          }(args),
          ...);

      m_promise->reject(list);
    }

   private:
    Promise* m_promise = nullptr;
  };

  enum State {
    Uncompleted,
    Resolved,
    Rejected,
  } m_state = Uncompleted;

  Promise(QObject* parent,
          const std::function<void(Resolver* resolver)>& initFunc)
      : QObject(parent), m_resolver(this) {
    initFunc(&m_resolver);
  }

  State state() const { return m_state; }

  QVariantList resolveValues() const { return m_resolveValues; }
  QVariantList rejectValues() const { return m_rejectValues; }

  void start(State state, const QVariantList& values) {
    switch (state) {
      case Resolved: {
        if (m_resolveFunc) {
          m_resolveFunc(&m_resolver, values);
        } else {
          resolve(values);
        }
      } break;

      case Rejected: {
        if (m_rejectFunc) {
          m_rejectFunc(&m_resolver, values);
        } else {
          reject(values);
        }
      } break;

      case Uncompleted:
        break;
    }
  }

  Promise* then(
      const std::function<void(Resolver* resolver, const QVariantList& values)>&
          resolveFunc) {
    return then(resolveFunc, nullptr);
  }

  Promise* then(
      const std::function<void(Resolver* resolver, const QVariantList& values)>&
          resolveFunc,
      const std::function<void(Resolver* resolver, const QVariantList& values)>&
          rejectFunc) {
    Promise* next = new Promise(this);
    next->m_resolveFunc = resolveFunc;
    next->m_rejectFunc = rejectFunc;

    switch (m_state) {
      case Uncompleted:
        m_nextPromises.push_back(next);
        break;

      case Resolved:
        next->start(m_state, m_resolveValues);
        break;

      case Rejected:
        next->start(m_state, m_rejectValues);
        break;
    }

    return next;
  }

  Promise* _catch(
      const std::function<void(Resolver* resolver, const QVariantList& values)>&
          rejectFunc) {
    return then(nullptr, rejectFunc);
  }

 signals:
  void resolved();
  void rejected();

 private:
  explicit Promise(QObject* parent) : QObject(parent), m_resolver(this) {}

  void resolve(const QVariantList& values) {
    if (m_state != Uncompleted) {
      return;
    }

    m_state = Resolved;
    m_resolveValues = values;

    emit resolved();

    for (Promise* p : m_nextPromises) {
      p->start(m_state, m_resolveValues);
    }
  }

  void reject(const QVariantList& values) {
    if (m_state != Uncompleted) {
      return;
    }

    m_state = Rejected;
    m_rejectValues = values;

    emit rejected();

    for (Promise* p : m_nextPromises) {
      p->start(m_state, m_rejectValues);
    }
  }

  std::function<void(Resolver*, const QVariantList&)> m_resolveFunc;
  std::function<void(Resolver*, const QVariantList&)> m_rejectFunc;

  QVariantList m_resolveValues;
  QVariantList m_rejectValues;

  std::vector<Promise*> m_nextPromises;
  Resolver m_resolver;
};

#endif  // PROMISE_H
