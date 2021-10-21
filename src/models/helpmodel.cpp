/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "helpmodel.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "features/featuresharelogs.h"

namespace {
bool s_initialized = false;
bool s_contactUsExternalLink = false;
Logger logger(LOG_MAIN, "HelpModel");

struct HelpEntry {
  HelpEntry(const char* nameId, bool externalLink, bool viewLog,
            MozillaVPN::LinkType linkType)
      : m_nameId(nameId),
        m_externalLink(externalLink),
        m_viewLog(viewLog),
        m_linkType(linkType) {}

  const char* m_nameId;
  bool m_externalLink;
  bool m_viewLog;
  MozillaVPN::LinkType m_linkType;
};

static QList<HelpEntry> s_helpEntries;

void maybeInitialize() {
  if (s_initialized) {
    return;
  }

  s_helpEntries.clear();

  s_initialized = true;
  // Here we use the logger to force lrelease to add the help menu Ids.

  //% "Help center"
  logger.debug() << "Adding:" << qtTrId("help.helpCenter2");
  s_helpEntries.append(
      HelpEntry("help.helpCenter2", true, false, MozillaVPN::LinkHelpSupport));

  //% "Contact us"
  logger.debug() << "Adding:" << qtTrId("help.contactUs");
  s_helpEntries.append(
      HelpEntry("help.contactUs", false, false, MozillaVPN::LinkContact));

  //% "View log"
  logger.debug() << "Adding:" << qtTrId("help.viewLog");
  s_helpEntries.append(HelpEntry("help.viewLog",
                                 FeatureShareLogs::instance()->isSupported(),
                                 true, MozillaVPN::LinkContact));
}

}  // namespace

HelpModel::HelpModel() { MVPN_COUNT_CTOR(HelpModel); }

HelpModel::~HelpModel() { MVPN_COUNT_DTOR(HelpModel); }

void HelpModel::open(int id) {
  Q_ASSERT(s_initialized);
  Q_ASSERT(id >= 0 && id < s_helpEntries.length());

  const HelpEntry& entry = s_helpEntries.at(id);
  if (entry.m_viewLog) {
    emit MozillaVPN::instance()->requestViewLogs();
    return;
  }

  if (!entry.m_externalLink && entry.m_linkType == MozillaVPN::LinkContact) {
    emit MozillaVPN::instance()->requestContactUs();
    return;
  }

  MozillaVPN::instance()->openLink(entry.m_linkType);
}

void HelpModel::forEach(std::function<void(const char*, int)>&& a_callback) {
  maybeInitialize();

  std::function<void(const char*, int)> callback = std::move(a_callback);
  for (int i = 0; i < s_helpEntries.length(); ++i) {
    callback(s_helpEntries.at(i).m_nameId, i);
  }
}

QHash<int, QByteArray> HelpModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[HelpEntryRole] = "name";
  roles[HelpIdRole] = "id";
  roles[HelpExternalLinkRole] = "externalLink";
  return roles;
}

int HelpModel::rowCount(const QModelIndex&) const {
  maybeInitialize();
  return s_helpEntries.length();
}

QVariant HelpModel::data(const QModelIndex& index, int role) const {
  maybeInitialize();

  if (!index.isValid() || index.row() >= s_helpEntries.length()) {
    return QVariant();
  }

  switch (role) {
    case HelpEntryRole:
      return QVariant(qtTrId(s_helpEntries.at(index.row()).m_nameId));

    case HelpIdRole:
      return QVariant(index.row());

    case HelpExternalLinkRole:
      return QVariant(s_helpEntries.at(index.row()).m_externalLink);

    default:
      return QVariant();
  }
}
