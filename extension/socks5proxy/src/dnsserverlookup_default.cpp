/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <qexception.h>

#include <QDnsLookup>

#include "dnsserverlookup.h"

namespace DNSServerLookup {

std::optional<QHostAddress> getLocalDNSName() { return {}; }

QFuture<QHostAddress> resolve(const QString& hostname,
                              std::optional<QHostAddress> nameServer) {
  const auto sharedPromise = std::make_shared<QPromise<QHostAddress>>();
  QPromise<QHostAddress> status{};
  auto dnsLookupAttempts = MAX_DNS_LOOKUP_ATTEMPTS;

  QDnsLookup* lookup =
      nameServer.has_value()
          ? new QDnsLookup(QDnsLookup::ANY, hostname, nameServer.value())
          : new QDnsLookup(QDnsLookup::ANY, hostname);
  QObject::connect(
      lookup, &QDnsLookup::finished,
      [dnsLookupAttempts, status = sharedPromise, lookup]() mutable {
        // Garbage collect the lookup when we're finished.
        dnsLookupAttempts--;
        if (lookup->error() != QDnsLookup::NoError) {
          status->setException(DnsFetchException{"Unknown Error"});
          status->finish();
          return;
        }
        auto guard = qScopeGuard([lookup]() {
          if (lookup->isFinished()) {
            lookup->deleteLater();
          }
        });

        // If we get a hostname record. Then DNS resolution has succeeded. and
        // we can proceed to the outbound socket setup.
        auto hostRecords = lookup->hostAddressRecords();
        if (hostRecords.length() > 0) {
          status->addResult(hostRecords.first().value());
          status->finish();
          return;
        }

        // If we have exhausted the DNS lookup attempts, then give up on DNS
        // resolution and report the host as unreachable. This safeguards us
        // from recursive DNS loops and other unresolvable situations.
        if (dnsLookupAttempts <= 0) {
          status->setException(
              DnsFetchException{"exhausted the DNS lookup attempts"});
          status->finish();
          return;
        }

        // If we got a canonical name record, restart the lookup using the
        // CNAME.
        auto cnameRecords = lookup->canonicalNameRecords();
        if (cnameRecords.length() > 0) {
          QString target = cnameRecords.first().value();
          lookup->setName(target);
          lookup->setType(QDnsLookup::ANY);
          lookup->lookup();
          return;
        }

        // Service records are not supported.
        auto serviceRecords = lookup->serviceRecords();
        if (serviceRecords.length() > 0) {
          // TODO: Not supported.
          //
          // In theory we can restart the DNS lookup with the target name, but
          // the port may have changed too and that is stored somewhere in the
          // signal binding. Service records aren't used a whole lot out in the
          // wild either.
          //
          // We can also receive more than one service record and we are
          // expected to load balance/fallback amongst them.
          status->setException(
              DnsFetchException{"DNS serviceRecords Not supported"});
          status->finish();
          return;
        }

        // If we get this far, the request didn't fail, but we also didn't get
        // any records that we could make sense of. Fallback to an explicit IPv4
        // (A) query if this originated from an ANY query.
        if (lookup->type() == QDnsLookup::ANY) {
          lookup->setType(QDnsLookup::A);
          lookup->lookup();
          return;
        }
        // Otherwise, no such host was found.
        status->setException(DnsFetchException{"no such host was found"});
        status->finish();
      });

  lookup->lookup();
  return sharedPromise->future();
}
}  // namespace DNSServerLookup
