#! /usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import argparse
import json
from jsonschema import validate, RefResolver
import os
from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument('jsonSchema', metavar='JSONSchema', type=str, nargs=1,
                    help='the JSON Schema file to be used. The file must be stored in `scripts/ci/jsonSchemas`')
parser.add_argument('path', metavar='path', type=str, nargs='+',
                    help='the path containing the JSON files to be validated.')
args = parser.parse_args()

def validateFile(file, schema, resolver):
  print(f'Validating {file}...')
  with open(file, "r", encoding="utf-8") as file:
    json_data = json.load(file)
    validate(json_data, schema=schema, resolver=resolver)

jsonSchema = os.path.join(os.getcwd(), "scripts", "ci", "jsonSchemas", args.jsonSchema[0])
if not os.path.isfile(jsonSchema):
  exit(f"The JSONSchema {jsonSchema} does not exist")

with open(jsonSchema, "r", encoding="utf-8") as schema:
  schema = json.load(schema)
  resolver = RefResolver(Path(jsonSchema).as_uri(), "")

  for path in args.path:
    validateFile(path, schema, resolver)
