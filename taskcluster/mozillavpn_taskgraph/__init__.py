# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from importlib import import_module

from taskgraph.util import schema
from mozilla_taskgraph import register as register_mozilla_taskgraph
from mozilla_taskgraph.actions import enable_action

schema.EXCEPTED_SCHEMA_IDENTIFIERS.append(
    lambda path: any(
        exc in path for exc in ("['entitlementsUrl']", "['loginItemsEntitlementsUrl']")
    )
)


RELEASE_PROMOTION_PROJECTS = (
    "https://github.com/mozilla-mobile/mozilla-vpn-client",
    "https://github.com/mozilla-releng/staging-mozilla-vpn-client",
)


def is_relpro_available(parameters):
    return parameters["head_repository"] in RELEASE_PROMOTION_PROJECTS


def register(graph_config):
    """Setup for task generation."""
    # Setup mozilla-taskgraph
    register_mozilla_taskgraph(graph_config)
    # Enable the relpro action from mozilla-taskgraph
    enable_action("release-promotion", available=is_relpro_available)

    # Import sibling modules, triggering decorators in the process
    _import_modules(
        [
            "job",
            "parameters",
            "target_tasks",
            "worker_types",
        ]
    )


def _import_modules(modules):
    for module in modules:
        import_module(".{}".format(module), package=__name__)
