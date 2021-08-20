/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "supportcategorymodel.h"
#include "leakdetector.h"
#include "logger.h"
#include "l18nstrings.h"

namespace {
Logger logger(LOG_MODEL, "SupportCategoryModel");

struct SupportCategory {
  const char* m_categoryName;
  const char* m_nameId;
};

static QList<SupportCategory> s_supportCategories;
}  // namespace

SupportCategoryModel::SupportCategoryModel() {
  MVPN_COUNT_CTOR(SupportCategoryModel);

  // Here we use the logger to force lrelease to add the category ids.

  //% "Payment and billing"
  logger.debug()
      << "Adding:"
      << L18nStrings::instance()->tr(
             L18nStrings::InAppSupportWorkflowPaymentDropdownListItem);
  s_supportCategories.append(
      SupportCategory{"payment", "support.category.paymentDropdownListItem"});

  //% "Account issues"
  logger.debug()
      << "Adding:"
      << L18nStrings::instance()->tr(
             L18nStrings::InAppSupportWorkflowAccountDropdownListItem);
  s_supportCategories.append(
      SupportCategory{"account", "support.category.accountDropdownListItem"});

  //% "Technical issues"
  logger.debug()
      << "Adding:"
      << L18nStrings::instance()->tr(
             L18nStrings::InAppSupportWorkflowTechnicaDropdownListItem);
  s_supportCategories.append(
      SupportCategory{"technical", "support.category.technicaDropdownListItem"});

  //% "Request features"
  logger.debug()
      << "Adding:"
      << L18nStrings::instance()->tr(
             L18nStrings::InAppSupportWorkflowFeatureRequestDropdownListItem);
  s_supportCategories.append(
      SupportCategory{"feature", "support.category.featureRequestDropdownListItem"});

  //% "Other"
  logger.debug() << "Adding:"
                 << L18nStrings::instance()->tr(
                        L18nStrings::InAppSupportWorkflowOtherDropdownListItem);
  s_supportCategories.append(
      SupportCategory{"other", "support.category.otherDropdownListItem"});
}

SupportCategoryModel::~SupportCategoryModel() {
  MVPN_COUNT_DTOR(SupportCategoryModel);
}

QHash<int, QByteArray> SupportCategoryModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[CategoryNameRole] = "value";
  roles[LocalizedNameRole] = "name";
  return roles;
}

int SupportCategoryModel::rowCount(const QModelIndex&) const {
  return s_supportCategories.count();
}

QVariant SupportCategoryModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  switch (role) {
    case CategoryNameRole:
      return QVariant(s_supportCategories.at(index.row()).m_categoryName);

    case LocalizedNameRole:
      return QVariant(qtTrId(s_supportCategories.at(index.row()).m_nameId));

    default:
      return QVariant();
  }
}
