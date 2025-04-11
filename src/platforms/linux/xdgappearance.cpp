/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "xdgappearance.h"

#include "leakdetector.h"
#include "logger.h"

#include <QDBusReply>

constexpr const char* XDG_NAMESPACE_APPEARANCE = "org.freedesktop.appearance";

namespace {
Logger logger("XdgAppearance");
}  // namespace

XdgAppearance::XdgAppearance(QObject* parent)
    : XdgPortal(XDG_PORTAL_SETTINGS, parent) {
  MZ_COUNT_CTOR(XdgAppearance);

  connect(&m_portal, SIGNAL(SettingChanged(QString, QString, QDBusVariant)),
          this, SLOT(xdgSettingChanged(QString, QString, QDBusVariant)));
}

XdgAppearance::~XdgAppearance() {
  MZ_COUNT_DTOR(XdgAppearance);
}

uint XdgAppearance::colorScheme() { return readValueUint("color-scheme"); }

uint XdgAppearance::accentColor() { return readValueUint("accent-color"); }

uint XdgAppearance::contrast() { return readValueUint("contrast"); }

uint XdgAppearance::readValueUint(const QString& name) {
  QList<QVariant> args;
  args.append(QVariant(XDG_NAMESPACE_APPEARANCE));
  args.append(QVariant(name));
  QDBusReply<QDBusVariant> reply =
      m_portal.callWithArgumentList(QDBus::Block, "Read", args);
  if (!reply.isValid()) {
    logger.error() << "Failed to read" << name << reply.error().message();
    return 0;
  }

  QDBusVariant result = qvariant_cast<QDBusVariant>(reply.value().variant());
  return result.variant().toUInt();
}

void XdgAppearance::xdgSettingChanged(const QString& ns, const QString& key,
                                      const QDBusVariant& value) {
  if (ns != XDG_NAMESPACE_APPEARANCE) {
    return;
  }
  QVariant qv = value.variant();
  logger.debug() << "Appearance changed" <<key << "->" << qv.toString();
  if (key == "color-scheme") {
    emit colorSchemeChanged();
  } else if (key == "accent-color") {
    emit accentColorChanged();
  } else if (key == "contrast") {
    emit contrastChanged();
  }
}
