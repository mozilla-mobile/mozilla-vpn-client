/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

use serde::{Deserialize, Serialize};

/// Uniquely identifies a single metric within its metric type.
#[derive(Debug, PartialEq, Eq, Hash, Copy, Clone, Deserialize, Serialize)]
#[repr(transparent)]
pub struct MetricId(pub(crate) u32);

impl From<u32> for MetricId {
    fn from(id: u32) -> Self {
        Self(id)
    }
}

pub mod __generated_pings {
    include!(concat!(env!("CARGO_TARGET_DIR"), "/glean/generated/pings.rs"));
}

pub mod __generated_metrics {
    include!(concat!(env!("CARGO_TARGET_DIR"), "/glean/generated/metrics.rs"));
}
