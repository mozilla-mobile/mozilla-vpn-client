/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "navigationbarmodel.h"

#include <QCoreApplication>
#include <QFile>

#include "logger.h"
#include "navigator.h"
#include "utils/leakdetector/leakdetector.h"

namespace {
Logger logger("NavigationBarModel");
}

// static
NavigationBarModel* NavigationBarModel::instance() {
  static NavigationBarModel* s_instance = nullptr;
  if (!s_instance) {
    s_instance = new NavigationBarModel(qApp);
  }
  return s_instance;
}

NavigationBarModel::NavigationBarModel(QObject* parent)
    : QAbstractListModel(parent) {
  MZ_COUNT_CTOR(NavigationBarModel);

  connect(Navigator::instance(), &Navigator::currentComponentChanged, this,
          &NavigationBarModel::resetButtonSelection);
}

NavigationBarModel::~NavigationBarModel() { MZ_COUNT_DTOR(NavigationBarModel); }

QHash<int, QByteArray> NavigationBarModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[ButtonRole] = "buttonItem";
  return roles;
}

int NavigationBarModel::rowCount(const QModelIndex&) const {
  return static_cast<int>(m_buttons.count());
}

QVariant NavigationBarModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  return QVariant::fromValue(m_buttons.at(index.row()));
}

void NavigationBarModel::appendButton(NavigationBarButton* button) {
  m_buttons.append(button);
  emit modelChanged();
}

void NavigationBarModel::resetButtonSelection() {
  logger.debug() << "Reset button selection";

  int screen = Navigator::instance()->currentScreen();
  NavigationBarButton* selectedButton = nullptr;
  for (NavigationBarButton* button : m_buttons) {
    if (button->screen() == screen) {
      selectedButton = button;
      break;
    }
  }

  if (!selectedButton) {
    return;
  }

  for (NavigationBarButton* button : m_buttons) {
    button->setChecked(button == selectedButton);
  }
}
