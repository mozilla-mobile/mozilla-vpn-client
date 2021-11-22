/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SINGLETON_H
#define SINGLETON_H

#include <Qobject>

template <typename T>
class Singleton {
 public:
  static T* instance() {
    static T* instance = new T{SingleToken{}};
    return instance;
  }

  Singleton(const Singleton&) = delete;
  Singleton& operator=(const Singleton) = delete;
  Singleton(Singleton&&) = delete;
  Singleton& operator=(Singleton&&) = delete;

 protected:
  struct SingleToken {};
  Singleton() = default;
};
#endif  // SINGLETON_H
