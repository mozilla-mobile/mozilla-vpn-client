# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from taskgraph.target_tasks import _target_task


@_target_task("promote-addons")
def target_tasks_promote_addons(full_task_graph, parameters, graph_config):
    def filter(task):
        if (
            task.attributes.get("shipping-phase") == "promote-addons"
            and task.attributes.get("build-type") == "addons/opt"
        ):
            return True

    return [label for label, task in full_task_graph.tasks.items() if filter(task)]


@_target_task("ship-addons")
def target_tasks_ship_addons(full_task_graph, parameters, graph_config):
    filtered_for_candidates = target_tasks_promote_addons(
        full_task_graph,
        parameters,
        graph_config,
    )

    def filter(task):
        # Include promotion tasks; these will be optimized out
        if task.label in filtered_for_candidates:
            return True

        if (
            task.attributes.get("shipping-phase") == "ship-addons"
            and task.attributes.get("build-type") == "addons/opt"
        ):
            return True

    return [label for label, task in full_task_graph.tasks.items() if filter(task)]


@_target_task("promote-client")
def target_tasks_promote_client(full_task_graph, parameters, graph_config):
    def filter(task):
        if (
            task.attributes.get("shipping-phase") == "promote-client"
            and task.attributes.get("build-type") != "addons/opt"
        ):
            return True

    return [label for label, task in full_task_graph.tasks.items() if filter(task)]


@_target_task("ship-client")
def target_tasks_ship_client(full_task_graph, parameters, graph_config):
    filtered_for_candidates = target_tasks_promote_client(
        full_task_graph,
        parameters,
        graph_config,
    )

    def filter(task):
        # Include promotion tasks; these will be optimized out
        if task.label in filtered_for_candidates:
            return True

        if (
            task.attributes.get("shipping-phase") == "ship-client"
            and task.attributes.get("build-type") != "addons/opt"
        ):
            return True

    return [label for label, task in full_task_graph.tasks.items() if filter(task)]
