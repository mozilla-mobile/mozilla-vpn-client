#! /usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import os
import sys
import xml.etree.ElementTree as ET

def validateFile(file):
  print(f'Validating {file}...')

  tree = ET.parse(file)
  root = tree.getroot()

  files = []
  for node in root.findall(".//file"):
    if (node.text in files):
      sys.exit(f"{node.text} duplicated in {file}!")
    files.append(node.text)


for root,d_names,f_names in os.walk("."):
  for f in f_names:
    if f.endswith(".qrc") and not ("tests" in root):
      path = os.path.join(root, f)
      validateFile(path)
