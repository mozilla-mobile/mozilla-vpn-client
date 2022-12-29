/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "feedbackcategorymodel.h"

#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger("FeedbackCategoryModel");

struct FeedbackCategory {
  const char* m_categoryName;
  const char* m_nameId;
};

static QList<FeedbackCategory> s_feedbackCategories;
}  // namespace

FeedbackCategoryModel::FeedbackCategoryModel() {
  MZ_COUNT_CTOR(FeedbackCategoryModel);

  // Here we use the logger to force lrelease to add the category ids.

  //% "Product Bugs/Errors"
  logger.debug() << "Adding:" << qtTrId("feedback.category.bugError");
  s_feedbackCategories.append(
      FeedbackCategory{"bug", "feedback.category.bugError"});

  //% "Network Connection/Speed"
  logger.debug() << "Adding:" << qtTrId("feedback.category.networkSpeed");
  s_feedbackCategories.append(
      FeedbackCategory{"connection_speed", "feedback.category.networkSpeed"});

  //% "Product Quality"
  logger.debug() << "Adding:" << qtTrId("feedback.category.productQuality");
  s_feedbackCategories.append(
      FeedbackCategory{"quality", "feedback.category.productQuality"});

  //% "Access to service"
  logger.debug() << "Adding:" << qtTrId("feedback.category.accessToService");
  s_feedbackCategories.append(FeedbackCategory{
      "access_to_service", "feedback.category.accessToService"});

  //% "Compatibility"
  logger.debug() << "Adding:" << qtTrId("feedback.category.compatibility");
  s_feedbackCategories.append(
      FeedbackCategory{"compatibility", "feedback.category.compatibility"});

  //% "Ease of Use"
  logger.debug() << "Adding:" << qtTrId("feedback.category.easeToUse");
  s_feedbackCategories.append(
      FeedbackCategory{"ease_of_use", "feedback.category.easeToUse"});

  //% "Other"
  logger.debug() << "Adding:" << qtTrId("feedback.category.other");
  s_feedbackCategories.append(
      FeedbackCategory{"other", "feedback.category.other"});
}

FeedbackCategoryModel::~FeedbackCategoryModel() {
  MZ_COUNT_DTOR(FeedbackCategoryModel);
}

QHash<int, QByteArray> FeedbackCategoryModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[CategoryNameRole] = "value";
  roles[LocalizedNameRole] = "name";
  return roles;
}

int FeedbackCategoryModel::rowCount(const QModelIndex&) const {
  return static_cast<int>(s_feedbackCategories.count());
}

QVariant FeedbackCategoryModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  switch (role) {
    case CategoryNameRole:
      return QVariant(s_feedbackCategories.at(index.row()).m_categoryName);

    case LocalizedNameRole:
      return QVariant(qtTrId(s_feedbackCategories.at(index.row()).m_nameId));

    default:
      return QVariant();
  }
}
