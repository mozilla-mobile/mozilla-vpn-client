# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from taskgraph import config as tg_config
from voluptuous import Any, Optional, Required

# Extend Taskgraph's default graph_config_schema with custom fields.
tg_config.graph_config_schema = tg_config.graph_config_schema.extend(
    {
        Required("mac-signing"): {
            Required("hardened-sign-config"): [
                {
                    Optional("deep"): bool,
                    Optional("runtime"): bool,
                    Optional("force"): bool,
                    Optional("requirements"): str,
                    Optional("entitlements"): str,
                    Required("globs"): [str],
                },
            ],
        },
    },
)
