#! /usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import argparse
import os
import re

# This file checks to make sure all images in QML use the style that allows for different assets for dark mode and light mode. More information available in `assets.md`.

IMAGE_REGEX = r"qrc.*(svg)|(png)"
IMAGE_DEF_REGEX = r"'.*': \{"
IMAGE_USE_REGEX = r"MZAssetLookup\.getImageSource\(\".*?\"\)"

def fileContents(filepath):
    try:
        with open(filepath, 'r') as file:
            content = file.read()
            if not content:
                exit(f"No content found in: {filepath}")        
            return content
    except FileNotFoundError:
        exit(f"File not found: {filepath}")
    except Exception as e:
        exit(f"An error occurred while loading {filepath}: {e}")

def checkForExplicitImages(filepath):
    # print("Checking for explicit images in " + filepath)
    content = fileContents(filepath)
    image_list = re.findall(IMAGE_REGEX, content)
    
    # We expect 2 failing RegExs in ServerLabel due to country flags
    if "ServerLabel.qml" in filepath and len(image_list) == 2:
        return
    # We expect 1 failing RegExs in ServerCountry due to country flags
    if "ServerCountry.qml" in filepath and len(image_list) == 1:
        return
    
    if image_list:
        exit(f"Explict image found in: {filepath}")

def imagesDefinedInFile(filepath):
    # print("Loading images defined in " + filepath)
    content = fileContents(filepath)
    image_list = re.findall(IMAGE_DEF_REGEX, content)
    if not image_list:
        exit(f"No images found in: {filepath}")
    # Just keep the image name
    image_list = list(map(lambda x: x.split("'")[1], image_list))
    return image_list

def imagesUsedInFile(filepath):
    # print("Loading images used in " + filepath)
    content = fileContents(filepath)
    image_list = re.findall(IMAGE_USE_REGEX, content)
    # Remove the 'MZAssetLookup.getImageSource("' at the start of each line and the '")' at the end of each line
    image_list = list(map(lambda x: x.split("\"")[1], image_list))
    return image_list

def getQmlFiles(directory):
    print("Loading all QML files in " + directory)
    qml_files = []
    try:
        for root, _, files in os.walk(directory):
            for file in files:
                if file.endswith(".qml"):
                    qml_files.append(os.path.join(root, file))
        if not qml_files:
            exit(f"No QML files found in: {directory}")
        return qml_files
    except FileNotFoundError:
        exit(f"Directory not found: {directory}")
    except Exception as e:
        exit(f"An error occurred when getting QML files in {directory}: {e}")

###
# 0. Prepare items needed for tests

# A. Get list of all named images in the image dictionary
script_path = os.path.dirname(os.path.abspath(__file__))
asset_lookup_path = os.path.join(script_path, os.path.pardir, os.path.pardir, "nebula", "ui", "utils")
image_list = imagesDefinedInFile(asset_lookup_path + "/MZAssetLookup.js")

# B. Get list of QML files to check
#    (All QML files are in one of these 3 directories or a subdirectory of one: nebula/ui/compat, nebula/ui/components, src/ui)
script_path = os.path.dirname(os.path.abspath(__file__))
src_ui_path = os.path.join(script_path, os.path.pardir, os.path.pardir, "src", "ui")
nebula_compat_path = os.path.join(script_path, os.path.pardir, os.path.pardir, "nebula", "ui", "compat")
nebula_component_path = os.path.join(script_path, os.path.pardir, os.path.pardir, "nebula", "ui", "components")
qml_directories = [src_ui_path, nebula_compat_path, nebula_component_path]
all_qml_files = []
for direc in qml_directories:
    all_qml_files = all_qml_files + getQmlFiles(direc)

###
# 1. All images in MZAssetLookup.js must have a distinct name - no duplicates
if len(image_list) is not len(set(image_list)):
  exit(f"MZAssetLookup.imageLookup contains at least one name used multiple times")
else:
  print("No duplicate image names found!")

###
# 2. Check that QML files do not include explicit images, and only use intended images
for qml_file_path in all_qml_files:
    # check for explict images - .svg and .png should not appear in code
    checkForExplicitImages(qml_file_path)

    # get image names from QML file
    images_used = imagesUsedInFile(qml_file_path)
    # confirm the image names are all on the approved list
    for image in images_used:
        if image not in image_list:
            exit(f"Unexpected image {image} found in {qml_file_path}")

print("All QML files used images properly.")
