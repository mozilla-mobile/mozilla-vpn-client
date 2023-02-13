/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "supportcategorymodel.h"

#include "i18nstrings.h"
#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger("SupportCategoryModel");

struct SupportCategory {
  const char* m_categoryName;
  I18nStrings::String m_stringId;
};

static QList<SupportCategory> s_supportCategories{
    SupportCategory{"payment",
                    I18nStrings::InAppSupportWorkflowPaymentDropdownListItem},
    SupportCategory{"account",
                    I18nStrings::InAppSupportWorkflowAccountDropdownListItem},
    SupportCategory{"technical",
                    I18nStrings::InAppSupportWorkflowTechnicaDropdownListItem},
    SupportCategory{
        "feature",
        I18nStrings::InAppSupportWorkflowFeatureRequestDropdownListItem},
    SupportCategory{"other",
                    I18nStrings::InAppSupportWorkflowOtherDropdownListItem},
};

}  // namespace

SupportCategoryModel::SupportCategoryModel() {
  MZ_COUNT_CTOR(SupportCategoryModel);
}

SupportCategoryModel::~SupportCategoryModel() {
  MZ_COUNT_DTOR(SupportCategoryModel);
}

QHash<int, QByteArray> SupportCategoryModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[CategoryNameRole] = "value";
  roles[LocalizedNameRole] = "name";
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

    case LocalizedNameRole:
      return QVariant(I18nStrings::instance()->t(
          s_supportCategories.at(index.row()).m_stringId));

    default:
      return QVariant();
  }
}
