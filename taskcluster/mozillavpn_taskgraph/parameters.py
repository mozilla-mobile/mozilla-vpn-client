# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import os
from typing import Annotated, Optional

import msgspec

from taskgraph.parameters import extend_parameters_schema
from taskgraph.util.schema import Schema

def get_defaults(repo_root):
    return {
        "pull_request_number": None,
        "version": "",
    }

class MozillaVpnParametersSchema(Schema, kw_only=True, rename=None):
    pull_request_number: Optional[Annotated[int, msgspec.Meta(ge=1)]]
    version: str

extend_parameters_schema(MozillaVpnParametersSchema, defaults_fn=get_defaults)

def get_decision_parameters(graph_config, parameters):
    head_tag = parameters["head_tag"]
    parameters["version"] = head_tag[1:] if head_tag else ""

    pr_number = os.environ.get("MOZILLAVPN_PULL_REQUEST_NUMBER", None)
    parameters["pull_request_number"] = None if pr_number is None else int(pr_number)
