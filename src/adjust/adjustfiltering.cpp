/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "adjustfiltering.h"

#include <algorithm>

namespace {
AdjustFiltering* s_instance = nullptr;
}

// static
AdjustFiltering* AdjustFiltering::instance() {
  if (!s_instance) {
    s_instance = new AdjustFiltering();
  };
  return s_instance;
}

void AdjustFiltering::allowField(const QString& field) {
  allowList.insert(field);

  if (denyList.contains(field)) {
    denyList.remove(field);
  }

  if (mirrorList.contains(field)) {
    mirrorList.remove(field);
  }
}

void AdjustFiltering::denyField(const QString& field, const QString& param) {
  denyList.insert(field, param);

  if (allowList.contains(field)) {
    allowList.remove(field);
  }

  if (mirrorList.contains(field)) {
    mirrorList.remove(field);
  }
}

void AdjustFiltering::mirrorField(const QString& field,
                                  const MirrorParam& param) {
  mirrorList.insert(field, param);

  if (allowList.contains(field)) {
    allowList.remove(field);
  }

  if (denyList.contains(field)) {
    denyList.remove(field);
  }
}

QUrlQuery AdjustFiltering::filterParameters(QUrlQuery& parameters,
                                            QStringList& unknownParameters) {
  QUrlQuery newParameters;

  for (const QPair<QString, QString>& parameter : parameters.queryItems()) {
    if (allowList.contains(parameter.first)) {
      newParameters.addQueryItem(parameter.first, parameter.second);
      continue;
    }

    if (denyList.contains(parameter.first)) {
      newParameters.addQueryItem(parameter.first,
                                 denyList.value(parameter.first));
      continue;
    }

    if (mirrorList.contains(parameter.first)) {
      const MirrorParam& mirrorParam = mirrorList.value(parameter.first);
      bool found = false;

      if (!allowList.contains(mirrorParam.m_mirrorParamName)) {
        break;
      }

      for (const QPair<QString, QString>& otherParameter :
           parameters.queryItems()) {
        if (mirrorParam.m_mirrorParamName == otherParameter.first) {
          newParameters.addQueryItem(parameter.first, otherParameter.second);
          found = true;
          break;
        }
      }

      if (!found) {
        newParameters.addQueryItem(parameter.first, mirrorParam.m_defaultValue);
      }

      continue;
    }

    unknownParameters.append(parameter.first);
  }

  return newParameters;
}
