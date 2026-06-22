# -*- coding: utf-8 -*-

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import argparse
import json
import os
import xml.etree.ElementTree as ET
import shutil
import subprocess
import sys
from translate import translate_addon

# hack to be able to re-use things in shared.py
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '../')))
from shared import qtquery

def get_file_list(path, prefix):
    file_list = []
    for file in os.listdir(path):
        file_path = os.path.join(path, file)
        if (
            os.path.isfile(file_path)
            and not file_path.endswith(".ts")
            and not file_path.endswith(".qrc")
            and not file_path.endswith(".rcc")
        ):
            file_list.append(f"{prefix}{file}")
            continue

        if os.path.isdir(file_path):
            file_list += get_file_list(file_path, f"{prefix}{file}/")

    return file_list

parser = argparse.ArgumentParser(description="Generate an addon package")
parser.add_argument(
    "source",
    metavar="MANIFEST",
    type=str,
    action="store",
    help="The addon manifest",
)
parser.add_argument(
    "dest",
    metavar="DEST",
    type=str,
    action="store",
    help="The destination folder",
)
parser.add_argument(
    "-q",
    "--qt_path",
    default=[],
    action='append',
    dest="qtpath",
    help="The QT binary path. If not set, we try to guess.",
)
parser.add_argument(
    "-d",
    "--depfile",
    default=None,
    dest="depfile",
    help="Generate a dependency file"
)
parser.add_argument(
    "-i",
    "--i18n",
    default=None,
    dest="i18npath",
    help="Internationalization project path"
)
args = parser.parse_args()

qtpathsep = ";" if (os.name == "nt") else ":"

if len(args.qtpath) == 0:
    # Try to get the Qt tooling paths from qmake.
    p = qtquery("qmake", "QT_INSTALL_BINS")
    if p is not None:
        args.qtpath.append(p)
    p = qtquery("qmake", "QT_INSTALL_LIBEXECS")
    if p is not None:
        args.qtpath.append(p)

    p = qtquery("qmake6", "QT_INSTALL_BINS")
    if p is not None:
        args.qtpath.append(p)
    p = qtquery("qmake6", "QT_INSTALL_LIBEXECS")
    if p is not None:
        args.qtpath.append(p)
else:
    # If we can find a qmake, then add libexec to our search path too.
    qmake = shutil.which("qmake", path=qtpathsep.join(args.qtpath))
    libexecs = qtquery(qmake, "QT_INSTALL_LIBEXECS")
    if libexecs is not None:
        args.qtpath.append(libexecs)

qtsearchpath=qtpathsep.join(args.qtpath)

rcc = shutil.which("rcc", path=qtsearchpath)
if rcc is None:
    print("Unable to locate rcc path.", file=sys.stderr)
    sys.exit(1)

if not os.path.isfile(args.source):
    exit(f"`{args.source}` is not a file")

if not os.path.isdir(args.dest):
    exit(f"`{args.dest}` is not a directory")

script_path = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))

jsonSchema = os.path.join(script_path, "ci", "jsonSchemas", "addon.json")
if not os.path.isfile(jsonSchema):
    exit(f"The JSONSchema {jsonSchema} does not exist")

tmp_path, addon_id, unneeded_1, unneeded_2 = translate_addon(qtsearchpath, args.source, args.dest, args.i18npath, script_path)

if args.depfile is not None:
    print("Generate the dependency file...")
    with open(args.depfile, "w") as f:
        f.write(f"{os.path.join(args.dest, addon_id)}.rcc: {args.source}")
        srcdir = os.path.dirname(args.source)
        for file in get_file_list(srcdir, ""):
            f.write(f" {os.path.join(srcdir, file)}")

print("Generate the RCC file...")
files = get_file_list(tmp_path, "")

qrc_file = os.path.join(tmp_path, f"{addon_id}.qrc")
with open(qrc_file, "w", encoding="utf-8") as f:
    rcc_elm = ET.Element("RCC")
    qresource = ET.SubElement(rcc_elm, "qresource")
    qresource.set("prefix", "/")
    for file in files:
        elm = ET.SubElement(qresource, "file")
        elm.text = file
    f.write(ET.tostring(rcc_elm, encoding="unicode"))

print("Creating the final addon...")
rcc_file = os.path.join(args.dest, f"{addon_id}.rcc")
os.system(f"{rcc} --binary --no-zstd --output {rcc_file} {qrc_file}")
print(f"Done: {rcc_file}")
