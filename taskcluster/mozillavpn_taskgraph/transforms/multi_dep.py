# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
"""
Apply some defaults and minor modifications to the single_dep jobs.
"""


from taskgraph.transforms.base import TransformSequence
from taskgraph.util.schema import resolve_keyed_by
from taskgraph.util.treeherder import inherit_treeherder_from_dep, join_symbol


transforms = TransformSequence()
upstream_artifacts = TransformSequence()
treeherder = TransformSequence()


@transforms.add
def build_name_and_attributes(config, tasks):
    for task in tasks:
        task["dependencies"] = {
            dep_key: dep.label for dep_key, dep in _get_all_deps(config, task).items()
        }
        primary_dep = task["primary-dependency"]
        copy_of_attributes = primary_dep.attributes.copy()
        task.setdefault("attributes", copy_of_attributes)
        # run_on_tasks_for is set as an attribute later in the pipeline
        task.setdefault("run-on-tasks-for", copy_of_attributes["run_on_tasks_for"])
        task["name"] = _get_dependent_job_name_without_its_kind(primary_dep)

        yield task


def _get_dependent_job_name_without_its_kind(dependent_job):
    return dependent_job.label[len(dependent_job.kind) + 1 :]


def _get_all_deps(config, task):
    if task.get("dependent-tasks"):
        return task["dependent-tasks"]

    if task.get("primary-dependency"):
        return {task["primary-dependency"].kind: task["primary-dependency"]}

    return {
        dep.kind: dep
        for label, dep in config.kind_dependencies_tasks.items()
        if label in task["dependencies"].values()
    }


def _get_primary_dep(config, task):
    if "primary-dependency" in task:
        return task["primary-dependency"]

    if "primary-kind-dependency" in task["attributes"]:
        for label, dep in config.kind_dependencies_tasks.items():
            if (
                label
                == task["dependencies"][task["attributes"]["primary-kind-dependency"]]
            ):
                return dep

    raise Exception(f"Could not find primary dependency for {task['name']}!")


@transforms.add
@upstream_artifacts.add
def build_upstream_artifacts(config, tasks):
    for task in tasks:
        worker_definition = {
            "upstream-artifacts": [],
        }

        for dep in _get_all_deps(config, task).values():
            paths = sorted(
                artifact["name"]
                for artifact in dep.attributes.get("release-artifacts", [])
            )

            if paths:
                worker_definition["upstream-artifacts"] = [
                    {
                        "taskId": {"task-reference": f"<{dep.kind}>"},
                        "taskType": dep.kind,
                        "paths": paths,
                    }
                ]

        task.setdefault("worker", {}).update(worker_definition)
        yield task


@transforms.add
@treeherder.add
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


@transforms.add
@treeherder.add
def build_treeherder_definition(config, tasks):
    for task in tasks:
        dep = _get_primary_dep(config, task)

        task.setdefault("treeherder", {}).update(inherit_treeherder_from_dep(task, dep))
        job_group = dep.task["extra"]["treeherder"].get("groupSymbol", "?")
        job_symbol = task["treeherder"].pop("job-symbol")
        full_symbol = join_symbol(job_group, job_symbol)
        task["treeherder"]["symbol"] = full_symbol

        yield task


@transforms.add
def remove_multi_dep_keys(config, tasks):
    for task in tasks:
        if "dependent-tasks" in task:
            del task["dependent-tasks"]

        if "primary-dependency" in task:
            del task["primary-dependency"]

        yield task
