# -*- coding: utf-8 -*-

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import sys
from pathlib import Path

import cpp
import rust
from glean_parser import lint, parser, translate, util


class ParserError(Exception):
    """Thrown from parse if something goes wrong"""

    pass


def parse(args):
    """
    Parse and lint the input files,
    then return the parsed objects for further processing.
    """

    input_files = [Path(x) for x in args]

    return parse_with_options(input_files, {
        "allow_reserved": False,
    })


def parse_with_options(input_files, options):
    # Derived heavily from glean_parser.translate.translate.
    # Adapted to how mozbuild sends us a fd, and to expire on versions not dates.

    # Lint the yaml first, then lint the metrics.
    if lint.lint_yaml_files(input_files, parser_config=options):
        # Warnings are Errors
        raise ParserError("linter found problems")

    all_objs = parser.parse_objects(input_files, options)
    if util.report_validation_errors(all_objs):
        raise ParserError("found validation errors during parse")

    nits = lint.lint_metrics(all_objs.value, options)
    if nits is not None and any(nit.check_name != "EXPIRED" for nit in nits):
        # Treat Warnings as Errors.
        # But don't fail the whole build on expired metrics (it blocks testing).
        raise ParserError("glinter nits found during parse")

    objects = all_objs.value

    translate.transform_metrics(objects)

    return objects, options



def rust_metrics(output_fd, *args):
    all_objs, options = parse(args)
    rust.output_rust(all_objs, output_fd, options)


def cpp_metrics(output_fd, *args):
    all_objs, options = parse(args)
    cpp.output_cpp(all_objs, output_fd, options)


if __name__ == "__main__":
    lang = sys.argv[1]
    file_path = sys.argv[2]
    fn = rust_metrics if lang == "rust" else cpp_metrics
    with open(file_path, 'w+') as f:
        fn(f, *sys.argv[3:])
