/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commandlineparser.h"
#include "leakdetector.h"
#include "sentry.h"

#include <QtWidgets>

int main(int argc, char* argv[]) {
#ifdef MVPN_DEBUG
  LeakDetector leakDetector;
  Q_UNUSED(leakDetector);
#endif

  sentry_options_t *options = sentry_options_new();
  sentry_options_set_dsn(options, "https://6a476c1b57a34773a75c60036236a01d@o1396220.ingest.sentry.io/6719480");
  sentry_options_set_release(options, "my-project-name@2.3.12");
  //sentry_options_set_debug(options, 1);
  sentry_init(options);

  CommandLineParser clp;
  return clp.parse(argc, argv);
}
