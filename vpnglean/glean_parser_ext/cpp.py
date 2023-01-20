# -*- coding: utf-8 -*-

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# !IMPORTANT!
#
# This file is copy-pasted from https://searchfox.org/mozilla-central/source/toolkit/components/glean/build_scripts/glean_parser_ext/cpp.py
# with minimal changes for the `extra_type_name` function to return Qt types instead of mozilla-central types.

"""
Outputter to generate C++ code for metrics.
"""

import json

import jinja2
from glean_parser import util
from util import generate_metric_ids, generate_ping_ids, get_metrics


def cpp_datatypes_filter(value):
    """
    A Jinja2 filter that renders C++ literals.

    Based on Python's JSONEncoder, but overrides:
      - lists to array literals {}
      - strings to "value"
    """

    class CppEncoder(json.JSONEncoder):
        def iterencode(self, value):
            if isinstance(value, list):
                yield "{"
                first = True
                for subvalue in list(value):
                    if not first:
                        yield ", "
                    yield from self.iterencode(subvalue)
                    first = False
                yield "}"
            elif isinstance(value, str):
                yield '"' + value + '"'
            else:
                yield from super().iterencode(value)

    return "".join(CppEncoder().iterencode(value))


def type_name(obj):
    """
    Returns the C++ type to use for a given metric object.
    """

    if getattr(obj, "labeled", False):
        class_name = util.Camelize(obj.type[8:])  # strips "labeled_" off the front.
        return "Labeled<impl::{}Metric>".format(class_name)
    return util.Camelize(obj.type) + "Metric"


def extra_type_name(typ: str) -> str:
    """
    Returns the corresponding C++ type for event's extra key types.
    """

    if typ == "boolean":
        return "bool"
    elif typ == "string":
        return "QString"
    elif typ == "quantity":
        return "int"
    else:
        # Note: glean_parser will error before we ever get to this line
        # in an unsupported extra key type is used.
        return "UNSUPPORTED"

def output_header(objs, output_fd, options={}):
    """
    Given a tree of objects, output C++ code to the file-like object `output_fd`.

    :param objs: A tree of objects (metrics and pings) as returned from
    `parser.parse_objects`.
    :param output_fd: Writeable file to write the output to.
    :param options: options dictionary.
    """

    # Monkeypatch a util.snake_case function for the templates to use
    util.snake_case = lambda value: value.replace(".", "_").replace("-", "_")
    # Monkeypatch util.get_jinja2_template to find templates nearby

    def get_local_template(template_name, filters=()):
        env = jinja2.Environment(
            loader=jinja2.PackageLoader("cpp", "templates"),
            trim_blocks=True,
            lstrip_blocks=True,
        )
        env.filters["camelize"] = util.camelize
        env.filters["Camelize"] = util.Camelize
        for filter_name, filter_func in filters:
            env.filters[filter_name] = filter_func
        return env.get_template(template_name)

    util.get_jinja2_template = get_local_template
    get_metric_id = generate_metric_ids(objs)
    get_ping_id = generate_ping_ids(objs)

    if "pings" in objs:
        template_filename = "cpp_pings_header.jinja2"
        if objs.get("tags"):
            del objs["tags"]
    else:
        template_filename = "cpp_metrics_header.jinja2"
        objs = get_metrics(objs)

    template = util.get_jinja2_template(
        template_filename,
        filters=(
            ("cpp", cpp_datatypes_filter),
            ("snake_case", util.snake_case),
            ("type_name", type_name),
            ("extra_type_name", extra_type_name),
            ("metric_id", get_metric_id),
            ("ping_id", get_ping_id),
            ("Camelize", util.Camelize),
        ),
    )

    output_fd.write(template.render(all_objs=objs))
    output_fd.write("\n")

def output_source(objs, output_fd, options={}):
    """
    Given a tree of objects, output C++ code to the file-like object `output_fd`.

    :param objs: A tree of objects (metrics and pings) as returned from
    `parser.parse_objects`.
    :param output_fd: Writeable file to write the output to.
    :param options: options dictionary.
    """

    # Monkeypatch a util.snake_case function for the templates to use
    util.snake_case = lambda value: value.replace(".", "_").replace("-", "_")
    # Monkeypatch util.get_jinja2_template to find templates nearby

    def get_local_template(template_name, filters=()):
        env = jinja2.Environment(
            loader=jinja2.PackageLoader("cpp", "templates"),
            trim_blocks=True,
            lstrip_blocks=True,
        )
        env.filters["camelize"] = util.camelize
        env.filters["Camelize"] = util.Camelize
        for filter_name, filter_func in filters:
            env.filters[filter_name] = filter_func
        return env.get_template(template_name)

    util.get_jinja2_template = get_local_template
    get_metric_id = generate_metric_ids(objs)
    get_ping_id = generate_ping_ids(objs)

    if "pings" in objs:
        template_filename = "cpp_pings_source.jinja2"
        if objs.get("tags"):
            del objs["tags"]
    else:
        template_filename = "cpp_metrics_source.jinja2"
        objs = get_metrics(objs)

    template = util.get_jinja2_template(
        template_filename,
        filters=(
            ("cpp", cpp_datatypes_filter),
            ("snake_case", util.snake_case),
            ("type_name", type_name),
            ("extra_type_name", extra_type_name),
            ("metric_id", get_metric_id),
            ("ping_id", get_ping_id),
            ("Camelize", util.Camelize),
        ),
    )

    output_fd.write(template.render(all_objs=objs))
    output_fd.write("\n")
