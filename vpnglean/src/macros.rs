// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

//! Helper macros for implementing the FFI API for metric types.

/// Get a metric object by ID from the corresponding map, then
/// execute the provided closure with it.
///
/// # Arguments
///
/// * `$map` - The name of the hash map within `metrics::__generated_metrics`
///            (or `factory::__jog_metric_maps`)
///            as __generated_metrics by glean_parser.
/// * `$id`  - The ID of the metric to get.
/// * `$m`   - The identifier to use for the retrieved metric.
///            The expression `$f` can use this identifier.
/// * `$f`   - The expression to execute with the retrieved metric `$m`.
macro_rules! with_metric {
    ($map:ident, $id:ident, $m:ident, $f:expr) => {
        just_with_metric!($map, $id, $m, $f)
    };
}

/// Get a metric object by id from the corresponding map, then
/// execute the provided closure with it.
///
/// Ignores the possibility that the $id might be for a labeled submetric.
///
/// # Arguments
///
/// * `$map` - The name of the hash map within `metrics::__generated_metrics`
///            (or `factory::__jog_metric_maps`)
///            as __generated_metrics by glean_parser.
/// * `$id`  - The ID of the metric to get.
/// * `$m`   - The identifier to use for the retrieved metric.
///            The expression `$f` can use this identifier.
/// * `$f`   - The expression to execute with the retrieved metric `$m`.
macro_rules! just_with_metric {
    ($map:ident, $id:ident, $m:ident, $f:expr) => {
        match $crate::metrics::__generated_metrics::$map.get(&$id.into()) {
            Some($m) => $f,
            None => panic!("No metric for id {}", $id),
        }
    };
}

/// Get the currently stored value for the given metric.
///
/// # Arguments
///
/// * `$metric`  - The metric to test.
/// * `$storage` - the storage name to look into.
macro_rules! test_get {
    ($metric:ident) => {{
        $metric.test_get_value(None).unwrap()
    }};
}

/// Check the provided metric in the provided storage for errors.
/// On finding one, return an error string.
///
/// # Arguments
///
/// * `$metric`  - The metric to test.
macro_rules! test_get_errors {
    ($metric:path) => {{
        let error_types = [
            glean::ErrorType::InvalidValue,
            glean::ErrorType::InvalidLabel,
            glean::ErrorType::InvalidState,
            glean::ErrorType::InvalidOverflow,
        ];
        let mut error_str = None;
        for &error_type in error_types.iter() {
            let num_errors = $metric.test_get_num_recorded_errors(error_type);
            if num_errors > 0 {
                error_str = Some(format!(
                    "Metric had {} error(s) of type {}!",
                    num_errors,
                    error_type.as_str()
                ));
                break;
            }
        }
        error_str
    }};
}
