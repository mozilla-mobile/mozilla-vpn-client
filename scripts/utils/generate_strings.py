#! /usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import re
import os
import yaml
import json
import argparse

comment_types = {
    "text": f"Standard text in a guide block",
    "title": f"Title in a guide block",
    "ulist": f"Bullet unordered list item in a guide block",
    "olist": f"Bullet ordered list item in a guide block",
}


def stop(string_id):
    exit(
        f"Each key must be a string or a list with 1 or more items. Fix string ID `{string_id}`"
    )


def pascalize(string):
    output = ""
    for chunk in string.split("_"):
        output += chunk[0].upper()
        output += chunk[1:]
    return output


# special loader with duplicate key checking
# From: https://gist.github.com/pypt/94d747fe5180851196eb
class UniqueKeyLoader(yaml.SafeLoader):
    def construct_mapping(self, node, deep=False):
        mapping = []
        for key_node, value_node in node.value:
            key = self.construct_object(key_node, deep=deep)
            if key in mapping:
                print(f"Warning!! {key} is duplicated!")
            assert key not in mapping
            mapping.append(key)
        return super().construct_mapping(node, deep)

def parseTranslationStrings(yamlfile):
    if not os.path.isfile(yamlfile):
        exit(f"Unable to find {yamlfile}")

    yaml_strings = {}
    with open(yamlfile, "r", encoding="utf-8") as yaml_file:
        # Enforce a new line at the end of the file
        last_line = yaml_file.readlines()[-1]
        if last_line == last_line.rstrip():
            exit("The yaml file must have an empty line at the end")

        # Reset position after reading the whole content
        yaml_file.seek(0)
        yaml_content = yaml.load(yaml_file, UniqueKeyLoader)
        if yaml_content is None:
            return yaml_strings

        if type(yaml_content) is not dict:
            exit(f"The {yamlfile} file must contain collections only")

        for category in yaml_content:
            for key in yaml_content[category]:
                string_id = f"vpn.{category}.{key}"
                obj = yaml_content[category][key]
                value = []
                comments = []

                if type(obj) is str:
                    if len(obj) == 0:
                        stop(string_id)
                    value = [obj]

                elif type(obj) is dict:
                    if not ("value" in obj):
                        exit(
                            f"The key {string_id} must contain a `value` string or an array of strings"
                        )

                    if type(obj["value"]) is str:
                        value = [obj["value"]]

                    elif type(obj["value"]) is list:
                        for x in range(0, len(obj["value"])):
                            value.append(obj["value"][x])

                    else:
                        exit(
                            f"The value of {string_id} must be a string or an array of strings"
                        )

                    if "comment" in obj:
                        if type(obj["comment"]) is str:
                            comments = [obj["comment"]]

                        elif type(obj["comment"]) is list:
                            for x in range(0, len(obj["comment"])):
                                comments.append(obj["comment"][x])

                        else:
                            exit(
                                f"The comment of {string_id} must be a string or an array of strings"
                            )

                    if len(value) == 0:
                        stop(string_id)

                else:
                    stop(string_id)

                yaml_strings[pascalize(f"{category}_{key}")] = {
                    "string_id": string_id,
                    "value": value,
                    "comments": comments,
                }
        
        return yaml_strings

## Parse the strings from a guide block and append them to the output dictionary.
def parseGuideBlock(block, guide_id, output, filename):
    if not "id" in block:
        exit(f"Guide {filename} does not have an id for one of the blocks")
    if not "type" in block:
        exit(f"Guide {filename} does not have a type for block id {block['id']}")
    if not "content" in block:
        exit(f"Guide {filename} does not have a content for block id {block['id']}")

    block_id = block['id']
    block_enum = pascalize(f"guide_{guide_id}_block_{block_id}")
    block_default_comment = comment_types.get(block['type'], "")
    if block_enum in output:
        exit(f"Duplicate block enum {block_enum} when parsing {filename}")

    if not isinstance(block["content"], list):
        output[block_enum] = {
            "string_id": f"guide.{guide_id}.block.{block_id}",
            "value": [block["content"]],
            "comments": [block.get("comment", block_default_comment)],
        }
        return
    
    for subblock in block["content"]:
        if not "id" in subblock:
            exit(f"Guide {filename} does not have an id for one of the subblocks of block {block_id}")
        if not "content" in subblock:
            exit(f"Guide file {filename} does not have a content for subblock id {subblock['id']}")
        
        subblock_id = subblock['id']
        subblock_enum = pascalize(f"guide_{guide_id}_block_{block_id}_{subblock_id}")
        if subblock_enum in output:
            exit(f"Duplicate sub-block enum {subblock_enum} when parsing {filename}")

        output[subblock_enum] = {
            "string_id": f"guide.{guide_id}.block.{block_id}.{subblock_id}",
            "value": [subblock["content"]],
            "comments": [subblock.get("comment", block_default_comment)],
        }

## Parse the strings from a JSON guide and return them as a dictionary.
def parseGuideJson(guide_json, filename):
    guide_strings = {}

    if not "id" in guide_json:
        exit(f"Guide {filename} does not have an id")
    if not "title" in guide_json:
        exit(f"Guide {filename} does not have a title")
    if not "subtitle" in guide_json:
        exit(f"Guide {filename} does not have a subtitle")
    if not "blocks" in guide_json:
        exit(f"Guide {filename} does not have a blocks")

    guide_id = guide_json['id']
    title_enum = pascalize(f"guide_{guide_id}_title")
    guide_strings[title_enum] = {
        "string_id": f"guide.{guide_id}.title",
        "value": [guide_json["title"]],
        "comments": [guide_json.get("title_comment", "Title for a guide view")],
    }
    subtitle_enum = pascalize(f"guide_{guide_id}_subtitle")
    guide_strings[subtitle_enum] = {
        "string_id": f"guide.{guide_id}.subtitle",
        "value": [guide_json["subtitle"]],
        "comments": [guide_json.get("subtitle_comment", "Subtitle for a guide view")],
    }

    for block in guide_json["blocks"]:
        parseGuideBlock(block, guide_id, guide_strings, filename)

    return guide_strings

## Parse a directory of JSON guides, returning their combined strings as a dictionary
def parseGuideStrings(guidepath):
    guide_strings = {}
    for filename in os.listdir(guidepath):
        if not filename.endswith(".json"):
            continue

        with open(os.path.join(guidepath, filename), "r", encoding="utf-8") as fp:
            guide_json = json.load(fp)

            for key, value in parseGuideJson(guide_json, filename).items():
                if key in guide_strings:
                    exit(f"Duplicate enum {key} when parsing {filename}")
                guide_strings[key] = value

    return guide_strings

## Parse the strings from a JSON tutorial and return them as a dictionary.
def parseTutorialJson(tutorial_json, filename):
    tutorial_strings = {}

    if not "id" in tutorial_json:
        exit(f"Tutorial {filename} does not have an id")
    if not "title" in tutorial_json:
        exit(f"Tutorial {filename} does not have a title")
    if not "subtitle" in tutorial_json:
        exit(f"Tutorial {filename} does not have a subtitle")
    if not "completion_message" in tutorial_json:
        exit(f"Tutorial {filename} does not have a completion message")
    if not "steps" in tutorial_json:
        exit(f"Tutorial {filename} does not have steps")

    tutorial_id = tutorial_json['id']
    title_enum = pascalize(f"tutorial_{tutorial_id}_title")
    tutorial_strings[title_enum] = {
        "string_id": f"tutorial.{tutorial_id}.title",
        "value": [tutorial_json["title"]],
        "comments": [tutorial_json.get("title_comment", "Title for a tutorial view")],
    }

    subtitle_enum = pascalize(f"tutorial_{tutorial_id}_subtitle")
    tutorial_strings[subtitle_enum] = {
        "string_id": f"tutorial.{tutorial_id}.subtitle",
        "value": [tutorial_json["subtitle"]],
        "comments": [tutorial_json.get("subtitle_comment", "Subtitle for a tutorial view")],
    }

    completion_enum = pascalize(f"tutorial_{tutorial_id}_completion_message")
    tutorial_strings[completion_enum] = {
        "string_id": f"tutorial.{tutorial_id}.completion_message",
        "value": [tutorial_json["completion_message"]],
        "comments": [tutorial_json.get("completion_message_comment", "Completion message for a tutorial view")],
    }

    for step in tutorial_json["steps"]:
        if not "id" in step:
            exit(f"Tutorial {filename} does not have an id for one of the steps")
        if not "tooltip" in step:
            exit(f"Tutorial {filename} does not have a tooltip for step id {step['id']}")

        step_id = step['id']
        step_enum = pascalize(f"tutorial_{tutorial_id}_step_{step_id}")
        if step_enum in tutorial_strings:
            exit(f"Duplicate step enum {step_enum} when parsing {filename}")

        tutorial_strings[step_enum] = {
            "string_id": f"tutorial.{tutorial_id}.step.{step_id}",
            "value": [step["tooltip"]],
            "comments": [step.get("comment", "A tutorial step tooltip")],
        }

    return tutorial_strings

## Parse a directory of JSON tutorials, returning their combined strings as a dictionary
def parseTutorialStrings(tutorialpath):
    tutorial_strings = {}
    for filename in os.listdir(tutorialpath):
        if not filename.endswith(".json"):
            continue

        with open(os.path.join(tutorialpath, filename), "r", encoding="utf-8") as fp:
            tutorial_json = json.load(fp)

            for key, value in parseTutorialJson(tutorial_json, filename).items():
                if key in tutorial_strings:
                    exit(f"Duplicate enum {key} when parsing {filename}")
                tutorial_strings[key] = value

    return tutorial_strings

## Render a dictionary of strings into the l18nstrings module.
def generateStrings(strings, outdir):
    os.makedirs(outdir, exist_ok=True)
    with open(os.path.join(outdir, "l18nstrings.h"), "w", encoding="utf-8") as output:
        output.write(
            """/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// AUTOGENERATED! DO NOT EDIT!!

#ifndef L18NSTRINGS_H
#define L18NSTRINGS_H

#include <QQmlPropertyMap>

class L18nStrings final : public QQmlPropertyMap {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(L18nStrings)

 public:
  enum String {
    Empty,
"""
        )

        for key in strings:
            output.write(f"    {key},\n")

        output.write(
            """    __Last,
  };

  static L18nStrings* instance();
  static void initialize();

  explicit L18nStrings(QObject* parent);
  ~L18nStrings() = default;

  void retranslate();

  QString t(String) const;

 private:
  static const char* const _ids[];
};

#endif  // L18NSTRINGS_H
"""
        )

    with open(os.path.join(outdir, "l18nstrings_p.cpp"), "w", encoding="utf-8") as output:
        output.write(
            """/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// AUTOGENERATED! DO NOT EDIT!!

#include "l18nstrings.h"

// static
const char* const L18nStrings::_ids[] = {
    // The first ID is for the empty string.
    "",

"""
        )

        def serialize(string):
            ret = "\\n".join(string)
            return ret.replace('"', '\\"')

        for key, data in strings.items():
            output.write(f"    //% \"{serialize(data['value'])}\"\n")
            for comment in data["comments"]:
                output.write(f"    //: {comment}\n")
            output.write(f"    QT_TRID_NOOP(\"{data['string_id']}\"),\n\n")

        # This is done to make windows compiler happy
        if len(strings) == 0:
            output.write(f'    "vpn.dummy.ignore",\n\n')

        output.write(
            """
};

"""
        )

        # Generate the retranslate() method.
        output.write("void L18nStrings::retranslate() {\n")
        for key in strings:
            output.write(f"    insert(\"{key}\", qtTrId(_ids[{key}]));\n")
        output.write("}")


if __name__ == "__main__":
    ## Parse arguments to locate the input and output files.
    parser = argparse.ArgumentParser(
        description='Generate internationaliation strings database from a YAML source')
    parser.add_argument('source', metavar='SOURCE', type=str, action='store', nargs='?',
        help='YAML strings file to process')
    parser.add_argument('-o', '--output', metavar='DIR', type=str, action='store',
        help='Output directory for generated files')
    parser.add_argument('-g', '--guides', metavar='DIR', type=str, action='store',
        help='Parse JSON guides from DIR')
    parser.add_argument('-t', '--tutorials', metavar='DIR', type=str, action='store',
        help='Parse JSON tutorials from DIR')
    args = parser.parse_args()

    ## If no source was provided, find it relative to this script file.
    if args.source is None:
        rootpath = os.path.abspath(os.path.join(os.path.dirname(__file__), os.pardir, os.pardir))
        args.source = os.path.join('translations', 'strings.yaml')
    
    ## If no output directory was provided, use the current directory.
    if args.output is None:
        args.output = os.getcwd()
    
    ## Parse the inputs for their sweet juicy strings.
    strings = parseTranslationStrings(args.source)
    if args.guides:
        strings.update(parseGuideStrings(args.guides))
    if args.tutorials:
        strings.update(parseTutorialStrings(args.tutorials))
    
    ## Render the strings into generated content.
    generateStrings(strings, args.output)
