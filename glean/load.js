/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// clang-format off
// This file exposes a list of metrics/pings to the QML/JS world.
.pragma library
// Importing Glean.js as released by the Glean team.
.import 'glean.js' as RealGlean
.import 'generated/pings.js' as Pings
.import 'generated/sample.js' as Sample
// clang-format on

// public functions

// In this way we expose the samples to what imports the module.
const sample = Sample;
const glean = RealGlean.Glean.default;

function sendPing() {
  Pings.main.submit();
}

function setUploadEnabled(enabled) {
  glean.setUploadEnabled(enabled);
}
