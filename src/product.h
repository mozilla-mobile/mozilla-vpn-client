/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PRODUCT_H
#define PRODUCT_H

#include <QObject>

class Product final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Product);

 public:
  explicit Product(QObject* parent);
  ~Product();
};

#endif  // PRODUCT_H
