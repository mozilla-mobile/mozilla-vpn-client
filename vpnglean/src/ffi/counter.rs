// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#[no_mangle]
pub unsafe extern "C" fn glean_counter_add(id: u32, amount: i32) {
    with_metric!(COUNTER_MAP, id, metric, metric.add(amount));
}

#[no_mangle]
pub unsafe extern "C" fn glean_counter_test_get_value(id: u32) -> i32 {
    with_metric!(COUNTER_MAP, id, metric, test_get!(metric))
}

#[no_mangle]
pub extern "C" fn glean_counter_test_get_error(id: u32) -> bool {
    let err = with_metric!(COUNTER_MAP, id, metric, test_get_errors!(metric));
    err.is_some()
}
