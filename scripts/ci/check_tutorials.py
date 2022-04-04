#! /usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import os
import json
from jsonschema import validate

schema = {
  "$schema": "https://json-schema.org/draft/2020-12/schema",
  "type": "object",
  "properties": {
    "id": {
      "type": "string",
      "description": "The ID of the tutorial",
      "pattern": "^[A-Za-z0-9_]+"
    },
    "title": {
      "type": "string",
      "description": "The title of the tutorial",
    },
    "comment": {
      "type": "string",
      "description": "A comment to describe how to translate the title",
    },
    "image": {
      "type": "string",
      "description": "The image of the tutorial",
    },
    "steps": {
      "type": "array",
      "description": "The list of text steps",
      "items": {
        "type": "object",
        "properties": {
          "id": {
            "type": "string",
            "description": "The ID of the step",
            "pattern": "^[A-Za-z0-9_]+"
          },
          "comment": {
            "type": "string",
            "description": "A comment to describe how to translate this step content",
          },
          "element": {
            "type": "string",
            "description": "The objectName to highlight",
          },
          "tooltip": {
            "type": "string",
            "description": "The message to show near the element",
          },
        },
        "required": [ "id", "element", "tooltip" ]
      }
    },
  },
  "required": [ "id", "title", "image", "steps" ],
}

def validateFile(file):
  print(f'Validating {file}...')
  with open(file, "r", encoding="utf-8") as file:
    tutorial_json = json.load(file)
    validate(tutorial_json, schema=schema)

path = os.path.join("src", "ui", "tutorials")
for root,d_names,f_names in os.walk(path):
  for f in f_names:
    if f.endswith(".json"):
      validateFile(os.path.join(path, f))
