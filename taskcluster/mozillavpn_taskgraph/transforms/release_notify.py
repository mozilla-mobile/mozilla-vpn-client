# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
"""
Add dependencies to release tasks.
"""
import json
import os
from string import Template
from textwrap import dedent

from taskgraph.transforms.base import TransformSequence
from taskgraph.util.schema import resolve_keyed_by

transforms = TransformSequence()
SLACK_TEMPLATE = Template(
    """
[
    {
        "type": "context",
        "elements": [
            {
                "type": "image",
                "image_url": "https://a.slack-edge.com/production-standard-emoji-assets/14.0/google-medium/2714-fe0f.png",
                "alt_text": "notifications warning icon"
            },
            {
                "type": "mrkdwn",
                "text": "Release phase *<$taskcluster_root_url/tasks/groups/$${task.taskGroupId}|$phase>* is successful"
            }
        ]
    },
    {"type": "divider"},
    {
        "type": "context",
        "elements": [
            {"type": "mrkdwn", "text": "Build Directory:"},
            $destinations
        ]
    },
    {
        "type": "context",
        "elements": [
            {"type": "mrkdwn", "text": "Revision:"},
            {
                "type": "mrkdwn",
                "text": "<$repo/commit/$rev|$rev>"
            }
        ]
    }
]
"""
)
MATRIX_BODY_TEMPLATE = Template(
    """mozilla-vpn-client $version release phase $phase is successful

Revision: $repo/commit/$rev
Task group: $${{task.taskGroupId}}
"""
)
MATRIX_FORMATTED_TEMPLATE = Template(
    """mozilla-vpn-client $version release phase <a href="$taskcluster_root_url/tasks/groups/$${{task.taskGroupId}}">$phase</a> is successful.
<p>
Build Directory: $destinations
<p>
Revision: <a href="$repo/commit/$rev">$rev</a>
"""
)


@transforms.add
def resolve_keys(config, tasks):
    for task in tasks:
        for key in ("notify.recipients",):
            resolve_keyed_by(
                task,
                key,
                item_name=task["name"],
                **{
                    "level": config.params["level"],
                },
            )
        yield task


@transforms.add
def add_dependencies(config, tasks):
    for task in tasks:
        dependencies = {}
        # Add any kind_dependencies_tasks with matching product as dependencies
        phase = task["attributes"]["shipping-phase"]
        for dep_task in config.kind_dependencies_tasks.values():
            # only depend on tasks in the same shipping phase
            dep_phase = dep_task.attributes.get("shipping-phase")
            if dep_phase != phase:
                continue

            # Only depend on appropriate build-type
            dep_build = dep_task.attributes.get("build-type")
            if ("addons" in phase) != ("addons" in dep_build):
                continue

            dependencies[dep_task.label] = dep_task.label

        if not dependencies:
            continue

        task.setdefault("dependencies", {}).update(dependencies)
        task.setdefault("if-dependencies", []).extend(list(dependencies.keys()))
        yield task


@transforms.add
def format_message(config, tasks):
    if config.params["version"]:
        version = config.params["version"]
    elif "releases" in config.params["head_ref"]:
        version = config.params["head_ref"].split("/")[-1]
    else:  # addons are not versioned
        version = config.params["moz_build_date"]
    for task in tasks:
        slack_context = {
            "destinations": "",
            "phase": task["attributes"]["shipping-phase"],
            "repo": config.params["head_repository"],
            "rev": config.params["head_rev"],
            "taskcluster_root_url": os.environ["TASKCLUSTER_ROOT_URL"],
            "version": version,
        }
        matrix_context = slack_context.copy()

        dirs = set()
        for label, dep_task in config.kind_dependencies_tasks.items():
            if label not in task["dependencies"] or dep_task.kind != "beetmover":
                continue

            platform = dep_task.attributes["build-type"].rsplit("/")[0]
            dirs.add((dep_task.task["extra"]["release_destinations"][0], platform))

        for i, d in enumerate(dirs):
            slack_context["destinations"] += dedent(
                f"""
                {{
                    "type": "mrkdwn",
                    "text": "<{d[0]}|{d[1]}>"
                }}{"," if i != len(dirs) - 1 else ""}
            """.lstrip()
            )
            matrix_context["destinations"] += f'\t<a href="{d[0]}">{d[1]}</a>'

        slack_message = json.loads(SLACK_TEMPLATE.substitute(**slack_context))
        task.setdefault("notify", {}).setdefault("content", {}).setdefault("slack", {})[
            "blocks"
        ] = slack_message
        matrix_message = {
            "body": MATRIX_BODY_TEMPLATE.substitute(**matrix_context),
            "formatted-body": MATRIX_FORMATTED_TEMPLATE.substitute(**matrix_context),
            "format": "org.matrix.custom.html",
        }
        task["notify"]["content"]["matrix"] = matrix_message
        yield task
