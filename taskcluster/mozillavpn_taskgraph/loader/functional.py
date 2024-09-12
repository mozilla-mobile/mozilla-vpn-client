# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


import logging
import os

from taskgraph.loader.transform import loader as transform_loader
from taskgraph.util.copy import deepcopy
from taskgraph.util.yaml import load_yaml

logger = logging.getLogger(__name__)

def enumerate_test_scripts(path):
    """
    Generate a list of functional test scripts
    """
    listing = os.listdir(os.path.join(path, '..', '..', '..', 'tests', 'functional'))
    return [name for name in listing if name.startswith('test') and name.endswith('.js')]

def script2words(script):
    # Strip the 'test' prefix and '.js' suffix.
    camelcase = script[4:-3]
    words = []
    for char in camelcase:
        if len(words) != 0 and char.islower():
            words[-1] += char
        else:
            words.append(char.lower())

    return words

def script2phrase(script):
    return " ".join(script2words(script))

def script2name(script):
    # Rename some tests that are too long.
    # The name must be less than 25 chars long.
    rename = {
        'authentication': 'auth',
        'errors': 'err',
    }
    return '-'.join(
        rename[word] if word in rename else word
        for word in script2words(script)
    )

def loader(kind, path, config, params, loaded_tasks):
    """
    Generate tasks for the VPN functional tests.
    """
    logger.info(f"Hello world from {__file__}")

    scripts = enumerate_test_scripts(path)

    for platform in transform_loader(kind, path, config, params, loaded_tasks):
        logger.info(f'Found test platform {platform["name"]}')
        for script in scripts:
            test = deepcopy(platform)
            test["name"] = f'{platform["name"]}-{script2name(script)}'
            test["description"] += f' {script2phrase(script)} ({platform["name"]})'
            test["treeherder"]["symbol"] = f'T({script2name(script)})'

            yield test
