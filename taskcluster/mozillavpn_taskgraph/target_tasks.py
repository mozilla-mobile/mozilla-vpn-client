# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from taskgraph.target_tasks import _target_task


@_target_task("addons-target-tasks")
def addons_target_tasks(full_task_graph, parameters, graph_config):
    def filter(task):
        if (
            task.attributes.get("shipping-phase") == parameters["shipping_phase"]
            and task.attributes.get("build-type") in ("addons/opt", None)
        ):
            return True

    return [label for label, task in full_task_graph.tasks.items() if filter(task)]


@_target_task("client-target-tasks")
def client_target_tasks(full_task_graph, parameters, graph_config):
    def filter(task):
        if (
            task.attributes.get("shipping-phase") == parameters["shipping_phase"]
            and task.attributes.get("build-type") != "addons/opt"
        ):
            return True

    return [label for label, task in full_task_graph.tasks.items() if filter(task)]
