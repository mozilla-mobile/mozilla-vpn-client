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
      "description": "The ID of the guide",
      "pattern": "^[A-Za-z0-9_]+"
    },
    "title": {
      "type": "string",
      "description": "The title of the guide",
    },
    "comment": {
      "type": "string",
      "description": "A comment to describe how to translate the title",
    },
    "image": {
      "type": "string",
      "description": "The image of the guide",
    },
    "blocks": {
      "type": "array",
      "description": "The list of text blocks",
      "items": {
        "type": "object",
        "properties": {
          "id": {
            "type": "string",
            "description": "The ID of the block",
            "pattern": "^[A-Za-z0-9_]+"
          },
          "content": {
            "anyOf": [
              {
                "type": "string",
              },
              {
                "type": "array",
                "items": {
                  "type": "object",
                  "properties": {
                    "id": {
                      "type": "string",
                      "description": "The ID of the sub block",
                      "pattern": "^[A-Za-z0-9_]+"
                    },
                    "content": {
                      "type": "string",
                      "description": "The content of this sub block",
                    },
                    "comment": {
                      "type": "string",
                      "description": "A comment to describe how to translate this sub block",
                    },
                  },
                  "required": [ "id", "content" ],
                },
              },
            ],
            "description": "The content of this block",
          },
          "comment": {
            "type": "string",
            "description": "A comment to describe how to translate this block content",
          },
        },
        "required": [ "id", "content" ]
      }
    },
  },
  "required": [ "id", "title", "image", "blocks" ],
}

def validateFile(file):
  print(f'Validating {file}...')
  with open(file, "r", encoding="utf-8") as file:
    guide_json = json.load(file)
    validate(guide_json, schema=schema)

path = os.path.join("src", "ui", "guides")
for root,d_names,f_names in os.walk(path):
  for f in f_names:
    if f.endswith(".json"):
      validateFile(os.path.join(path, f))
