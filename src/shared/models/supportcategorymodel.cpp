/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "supportcategorymodel.h"

#include "i18nstrings.h"
#include "leakdetector.h"
#include "logger.h"

namespace {
SupportCategoryModel* s_instance = nullptr;
Logger logger("SupportCategoryModel");

struct SupportCategory {
  const char* m_categoryName;
};

static QList<SupportCategory> s_supportCategories{
    SupportCategory{"InAppSupportWorkflowPaymentDropdownListItem"},
    SupportCategory{"InAppSupportWorkflowAccountDropdownListItem"},
    SupportCategory{"InAppSupportWorkflowTechnicaDropdownListItem"},
    SupportCategory{"InAppSupportWorkflowFeatureRequestDropdownListItem"},
    SupportCategory{"InAppSupportWorkflowOtherDropdownListItem"},
};

}  // namespace

SupportCategoryModel* SupportCategoryModel::instance() {
  if (!s_instance) {
    s_instance = new SupportCategoryModel();
  };
  return s_instance;
}

QHash<int, QByteArray> SupportCategoryModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[CategoryNameRole] = "value";
  return roles;
}

int SupportCategoryModel::rowCount(const QModelIndex&) const {
  return static_cast<int>(s_supportCategories.count());
}

QVariant SupportCategoryModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  switch (role) {
    case CategoryNameRole:
      return QVariant(s_supportCategories.at(index.row()).m_categoryName);

    default:
      return QVariant();
  }
}
