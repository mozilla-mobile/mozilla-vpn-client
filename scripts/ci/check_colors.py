#! /usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import argparse
import os
import re

HEX_COLOR_REGEX = r"#[0-9a-fA-F]{6}"
COLOR_DEF_REGEX = r"color\..* ="
COLOR_USE_REGEX = r"MZTheme\.colors\.[0-9A-Za-z]+"

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

def checkForHexColor(filepath):
    # print("Checking for hex colors in " + filepath)
    content = fileContents(filepath)
    hex_color_list = re.findall(HEX_COLOR_REGEX, content)
    if hex_color_list:
        exit(f"Unexpected hex color found in: {filepath}")

def colorsDefinedInFile(filepath):
    print("Loading colors defined in " + filepath)
    content = fileContents(filepath)
    color_list = re.findall(COLOR_DEF_REGEX, content)
    if not color_list:
        exit(f"No colors found in: {filepath}")
    # Remove the " =" at the end of each line
    color_list = list(map(lambda x: x.split(" ")[0], color_list))
    # Remove the "color." at the start
    color_list = list(map(lambda x: x.split(".")[1], color_list))
    return color_list

def colorsUsedInFile(filepath):
    # print("Loading colors used in " + filepath)
    content = fileContents(filepath)
    color_list = re.findall(COLOR_USE_REGEX, content)
    # Remove the "MZTheme.colors." at the start of each line
    color_list = list(map(lambda x: x.split(".")[2], color_list))
    return color_list

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

parser = argparse.ArgumentParser()
parser.add_argument('themeDirectory', metavar='themeDirectory', type=str, nargs=1,
                    help='The directory for all theming')
args = parser.parse_args()

###
# 0. Get list of all themes
try:
    all_themes = [item for item in os.listdir(args.themeDirectory[0]) if os.path.isdir(os.path.join(args.themeDirectory[0], item))]
except FileNotFoundError:
    exit(f"Path not found: {args.themeDirectory[0]}")
except Exception as e:
    exit(f"An error occurred when finding all themes: {e}")

if len(all_themes) == 0:
    exit(f"No themes found")

###
# 1. All theme's `theme.js` should have the same colors
file_paths = list(map(lambda x: args.themeDirectory[0] + "/" + x + "/theme.js", all_themes))
baseline_colors = colorsDefinedInFile(file_paths[0])
for file_path in file_paths:
    colors_in_file = colorsDefinedInFile(file_path)
    if len(colors_in_file) != len(baseline_colors):
        exit(f"Themes should define identical list of colors, but have different count of colors")
    for color in colors_in_file:
        if color not in baseline_colors:
            exit(f"Color {color} is not in all themes but found in {file_path}")

###
# 2. Explict colors should not be used in theme-derived.js or any theme's theme.js file.

file_paths = list(map(lambda x: args.themeDirectory[0] + "/" + x + "/theme.js", all_themes))
file_paths.append(args.themeDirectory[0] + "/theme-derived.js")
for file_path in file_paths:
    checkForHexColor(file_path)

###
# 3. All colors in QML files should come from theme.js or theme-derived.js.
#    (For objects - like `iconButtonDarkBackground` - only the overall object name is checked.)

# A. Get list of colors that should be used
#   (Each theme's theme.js file should expose the same color names, so can just check one.)
color_list_derived = colorsDefinedInFile(args.themeDirectory[0] + "/theme-derived.js")
color_list_theme = colorsDefinedInFile(args.themeDirectory[0] + "/" + all_themes[0] + "/theme.js")
# `transparent` is defined in colors.js, but allowed to be used in code per many comments
color_list = color_list_derived + color_list_theme + ["transparent"]

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

# C. Check that QML files do not include explict colors, and only use intended colors
for qml_file_path in all_qml_files:
    # check for explict colors
    checkForHexColor(qml_file_path)

    # get color names from QML file - everything MZTheme.color
    colors_used = colorsUsedInFile(qml_file_path)
    # confirm the color names are all on the approved list
    for color in colors_used:
        if color not in color_list:
            exit(f"Unexpected color {color} found in {qml_file_path}")
