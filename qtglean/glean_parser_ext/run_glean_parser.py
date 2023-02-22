# -*- coding: utf-8 -*-

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import argparse
import sys
import os
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
    cpp.output_source(all_objs, output_fd, options)

def cpp_headers(output_fd, *args):
    all_objs, options = parse(args)
    cpp.output_header(all_objs, output_fd, options)


def create_dir(path):
    """
    Creates directory if it doesn't exist. Logs a message otherwise.
    Other types of errors are still thrown.
    """
    try:
        os.mkdir(path)
    except FileExistsError:
        # This error can be ignored.
        print("Glean files directory \"{}\" exists.".format(path))
    except Exception as e:
        print("Error generating Glean files directory \"{}\".".format(path))
        raise e;


if __name__ == "__main__":
    argparser = argparse.ArgumentParser(
        description='Parse glean telemetry definitions and generate C++/Rust code')

    argparser.add_argument('filename', metavar='FILE', type=str, nargs='?',
        help='Specific file to generate')
    argparser.add_argument('gleanargs', metavar='ARG', type=str, nargs='*',
        help='Extra arguments to pass to glean_parser')
    argparser.add_argument('-d', '--outdir', metavar='DIR', type=str,
        help='Output generated content to DIR')
    args = argparser.parse_args()

    # Setup default output directories.
    workspace_root = Path(os.path.dirname(os.path.realpath(__file__))).parent.parent
    if args.outdir is None:
        args.outdir = os.path.join(workspace_root, "qtglean", "prebuilt", "glean", "generated")

    if args.filename is None:
        print("Generating Mozilla VPN Glean files.")

        yaml_files_path = os.path.join(workspace_root, "glean")
        qtglean_yaml_files_path = os.path.join(workspace_root, "qtglean")

        os.makedirs(args.outdir, exist_ok=True)

        # Generate C++ header files
        for [ output, input ] in [
            [os.path.join(args.outdir, "pings.h"), [os.path.join(yaml_files_path, "pings.yaml"), os.path.join(qtglean_yaml_files_path, "pings.yaml")]],
            [os.path.join(args.outdir, "metrics.h"), [os.path.join(yaml_files_path, "metrics.yaml"), os.path.join(qtglean_yaml_files_path, "metrics.yaml")]],
        ]:
            print("Generating {} from {}".format(output, input))
            with open(output, 'w+', encoding='utf-8') as f:
                cpp_headers(f, *input)

        # Generate C++ source files
        for [ output, input ] in [
            [os.path.join(args.outdir, "pings.cpp"), [os.path.join(yaml_files_path, "pings.yaml"), os.path.join(qtglean_yaml_files_path, "pings.yaml")]],
            [os.path.join(args.outdir, "metrics.cpp"), [os.path.join(yaml_files_path, "metrics.yaml"), os.path.join(qtglean_yaml_files_path, "metrics.yaml")]],
        ]:
            print("Generating {} from {}".format(output, input))
            with open(output, 'w+', encoding='utf-8') as f:
                cpp_metrics(f, *input)

        # Generate Rust files
        for [ output, input ] in [
            [os.path.join(args.outdir, "pings.rs"), [os.path.join(yaml_files_path, "pings.yaml"), os.path.join(qtglean_yaml_files_path, "pings.yaml")]],
            [os.path.join(args.outdir, "metrics.rs"), [os.path.join(yaml_files_path, "metrics.yaml"), os.path.join(qtglean_yaml_files_path, "metrics.yaml")]],
        ]:
            print("Generating {} from {}".format(output, input))
            with open(output, 'w+', encoding='utf-8') as f:
                rust_metrics(f, *input)
    else:
        # Guess language and operation from the file extension.
        file_split = os.path.splitext(args.filename)
        if file_split[1] == '.rs':
            fn = rust_metrics
            lang = 'rust'
        elif file_split[1] == '.cpp':
            fn = cpp_metrics
            lang = 'c++'
        elif file_split[1] == '.h':
            fp = cpp_headers
            lang = 'c++'
        else:
            print(f'Unknown language for extension: "{file_split[1]}"', file=sys.stderr)
            sys.exit(1)

        print("Generating Glean files for '{}' on '{}' with arguments '{}'.".format(lang, args.filename, args.gleanargs))

        # Generate the file
        with open(args.filename, 'w+', encoding='utf-8') as f:
            print("Generating {} with args {}".format(args.filename, args.gleanargs))
            fn(f, *args.gleanargs)
