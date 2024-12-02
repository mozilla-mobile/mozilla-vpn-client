# -*- coding: utf-8 -*-

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

# hack to be able to re-use parseYAMLTranslationStrings
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '../utils')))
from generate_strings import parseYAMLTranslationStrings

# hack to be able to re-use things in shared.py
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '../')))
from shared import write_en_language, qtquery

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


def retrieve_strings_blocks(blocks, filename, strings, prefix, shared_strings):
    for block in blocks:
        if "id" not in block:
            exit(f"{filename} does not have an id for one of the blocks")
        if "type" not in block:
            exit(f"{filename} does not have a type for block id {block['id']}")
        if "content" not in block:
            exit(f"{filename} does not have a content for block id {block['id']}")

        block_id = block["id"]
        legacy_block_string_id = f"{prefix}.block.{block_id}"
        if block["type"] in ["olist", "ulist"]:
            block_string_id = legacy_block_string_id
        else:
            block_string_id = legacy_block_string_id if not shared_strings else block["content"]
        block_default_comment = comment_types.get(block["type"], "")
        if block_string_id in strings:
            exit(f"Duplicate block enum {block_string_id} when parsing {filename}")

        if not isinstance(block["content"], list):
            if shared_strings:
                translation_obj = find_translation_object(shared_strings, block_string_id)
                strings[block_string_id] = {
                    "value": translation_obj['value'][0],
                    "comments": translation_comment(translation_obj),
                    "legacy_id": legacy_block_string_id
                }
            else:
                strings[block_string_id] = {
                    "value": block["content"],
                    "comments": block.get("comment", block_default_comment)
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
            legacy_subblock_string_id = f"{prefix}.block.{block_id}.{subblock_id}"
            subblock_string_id = legacy_subblock_string_id if not shared_strings else subblock["content"]
            if subblock_string_id in strings:
                exit(
                    f"Duplicate sub-block enum {subblock_string_id} when parsing {filename}"
                )

            if shared_strings:
                translation_obj = find_translation_object(shared_strings, subblock_string_id)
                strings[subblock_string_id] = {
                    "value": translation_obj['value'][0],
                    "comments": translation_comment(translation_obj),
                    "legacy_id": legacy_subblock_string_id
                }
            else:
                strings[subblock_string_id] = {
                    "value": subblock["content"],
                    "comments": subblock.get("comment", block_default_comment)
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

    return retrieve_strings_blocks(guide_json["blocks"], filename, guide_strings, f"guide.{guide_id}", None)


def retrieve_strings_message(manifest, filename):
    message_json = manifest["message"]
    if "id" not in message_json:
        exit(f"Message {filename} does not have an id")
    if "title" not in message_json:
        exit(f"Message {filename} does not have a title")
    if "blocks" not in message_json:
        exit(f"Message {filename} does not have a blocks")

    use_shared_strings = "usesSharedStrings" in message_json and message_json["usesSharedStrings"] == True
    if use_shared_strings:
        return retrieve_shared_strings_message(message_json, filename)
    else:
        return retrieve_legacy_strings_message(message_json, filename)

def retrieve_legacy_strings_message(message_json, filename):
    message_strings = {}

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

    return retrieve_strings_blocks(message_json["blocks"], filename, message_strings, f"message.{message_id}", None)

def retrieve_shared_strings_message(message_json, filename):
    message_strings = {}

    # Manifest file is one level up from the main `addons` directory.
    addons_dir = os.path.join(os.path.dirname(os.path.abspath(filename)), "../")
    strings_file = os.path.normpath(os.path.join(addons_dir, "strings.yaml"))
    json_translations = parseYAMLTranslationStrings(strings_file)

    # For addons to work with older clients, the final strings must be in the id format expected by addons in the client.
    # However, to properly pull the translations for all languages, the shared string ID must be used.
    # Thus, the legacy_id is recorded here, and the shared id is swapped out for legacy_id in `transform_shared_strings`.
    title_id = message_json["title"]
    title_object = find_translation_object(json_translations, title_id)
    message_id = message_json["id"]
    legacy_title_id = f"message.{message_id}.title"
    message_strings[title_id] = {
        "value": title_object['value'][0],
        "comments": translation_comment(title_object),
        "legacy_id": legacy_title_id
    }
    subtitle_id = message_json["subtitle"]
    subtitle_object = find_translation_object(json_translations, subtitle_id)
    legacy_subtitle_id = f"message.{message_id}.subtitle"
    message_strings[subtitle_id] = {
        "value": subtitle_object['value'][0],
        "comments": translation_comment(subtitle_object),
        "legacy_id": legacy_subtitle_id
    }

    return retrieve_strings_blocks(message_json["blocks"], filename, message_strings, f"message.{message_id}", json_translations)

def find_translation_object(json_translations, target_id):
    if not target_id or not isinstance(target_id, str):
        exit(f"No target string")

    for translation_object in json_translations.values():
        if translation_object['string_id'] == target_id:
            # Make sure the found object has a value section, and that value section is a list with at least 1 entry and that first entry is a string.
            if not translation_object['value'] or not isinstance(translation_object['value'], list) or len(translation_object['value']) < 1 or not isinstance(translation_object['value'][0], str):
                exit(f"No value for translation {target_id}")
            return translation_object
    exit(f"No translation object found for {target_id}")

def translation_comment(translation_object):
    if not translation_object['comments'] or not isinstance(translation_object['comments'], list) or len(translation_object['comments']) < 1 or not isinstance(translation_object['comments'][0], str):
        return None
    return translation_object['comments'][0]

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

# Filter xliff file to those in `ids_to_filter`, and transform the ID to legacy ID
# Additionally, swap in the short version string if needed
def transform_shared_strings(input_file, output_file, relevant_strings, short_version):
    tree = ET.parse(input_file)
    root = tree.getroot()
    ns = {'xliff': 'urn:oasis:names:tc:xliff:document:1.2'}
    body = root.find('.//xliff:body', ns)

    ids_to_filter = list(relevant_strings.keys())

    # Iterate over all 'trans-unit' elements
    for trans_unit in root.findall('.//xliff:trans-unit', ns):
        # Check the id attribute...
        if trans_unit.get('id') not in ids_to_filter:
            # ...and remove if not in the filter list
            if body is not None:
                body.remove(trans_unit)
        else:
            # ...and replace it with the legacy ID if it is in the list
            string_details = relevant_strings[trans_unit.get('id')]
            if not string_details or not isinstance(string_details, dict) or not isinstance(string_details['legacy_id'], str):
                exit(f"No legacy_id for {string_details}")
            trans_unit.set('id', string_details['legacy_id'])

            # Then, swap in the short version number if needed.
            # All languages have source, but only non-English languages have target.
            source = trans_unit.find('.//xliff:source', ns)
            source.text = source.text.replace('%1', short_version)
            target = trans_unit.find('.//xliff:target', ns)
            if type(target) is ET.Element and target.text:
                target.text = target.text.replace('%1', short_version)

    # Write the filtered tree to the output file, creating the folders if needed
    output_dir = os.path.dirname(os.path.abspath(output_file))
    addons_dir = os.path.dirname(output_dir)
    lang_dir = os.path.dirname(addons_dir)
    if not os.path.isdir(lang_dir):
        os.mkdir(lang_dir)
    if not os.path.isdir(addons_dir):
        os.mkdir(addons_dir)
    if not os.path.isdir(output_dir):
        os.mkdir(output_dir)
    with open(output_file, 'wb') as f:
        tree.write(f, encoding='utf-8', xml_declaration=True)

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

# Lookup our required tools for addon generation.
lconvert = shutil.which("lconvert", path=qtsearchpath)
if lconvert is None:
    print("Unable to locate lconvert path.", file=sys.stderr)
    sys.exit(1)

lrelease = shutil.which("lrelease", path=qtsearchpath)
if lrelease is None:
    print("Unable to locate lrelease path.", file=sys.stderr)

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

print("Reading the translation fallback file...")
translations_fallback = {}
with open( os.path.join(os.path.dirname(script_path), "src", "translations", "extras", "translations_fallback.json"), "r", encoding="utf-8") as file:
    translations_fallback = json.load(file)

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
        elif manifest["type"] == "replacer":
          pass
        else:
            exit(f"Unsupported manifest type `{manifest['type']}`")

        print("Create localization file...")
        os.mkdir(os.path.join(tmp_path, "i18n"))
        template_ts_file = os.path.join(args.dest, f"{manifest['id']}.ts")
        write_en_language(template_ts_file, strings, True)

        # This will be probably replaced by the en locale if it exists
        en_ts_file = os.path.join(tmp_path, "i18n", "locale_en.ts")
        shutil.copyfile(template_ts_file, en_ts_file)
        os.system(f"{lrelease} -idbased {en_ts_file}")

        # Fallback
        ts_file = os.path.join(tmp_path, "i18n", "locale.ts")
        shutil.copyfile(template_ts_file, ts_file)
        os.system(f"{lrelease} -idbased {ts_file}")

        # Prepare for shared strings, if they will be used
        use_shared_strings = "message" in manifest and "usesSharedStrings" in manifest["message"] and manifest["message"]["usesSharedStrings"] == True
        short_version = None
        if use_shared_strings:
            # Confirm that shortVersion exists and is a string
            if "shortVersion" not in manifest["message"] or type(manifest["message"]["shortVersion"]) is not str:
                exit("shortVersion string required when usesSharedStrings is true")
            short_version = manifest["message"]["shortVersion"]

        # Include internationalization if the i18n path was specified.
        completeness = []
        i18nlocales = []
        if args.i18npath is not None:
            i18nlocales = os.listdir(args.i18npath)
        for locale in i18nlocales:
            if not os.path.isdir(os.path.join(args.i18npath, locale)) or locale.startswith("."):
                continue

            xliff_path = os.path.join(args.i18npath, locale, "addons", manifest["id"], "strings.xliff")
            if use_shared_strings:
                shared_xliff_path = os.path.join(args.i18npath, locale, "addons", "strings.xliff")
                # make sure we have a shared translation file
                if not os.path.isfile(shared_xliff_path):
                    continue

                xliff_path = os.path.join(tmp_path, "i18n", locale, "addons", manifest["id"], "strings.xliff")
                transform_shared_strings(shared_xliff_path, xliff_path, strings, short_version)

            if os.path.isfile(xliff_path):
                locale_file = os.path.join(tmp_path, "i18n", f"locale_{locale}.ts")

                # When 2.15 will be the min-required version, we can remove
                # this block and generate TS files with `no-untranslated'
                # option. But to be back-compatible, we need to compute the
                # language fallback here instead of in the client.
                if locale in translations_fallback:
                    # The fallback translations are computed in reverse order.
                    # First "en" where we have 100% of translations by default.
                    xliff_path_en = os.path.join(args.i18npath, "en", "addons", manifest["id"], "strings.xliff")
                    if use_shared_strings:
                        shared_xliff_path = os.path.join(args.i18npath, "en", "addons", "strings.xliff")
                        xliff_path_en = os.path.join(tmp_path, "i18n", "en", "addons", manifest["id"], "strings.xliff")
                        transform_shared_strings(shared_xliff_path, xliff_path_en, strings, short_version)
                    locale_file_en = os.path.join(tmp_path, "i18n", "locale_en_fallback.ts")
                    os.system(f"{lconvert} -if xlf -i {xliff_path_en} -o {locale_file_en}")

                    # Then the fallback languages
                    locale_file_fallbacks = []
                    for fallback in translations_fallback[locale]:
                        xliff_path_fallback = os.path.join(args.i18npath, fallback, "addons", manifest["id"], "strings.xliff")
                        if use_shared_strings:
                            shared_xliff_path = os.path.join(args.i18npath, fallback, "addons", "strings.xliff")
                            if not os.path.isfile(shared_xliff_path):
                                continue
                            xliff_path_fallback = os.path.join(tmp_path, "i18n", fallback, "addons", manifest["id"], "strings.xliff")
                            transform_shared_strings(shared_xliff_path, xliff_path_fallback, strings, short_version)
                        locale_file_fallback = os.path.join(tmp_path, "i18n", f"locale_{fallback}.ts")
                        locale_file_fallbacks.append(locale_file_fallback)
                        os.system(f"{lconvert} -if xlf -i {xliff_path_fallback} -no-untranslated -o {locale_file_fallback}")

                    # Finally, the current language
                    os.system(f"{lconvert} -if xlf -i {xliff_path} -no-untranslated -o {locale_file}")

                    # All is unified in reverse order.
                    os.system(f"{lconvert} -i {locale_file_en} {' '.join(locale_file_fallbacks)} {locale_file} -o {locale_file}")
                else:
                    os.system(f"{lconvert} -if xlf -i {xliff_path} -o {locale_file}")

                os.system(f"{lrelease} -idbased {locale_file}")

                xlifftool_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), "..", "utils", "xlifftool.py")
                xlifftool_cmd = [sys.executable, xlifftool_path, "-C", f"--locale={locale}", xliff_path]
                xlifftool = subprocess.run(xlifftool_cmd, capture_output=True)
                # This completeness metric can be out-of-date, sometimes reporting a higher-than-actual completeness after a new string is
                # added to the app. This happens because the .xliff file doesn't have all the source strings, so an incorrect denominator is used.
                # To show up in the .xliff, there must be a complete cycle of a new string being pulled into Pontoon (via the `extract new strings`
                # job being run and merged into the translation repo) and then the VPN repo merging in a new translations commit.
                # We protect against this by always loading English as a fallback language.
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

    if args.depfile is not None:
        print("Generate the dependency file...")
        with open(args.depfile, "w") as f:
            f.write(f"{os.path.join(args.dest, manifest['id'])}.rcc: {args.source}")
            srcdir = os.path.dirname(args.source)
            for file in get_file_list(srcdir, ""):
                f.write(f" {os.path.join(srcdir, file)}")

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
    os.system(f"{rcc} --binary --no-zstd --output {rcc_file} {qrc_file}")
    print(f"Done: {rcc_file}")
