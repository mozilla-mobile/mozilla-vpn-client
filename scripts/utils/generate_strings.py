#! /usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import re
import os
import yaml
import json

string_ids = []

comment_types = {
    "text": f"Standard text in a guide block",
    "title": f"Title in a guide block",
    "list": f"Bullet list item in a guide block",
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


def parseTranslationStrings():
    translations_path = os.path.abspath(
        os.path.join(os.path.dirname(__file__), os.pardir, os.pardir, "translations")
    )
    yaml_path = os.path.join(translations_path, "strings.yaml")

    if not os.path.isfile(yaml_path):
        exit("Unable to find translations/strings.yaml")

    with open(yaml_path, "r", encoding="utf-8") as yaml_file:
        yaml_content = yaml.load(yaml_file, UniqueKeyLoader)

        if yaml_content is not None:
            if type(yaml_content) is not dict:
                exit("The yaml file must contain collections only")

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

                    string_ids.append(
                        {
                            "enum_id": pascalize(f"{category}_{key}"),
                            "string_id": string_id,
                            "value": value,
                            "comments": comments,
                        }
                    )


def parseGuideStrings():
    guides_path = os.path.abspath(
        os.path.join(
            os.path.dirname(__file__), os.pardir, os.pardir, "src", "ui", "guides"
        )
    )

    guide_ids = []

    for guide_filename in os.listdir(guides_path):
        if not guide_filename.endswith(".json"):
            continue
        with open(os.path.join(guides_path, guide_filename), "r", encoding="utf-8") as guide_file:
            guide_json = json.load(guide_file)
        if not "id" in guide_json:
            exit(f"Guide file {guide_filename} does not have an id")
        if not "title" in guide_json:
            exit(f"Guide file {guide_filename} does not have a title")

        enum_id = pascalize(f"guide_{guide_json['id']}_title")
        if enum_id in guide_ids:
            exit(f"Duplicate id {enum_id} when parsing {guide_filename}")
        guide_ids.append(enum_id)

        comment = "Title for a guide view"
        if "comment" in guide_json:
            comment = guide_json["comment"]

        string_ids.append(
            {
                "enum_id": enum_id,
                "string_id": f"guide.{guide_json['id']}.title",
                "value": [guide_json["title"]],
                "comments": [comment],
            }
        )

        if not "blocks" in guide_json:
            exit(f"Guide file {guide_filename} does not have a blocks")
        for block in guide_json["blocks"]:
            if not "id" in block:
                exit(
                    f"Guide file {guide_filename} does not have an id for one of the blocks"
                )
            if not "type" in block:
                exit(
                    f"Guide file {guide_filename} does not have a type for block id {block['id']}"
                )
            if not "content" in block:
                exit(
                    f"Guide file {guide_filename} does not have a content for block id {block['id']}"
                )
            enum_id = pascalize(f"guide_{guide_json['id']}_block_{block['id']}")
            if enum_id in guide_ids:
                exit(
                    f"Duplicate id {enum_id} when parsing {guide_filename} - block {block['id']}"
                )
            guide_ids.append(enum_id)

            if isinstance(block["content"], list):
                for subblock in block["content"]:
                    if not "id" in subblock:
                        exit(
                            f"Guide file {guide_filename} does not have an id for one of the subblocks of block {block['id']}"
                        )
                    if not "content" in subblock:
                        exit(
                            f"Guide file {guide_filename} does not have a content for subblock id {subblock['id']}"
                        )
                    enum_id = pascalize(
                        f"guide_{guide_json['id']}_block_{block['id']}_{subblock['id']}"
                    )
                    if enum_id in guide_ids:
                        exit(
                            f"Duplicate id {enum_id} when parsing {guide_filename} - subblock {subblock['id']}"
                        )
                    guide_ids.append(enum_id)

                    comment = comment_types.get(block["type"], "")
                    if "comment" in subblock:
                        comment = subblock["comment"]

                    string_ids.append(
                        {
                            "enum_id": enum_id,
                            "string_id": f"guide.{guide_json['id']}.block.{block['id']}.{subblock['id']}",
                            "value": [subblock["content"]],
                            "comments": [comment],
                        }
                    )
            else:
                comment = comment_types.get(block["type"], "")
                if "comment" in block:
                    comment = block["comment"]

                string_ids.append(
                    {
                        "enum_id": enum_id,
                        "string_id": f"guide.{guide_json['id']}.block.{block['id']}",
                        "value": [block["content"]],
                        "comments": [comment],
                    }
                )


def parseTutorialStrings():
    tutorials_path = os.path.abspath(
        os.path.join(
            os.path.dirname(__file__), os.pardir, os.pardir, "src", "ui", "tutorials"
        )
    )

    tutorial_ids = []

    for tutorial_filename in os.listdir(tutorials_path):
        if not tutorial_filename.endswith(".json"):
            continue
        with open(os.path.join(tutorials_path, tutorial_filename), "r", encoding="utf-8") as tutorial_file:
            tutorial_json = json.load(tutorial_file)
        if not "id" in tutorial_json:
            exit(f"Tutorial file {tutorial_filename} does not have an id")
        if not "title" in tutorial_json:
            exit(f"Tutorial file {tutorial_filename} does not have a title")

        enum_id = pascalize(f"tutorial_{tutorial_json['id']}_title")
        if enum_id in tutorial_ids:
            exit(f"Duplicate id {enum_id} when parsing {tutorial_filename}")
        tutorial_ids.append(enum_id)

        comment = "Title for a tutorial view"
        if "comment" in tutorial_json:
            comment = tutorial_json["comment"]

        string_ids.append(
            {
                "enum_id": enum_id,
                "string_id": f"tutorial.{tutorial_json['id']}.title",
                "value": [tutorial_json["title"]],
                "comments": [comment],
            }
        )

        if not "steps" in tutorial_json:
            exit(f"Tutorial file {tutorial_filename} does not have a steps")
        for step in tutorial_json["steps"]:
            if not "id" in step:
                exit(
                    f"Tutorial file {tutorial_filename} does not have an id for one of the steps"
                )
            if not "tooltip" in step:
                exit(
                    f"Tutorial file {tutorial_filename} does not have a tooltip for step id {step['id']}"
                )
            enum_id = pascalize(f"tutorial_{tutorial_json['id']}_step_{step['id']}")
            if enum_id in tutorial_ids:
                exit(
                    f"Duplicate id {enum_id} when parsing {tutorial_filename} - step {step['id']}"
                )
            tutorial_ids.append(enum_id)

            comment = "A tutorial step tooltip"
            if "comment" in step:
                comment = step["comment"]

            string_ids.append(
                {
                    "enum_id": enum_id,
                    "string_id": f"tutorial.{tutorial_json['id']}.step.{step['id']}",
                    "value": [step["tooltip"]],
                    "comments": [comment],
                }
            )


def writeOutputFiles():
    translations_path = os.path.abspath(
        os.path.join(os.path.dirname(__file__), os.pardir, os.pardir, "translations")
    )
    with open(
        os.path.join(translations_path, "generated", "l18nstrings.h"),
        "w",
        encoding="utf-8",
    ) as output:
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

        for string in string_ids:
            output.write(f"    {string['enum_id']},\n")

        output.write(
            """    __Last,
  };

  static L18nStrings* instance();

  explicit L18nStrings(QObject* parent);
  ~L18nStrings();

  void retranslate();

  QString t(String) const;

 private:
  static const char* const _ids[];
};

#endif  // L18NSTRINGS_H
"""
        )

    with open(
        os.path.join(translations_path, "generated", "l18nstrings_p.cpp"),
        "w",
        encoding="utf-8",
    ) as output:
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

        for string in string_ids:
            output.write(f"    //% \"{serialize(string['value'])}\"\n")
            for comment in string["comments"]:
                output.write(f"    //: {comment}\n")
            output.write(f"    QT_TRID_NOOP(\"{string['string_id']}\"),\n\n")

        # This is done to make windows compiler happy
        if len(string_ids) == 0:
            output.write(f'    "vpn.dummy.ignore",\n\n')

        output.write(
            """
};

void L18nStrings::retranslate() {
"""
        )

        for string in string_ids:
            output.write(
                f"    insert(\"{string['enum_id']}\", qtTrId(_ids[{string['enum_id']}]));\n"
            )
        output.write("}")


def generateStrings():
    parseTranslationStrings()
    parseGuideStrings()
    parseTutorialStrings()
    writeOutputFiles()


if __name__ == "__main__":
    generateStrings()
