# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

"""
Inherit treeherder defaults from primary dependency.
"""

from taskgraph.transforms.base import TransformSequence
from taskgraph.util.schema import resolve_keyed_by
from taskgraph.util.treeherder import inherit_treeherder_from_dep, join_symbol


transforms = TransformSequence()
@transforms.add
def resolve_treeherder_keys(config, tasks):
    keys = (
        "treeherder.job-symbol",
        "treeherder.platform",
    )
    for task in tasks:
        for key in keys:
            resolve_keyed_by(
                task,
                key,
                item_name=task["name"],
                **{
                    "build-type": task["attributes"]["build-type"],
                    "level": config.params["level"],
                },
            )
        yield task


def _get_primary_dep(config, task):
    if "primary-kind-dependency" in task["attributes"]:
        for label, dep in config.kind_dependencies_tasks.items():
            if (
                label
                == task["dependencies"][task["attributes"]["primary-kind-dependency"]]
            ):
                return dep

    raise Exception(f"Could not find primary dependency for {task['name']}!")


@transforms.add
def build_treeherder_definition(config, tasks):
    for task in tasks:
        dep = _get_primary_dep(config, task)

        task.setdefault("treeherder", {}).update(inherit_treeherder_from_dep(task, dep))
        job_group = dep.task["extra"]["treeherder"].get("groupSymbol", "?")
        job_symbol = task["treeherder"].pop("job-symbol")
        full_symbol = join_symbol(job_group, job_symbol)
        task["treeherder"]["symbol"] = full_symbol

        yield task
