/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "helpmodel.h"
#include "leakdetector.h"
#include "mozillavpn.h"

namespace {
bool s_initialized = false;

struct HelpEntry {
  HelpEntry(const QString& name, bool externalLink, bool viewLog,
            MozillaVPN::LinkType linkType)
      : m_name(name),
        m_externalLink(externalLink),
        m_viewLog(viewLog),
        m_linkType(linkType) {}

  QString m_name;
  bool m_externalLink;
  bool m_viewLog;
  MozillaVPN::LinkType m_linkType;
};

static QList<HelpEntry> s_helpEntries;

void maybeInitialize() {
  if (s_initialized) {
    return;
  }

  s_initialized = true;

  //% "View log"
  s_helpEntries.append(HelpEntry(qtTrId("help.viewLog"),
#if defined(MVPN_ANDROID) || defined(MVPN_IOS)
                                 false,
#else
                                 true,
#endif
                                 true, MozillaVPN::LinkContact));

  s_helpEntries.append(
      //% "Help Center"
      HelpEntry(qtTrId("help.helpCenter"), true, false,
                MozillaVPN::LinkHelpSupport));

  //% "Contact us"
  s_helpEntries.append(HelpEntry(qtTrId("help.contactUs"), true, false,
                                 MozillaVPN::LinkContact));
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

  MozillaVPN::instance()->openLink(entry.m_linkType);
}

void HelpModel::forEach(std::function<void(const QString&, int)>&& a_callback) {
  maybeInitialize();

  std::function<void(const QString&, int)> callback = std::move(a_callback);
  for (int i = 0; i < s_helpEntries.length(); ++i) {
    callback(s_helpEntries.at(i).m_name, i);
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
      return QVariant(s_helpEntries.at(index.row()).m_name);

    case HelpIdRole:
      return QVariant(index.row());

    case HelpExternalLinkRole:
      return QVariant(s_helpEntries.at(index.row()).m_externalLink);

    default:
      return QVariant();
  }
}
