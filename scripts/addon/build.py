#! /usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import argparse
import json
import os
import xml.etree.ElementTree as ET
import tempfile
import shutil
import subprocess
import sys

comment_types = {
    "text": "Standard text in a composer block",
    "title": "Title in a composer block",
    "ulist": "Bullet unordered list item in a composer block",
    "olist": "Bullet ordered list item in a composer block",
}


def retrieve_strings_tutorial(manifest, filename):
    tutorial_strings = {}

    tutorial_json = manifest["tutorial"]
    if "id" not in tutorial_json:
        exit(f"Tutorial {filename} does not have an id")
    if "title" not in tutorial_json:
        exit(f"Tutorial {filename} does not have a title")
    if "subtitle" not in tutorial_json:
        exit(f"Tutorial {filename} does not have a subtitle")
    if "completion_message" not in tutorial_json:
        exit(f"Tutorial {filename} does not have a completion message")
    if "steps" not in tutorial_json:
        exit(f"Tutorial {filename} does not have steps")

    tutorial_id = tutorial_json["id"]
    title_id = f"tutorial.{tutorial_id}.title"
    tutorial_strings[title_id] = {
        "value": tutorial_json["title"],
        "comments": tutorial_json.get("title_comment", "Title for a tutorial view"),
    }

    subtitle_id = f"tutorial.{tutorial_id}.subtitle"
    tutorial_strings[subtitle_id] = {
        "value": tutorial_json["subtitle"],
        "comments": tutorial_json.get(
            "subtitle_comment", "Subtitle for a tutorial view"
        ),
    }

    completion_id = f"tutorial.{tutorial_id}.completion_message"
    tutorial_strings[completion_id] = {
        "value": tutorial_json["completion_message"],
        "comments": tutorial_json.get(
            "completion_message_comment", "Completion message for a tutorial view"
        ),
    }

    for step in tutorial_json["steps"]:
        if "id" not in step:
            exit(f"Tutorial {filename} does not have an id for one of the steps")
        if "tooltip" not in step:
            exit(
                f"Tutorial {filename} does not have a tooltip for step id {step['id']}"
            )

        step_id = f"tutorial.{tutorial_id}.step.{step['id']}"
        if step_id in tutorial_strings:
            exit(f"Duplicate step id {step_id} when parsing {filename}")

        tutorial_strings[step_id] = {
            "value": step["tooltip"],
            "comments": step.get("comment", "A tutorial step tooltip"),
        }

    return tutorial_strings


def retrieve_strings_blocks(blocks, filename, strings, prefix):
    for block in blocks:
        if "id" not in block:
            exit(f"{filename} does not have an id for one of the blocks")
        if "type" not in block:
            exit(f"{filename} does not have a type for block id {block['id']}")
        if "content" not in block:
            exit(f"{filename} does not have a content for block id {block['id']}")

        block_id = block["id"]
        block_string_id = f"{prefix}.block.{block_id}"
        block_default_comment = comment_types.get(block["type"], "")
        if block_string_id in strings:
            exit(f"Duplicate block enum {block_string_id} when parsing {filename}")

        if not isinstance(block["content"], list):
            strings[block_string_id] = {
                "value": block["content"],
                "comments": block.get("comment", block_default_comment),
            }
            continue

        for subblock in block["content"]:
            if "id" not in subblock:
                exit(
                    f"{filename} does not have an id for one of the subblocks of block {block_id}"
                )
            if "content" not in subblock:
                exit(
                    f"File {filename} does not have a content for subblock id {subblock['id']}"
                )

            subblock_id = subblock["id"]
            subblock_string_id = f"{prefix}.block.{block_id}.{subblock_id}"
            if subblock_string_id in strings:
                exit(
                    f"Duplicate sub-block enum {subblock_string_id} when parsing {filename}"
                )

            strings[subblock_string_id] = {
                "value": subblock["content"],
                "comments": subblock.get("comment", block_default_comment),
            }

    return strings


def retrieve_strings_guide(manifest, filename):
    guide_strings = {}

    guide_json = manifest["guide"]
    if "id" not in guide_json:
        exit(f"Guide {filename} does not have an id")
    if "title" not in guide_json:
        exit(f"Guide {filename} does not have a title")
    if "blocks" not in guide_json:
        exit(f"Guide {filename} does not have a blocks")

    guide_id = guide_json["id"]
    title_id = f"guide.{guide_id}.title"
    guide_strings[title_id] = {
        "value": guide_json["title"],
        "comments": guide_json.get("title_comment", "Title for a guide view"),
    }

    if guide_json.get("subtitle"):
        subtitle_id = f"guide.{guide_id}.subtitle"
        guide_strings[subtitle_id] = {
            "value": guide_json["subtitle"],
            "comments": guide_json.get("subtitle_comment", "Subtitle for a guide view"),
        }

    return retrieve_strings_blocks(guide_json["blocks"], filename, guide_strings, f"guide.{guide_id}")


def retrieve_strings_message(manifest, filename):
    message_strings = {}

    message_json = manifest["message"]
    if "id" not in message_json:
        exit(f"Message {filename} does not have an id")
    if "title" not in message_json:
        exit(f"Message {filename} does not have a title")
    if "blocks" not in message_json:
        exit(f"Message {filename} does not have a blocks")

    message_id = message_json["id"]
    title_id = f"message.{message_id}.title"
    message_strings[title_id] = {
        "value": message_json["title"],
        "comments": message_json.get("title_comment", "Title for a message view"),
    }
    subtitle_id = f"message.{message_id}.subtitle"
    message_strings[subtitle_id] = {
        "value": message_json["subtitle"],
        "comments": message_json.get("subtitle_comment", "Subtitle for a message view"),
    }

    return retrieve_strings_blocks(message_json["blocks"], filename, message_strings, f"message.{message_id}")


def write_en_language(filename, strings):
    ts = ET.Element("TS")
    ts.set("version", "2.1")
    ts.set("language", "en")

    context = ET.SubElement(ts, "context")
    ET.SubElement(context, "name")

    for key, value in strings.items():
        message = ET.SubElement(context, "message")
        message.set("id", key)

        location = ET.SubElement(message, "location")
        location.set("filename", "addon.qml")

        source = ET.SubElement(message, "source")
        source.text = value["value"]

        translation = ET.SubElement(message, "translation")
        translation.set("type", "unfinished")

        if len(value["comments"]) > 0:
            extracomment = ET.SubElement(message, "extracomment")
            extracomment.text = value["comments"]

    with open(filename, "w", encoding="utf-8") as f:
        f.write(ET.tostring(ts, encoding="unicode"))


def copy_files(path, dest_path):
    for file in os.listdir(path):
        if file.startswith("."):
            continue

        file_path = os.path.join(path, file)
        if os.path.isfile(file_path):
            if file_path.endswith((".ts", ".qrc", ".rcc")):
                exit(f"Unexpected extension file found: {os.path.join(path, file)}")

            shutil.copyfile(file_path, os.path.join(dest_path, file))
            continue

        if os.path.isdir(file_path):
            dir_path = os.path.join(dest_path, file)
            os.mkdir(dir_path)
            copy_files(file_path, dir_path)


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
    "project",
    metavar="PROJECT",
    type=str,
    action="store",
    help="The project name (vpn, relay, foobar, ...)",
)
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
    default=None,
    dest="qtpath",
    help="The QT binary path. If not set, we try to guess.",
)
args = parser.parse_args()

def qtquery(qmake, propname):
    try:
        qtquery = os.popen(f"{qmake} -query {propname}")
        qtpath = qtquery.read().strip()
        if len(qtpath) > 0:
            return qtpath
    finally:
        pass
    return None


qtbinpath = args.qtpath
if qtbinpath is None:
    qtbinpath = qtquery("qmake", "QT_INSTALL_BINS")
if qtbinpath is None:
    qtbinpath = qtquery("qmake6", "QT_INSTALL_BINS")
if qtbinpath is None:
    print("Unable to locate qmake tool.")
    sys.exit(1)

if not os.path.isdir(qtbinpath):
    print(f"QT path is not a diretory: {qtbinpath}")
    sys.exit(1)

lconvert = os.path.join(qtbinpath, "lconvert")
lrelease = os.path.join(qtbinpath, "lrelease")

rcc_bin = "rcc"
if os.name == "nt":
    rcc_bin = "rcc.exe"

rcc = os.path.join(qtbinpath, rcc_bin)
if not os.path.isfile(rcc):
    qtlibexecpath = qtquery(os.path.join(qtbinpath, "qmake"), "QT_INSTALL_LIBEXECS")
    if qtlibexecpath is None:
        qtlibexecpath = qtquery(
            os.path.join(qtbinpath, "qmake6"), "QT_INSTALL_LIBEXECS"
        )
    if qtlibexecpath is None:
        print("Unable to locate qmake libexec path.")
        sys.exit(1)
    rcc = os.path.join(qtlibexecpath, rcc_bin)
    if not os.path.isfile(rcc):
        print("Unable to locate rcc path.")
        sys.exit(1)

if not os.path.isfile(args.source):
    exit(f"`{args.source}` is not a file")

if not os.path.isdir(args.dest):
    exit(f"`{args.dest}` is not a directory")

script_path = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))

i18n_path = os.path.join(os.path.dirname(script_path), "src", "apps", args.project, "translations", "i18n")
if not os.path.isdir(i18n_path):
    exit(f"The translation for project `{args.project}` does not exist at path {i18n_path}")

jsonSchema = os.path.join(script_path, "ci", "jsonSchemas", "addon.json")
if not os.path.isfile(jsonSchema):
    exit(f"The JSONSchema {jsonSchema} does not exist")

with open(args.source, "r", encoding="utf-8") as file:
    manifest = json.load(file)

    print("Copying files in a temporary folder...")
    tmp_path = tempfile.mkdtemp()
    copy_files(os.path.dirname(args.source), tmp_path)

    strings = {}

    if "translatable" not in manifest or manifest["translatable"] == True:
        print("Retrieving strings...")
        if manifest["type"] == "tutorial":
            strings = retrieve_strings_tutorial(manifest, args.source)
        elif manifest["type"] == "guide":
            strings = retrieve_strings_guide(manifest, args.source)
        elif manifest["type"] == "message":
            strings = retrieve_strings_message(manifest, args.source)
        else:
            exit(f"Unupported manifest type `{manifest['type']}`")

        print("Create localization file...")
        os.mkdir(os.path.join(tmp_path, "i18n"))
        template_ts_file = os.path.join(args.dest, f"{manifest['id']}.ts")
        write_en_language(template_ts_file, strings)

        # This will be probably replaced by the en locale if it exists
        en_ts_file = os.path.join(tmp_path, "i18n", "locale_en.ts")
        shutil.copyfile(template_ts_file, en_ts_file)
        os.system(f"{lrelease} -idbased {en_ts_file}")

        # Fallback
        ts_file = os.path.join(tmp_path, "i18n", "locale.ts")
        shutil.copyfile(template_ts_file, ts_file)
        os.system(f"{lrelease} -idbased {ts_file}")

        completeness = [];
        for locale in os.listdir(i18n_path):
            if not os.path.isdir(os.path.join(i18n_path, locale)) or locale.startswith("."):
                continue

            xliff_path = os.path.join(
                i18n_path, locale, "addons", manifest["id"], "strings.xliff"
            )

            if os.path.isfile(xliff_path):
                locale_file = os.path.join(tmp_path, "i18n", f"locale_{locale}.ts")
                os.system(f"{lconvert} -if xlf -i {xliff_path} -o {locale_file}")
                os.system(f"{lrelease} -idbased {locale_file}")

                xlifftool_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), "..", "utils", "xlifftool.py")
                xlifftool_cmd = [sys.executable, xlifftool_path, "-C", f"--locale={locale}", xliff_path]
                xlifftool = subprocess.run(xlifftool_cmd, capture_output=True)
                completeness_output = xlifftool.stdout.decode("utf-8")
                completeness.append(f"{locale}:{completeness_output}")

        # If we don't have translations yet, we still have English at 100%.
        if not completeness:
            completeness.append("en:1.0")

        completeness_file = os.path.join(tmp_path, "i18n", f"translations.completeness")
        with open(completeness_file, "w", encoding="utf-8") as f:
            f.write("".join(completeness))

    else:
       print("Addon not translatable")

    print("Generate the RCC file...")
    files = get_file_list(tmp_path, "")

    qrc_file = os.path.join(tmp_path, f"{manifest['id']}.qrc")
    with open(qrc_file, "w", encoding="utf-8") as f:
        rcc_elm = ET.Element("RCC")
        qresource = ET.SubElement(rcc_elm, "qresource")
        qresource.set("prefix", "/")
        for file in files:
            elm = ET.SubElement(qresource, "file")
            elm.text = file
        f.write(ET.tostring(rcc_elm, encoding="unicode"))

    print("Creating the final addon...")
    rcc_file = os.path.join(args.dest, f"{manifest['id']}.rcc")
    os.system(f"{rcc} {qrc_file} -o {rcc_file} -binary")
    print(f"Done: {rcc_file}")
