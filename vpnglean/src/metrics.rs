use serde::{Deserialize, Serialize};

/// Uniquely identifies a single metric within its metric type.
#[derive(Debug, PartialEq, Eq, Hash, Copy, Clone, Deserialize, Serialize)]
#[repr(transparent)]
pub struct MetricId(pub(crate) u32);

impl MetricId {
    pub fn new(id: u32) -> Self {
        Self(id)
    }
}

impl From<u32> for MetricId {
    fn from(id: u32) -> Self {
        Self(id)
    }
}

pub mod __generated_pings {
    include!("./generated/pings.rs");
}

pub mod __generated_metrics {
    include!("./generated/metrics.rs");
}
