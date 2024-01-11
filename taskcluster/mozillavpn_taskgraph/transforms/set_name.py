from taskgraph.transforms.base import TransformSequence
from taskgraph.util.dependencies import get_primary_dependency


transforms = TransformSequence()


@transforms.add
def set_name(config, tasks):
    for task in tasks:
        if config.kind == "mark-as-shipped":
            dep = get_primary_dependency(config, task)
            assert dep

            product = (
                "addons" if dep.attributes["build-type"] == "addons/opt" else "client"
            )
            task["name"] = f"mark-as-shipped-{product}"

        yield task
