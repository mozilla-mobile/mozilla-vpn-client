/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// This file is the actual entry point file for Glean.js in QML, that users will interact with.
//
// I was not able to figure out a way to simply use the Webpack generated file to
// be the entry point in Qt, because of the unusual syntax allowed in Qt JavaScript.
// Thus, we compile the Glean.js library normaly into the `glean.lib.js` file and then
// we have this file which interacts opaquely with the Webpack generated one.
//
// **All functions and variables defined here are public.**

.pragma library

.import QtQuick.LocalStorage 2.15 as LocalStorage
.import "glean.lib.js" as Glean

/**
 * Initialize Glean. This method should only be called once, subsequent calls will be no-op.
 *
 * @param applicationId The application ID (will be sanitized during initialization).
 * @param uploadEnabled Determines whether telemetry is enabled.
 *        If disabled, all persisted metrics, events and queued pings (except
 *        first_run_date) are cleared.
 * @param config Glean configuration options.
 */
function initialize(applicationId, uploadEnabled, config) {
  Glean.Glean.default.initialize(applicationId, uploadEnabled, config);
}

/**
 * Sets whether upload is enabled or not.
 *
 * When uploading is disabled, metrics aren't recorded at all and no data is uploaded.
 *
 * When disabling, all pending metrics, events and queued pings are cleared.
 *
 * When enabling, the core Glean metrics are recreated.
 *
 * If the value of this flag is not actually changed, this is a no-op.
 *
 * If Glean has not been initialized yet, this is also a no-op.
 *
 * @param flag When true, enable metric collection.
 */
function setUploadEnabled(flag) {
  Glean.Glean.default.setUploadEnabled(flag);
}

/**
 * Sets the `logPings` flag.
 *
 * When this flag is `true` pings will be logged
 * to the console right before they are collected.
 *
 * @param flag Whether or not to log pings.
 */
function setLogPings(flag) {
  Glean.Glean.default.setLogPings(flag);
}

/**
 * Sets the `debugViewTag` debug option.
 *
 * When this property is set, all subsequent outgoing pings will include the `X-Debug-ID` header
 * which will redirect them to the ["Ping Debug Viewer"](https://debug-ping-preview.firebaseapp.com/).
 *
 * To unset the `debugViewTag` call `Glean.unsetDebugViewTag();
 *
 * @param value The value of the header.
 *        This value must satify the regex `^[a-zA-Z0-9-]{1,20}$` otherwise it will be ignored.
 */
function setDebugViewTag(value) {
  Glean.Glean.default.setDebugViewTag(value);
}

/**
 * Sets the `sourceTags` debug option.
 *
 * Ping tags will show in the destination datasets, after ingestion.
 *
 * Note** Setting `sourceTags` will override all previously set tags.
 *
 * To unset the `sourceTags` call `Glean.unsetSourceTags();
 *
 * @param value A vector of at most 5 valid HTTP header values.
 *        Individual tags must match the regex: "[a-zA-Z0-9-]{1,20}".
 */
function setSourceTags(value) {
  Glean.Glean.default.setSourceTags(value);
}

/**
 * Finishes executing any ongoing tasks and shuts down Glean.
 *
 * This will attempt to send pending pings before resolving.
 *
 * @returns A promise which resolves once shutdown is complete.
 */
function shutdown() {
  return Glean.shutdown();
}

const _private = Glean.Glean.default._private;
