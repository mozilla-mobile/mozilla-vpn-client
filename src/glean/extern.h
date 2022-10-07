/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef GLEAN_FFI_GENERATED_H
#define GLEAN_FFI_GENERATED_H


/* Generated with cbindgen:0.24.3 */

/* DO NOT MODIFY THIS MANUALLY! This file was generated using cbindgen. */

#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <new>

using RawStringArray = const char*const *;

extern "C" {

void glean_initialize(bool is_telemetry_enabled);

void glean_counter_add(uint32_t id, int32_t amount);

int32_t glean_counter_test_get_value(uint32_t id);

bool glean_counter_test_get_error(uint32_t id);

void glean_event_record_no_extra(uint32_t id);

void glean_event_record(uint32_t id,
                        RawStringArray extra_keys,
                        RawStringArray extra_values,
                        int32_t extras_len);

bool glean_event_test_get_error(uint32_t id);

void glean_submit_ping_by_id(uint32_t id);

} // extern "C"

#endif // GLEAN_FFI_GENERATED_H
