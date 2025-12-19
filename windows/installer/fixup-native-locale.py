# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import sys
import xml.etree.ElementTree as ET

tree = ET.parse(sys.argv[1])
del tree.getroot().attrib["Culture"]
tree.write(sys.argv[1])

