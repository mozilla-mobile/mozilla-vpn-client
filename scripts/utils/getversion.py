#! /usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import os
import re
import sys

# Some regex
# This is pretty crude and it won't catch exotic string escaping,
# but it works well enough against our CMakeLists.txt
prjstart = re.compile('^\s*project\s*\(', flags=re.IGNORECASE)
prjend = re.compile('^.*\)')

# Parse the top-level CMakeLists.txt
cmakefile = os.path.join(os.path.dirname(__file__), '..', '..', 'CMakeLists.txt')
with open(cmakefile, 'r') as fp:
    # Find the project() command
    project = None
    prjline = 1
    for line in fp.readlines():
        if project is not None:
            project += ' ' + line.strip()
        elif prjstart.match(line):
            project = line.strip()
        else:
            prjline += 1
        
        if project and prjend.match(line):
            break
    
    if project is None:
        print(f'Failed to find a project() definition in {os.path.abspath(cmakefile)}', file=sys.stfderr)
        sys.exit(1)

    # Print out whatever token we find after 'VERSION'
    tokens = project.split()
    for i in range(len(tokens) - 1):
        if tokens[i] == 'VERSION':
            print(tokens[i+1])
            sys.exit(0)
    
    # Otherwise, there was no VERSION that we could parse
    print(f'Failed to find a VERSION at {os.path.abspath(cmakefile)}:{prjline}', file=sys.stderr)
    sys.exit(1)
