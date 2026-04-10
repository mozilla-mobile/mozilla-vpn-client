#!/usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

"""
Generate AppShortcuts.xcstrings and Localizable.xcstrings from strings.yaml
and the i18n XLIFF translation files.

Usage:
    python3 generate_xcstrings.py <strings.yaml> <i18n_dir> <output_dir>

Example:
    python3 scripts/utils/generate_xcstrings.py \
        src/translations/strings.yaml \
        3rdparty/i18n \
        src/translations
"""

import argparse
import glob
import json
import os
import re
import sys
import xml.etree.ElementTree as ET
from generate_strings import parseYAMLTranslationStrings

XLIFF_NS = '{urn:oasis:names:tc:xliff:document:1.2}'

def load_xliff_translations(xliff_path, isEnglish):
    """Returns dict of {string_id: translated_string} for one locale's XLIFF file."""
    if not os.path.isfile(xliff_path):
        print(f"XLIFF file not found: {xliff_path}")
        exit(1)
    translations = {}
    tree = ET.parse(xliff_path)
    for unit in tree.getroot().iter(f'{XLIFF_NS}trans-unit'):
        trid = unit.attrib.get('id', '')
        if isEnglish:
          translations[trid] = unit.findtext(f'{XLIFF_NS}source', default='')
        else:
          translations[trid] = unit.findtext(f'{XLIFF_NS}target', default='')
    if not translations:
        print(f"No translations found: {xliff_path}")
        exit(1)
    return translations

SPECIAL_LOCALE_MAP = {
    'zh_CN': 'zh-Hans',
    'zh_TW': 'zh-Hant',
}

def normalize_locale(locale):
    if locale in SPECIAL_LOCALE_MAP:
        return SPECIAL_LOCALE_MAP[locale]
    return locale.replace('_', '-')


def get_locales(i18n_dir):
    """Returns list of locale codes that have a mozillavpn.xliff."""
    locales = []
    for entry in os.listdir(i18n_dir):
        xliff = os.path.join(i18n_dir, entry, 'mozillavpn.xliff')
        if os.path.isfile(xliff):
            locales.append(entry)
    if not locales:
        print(f"No other locales found")
        exit(1)
    return locales


def build_localizable_xcstrings(main_strings, locale_translations):
    """Build Localizable.xcstrings dict for iosAppIntents.main.* strings."""
    strings = {}

    for data in main_strings.values():
        english_value = data['value'][0]
        string_id = data['string_id']
        comment = data['comments'][0] if data['comments'] else None

        entry = {}
        if comment:
            entry['comment'] = comment

        localizations = {}
        for locale, translations in locale_translations.items():
            translated = translations.get(string_id)
            if translated:
                for placeholder in re.findall(r'%.{0,2}@', english_value):
                    if placeholder not in translated:
                        print(f"Error: placeholder '{placeholder}' from missing from {locale} translation of '{string_id}'")
                        exit(1)
                localizations[normalize_locale(locale)] = {
                    'stringUnit': {
                        'state': 'translated',
                        'value': translated
                    }
                }
        if localizations:
            entry['localizations'] = localizations

        strings[string_id] = entry

    return {'sourceLanguage': 'en', 'strings': strings, 'version': '1.1'}


def build_phrase_section(phrase_strings, locale_translations):
    """Build the xcstrings entry for a set of Siri phrases (stringSet format)."""
    en_values = [v for data in phrase_strings.values() for v in data['value']]

    localizations = {
        'en': {'stringSet': {'state': 'new', 'values': en_values}}
    }

    for locale, translations in locale_translations.items():
        locale_values = []
        for data in phrase_strings.values():                                                                                                                      
          translation = translations.get(data['string_id'])
          if translation:                                                                                                                                       
              locale_values.append(translation)
              if not f"${{applicationName}}" in translation:
                  print(f"Missing required `${{applicationName}}` in {translation} for {locale}")
                  exit(1)

        if locale_values:
            localizations[normalize_locale(locale)] = {
                'stringSet': {'state': 'translated', 'values': locale_values}
            }

    return {'localizations': localizations}


def build_appshortcuts_xcstrings(activate_strings, deactivate_strings, locale_translations):
    """Build AppShortcuts.xcstrings dict from activate/deactivate phrase strings."""
    strings = {}

    if activate_strings:
        # The xcstrings key for a phrase group is the first English phrase value
        section_key = next(iter(activate_strings.values()))['value'][0]
        strings[section_key] = build_phrase_section(activate_strings, locale_translations)

    if deactivate_strings:
        section_key = next(iter(deactivate_strings.values()))['value'][0]
        strings[section_key] = build_phrase_section(deactivate_strings, locale_translations)

    return {'sourceLanguage': 'en', 'strings': strings, 'version': '1.1'}

def find_localized_string_resources_in_swift():
      """Find all LocalizedStringResource string literals in Swift files."""
      repo_root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
      pattern = re.compile(r'LocalizedStringResource\("([^"]*)"')
      results = []
      for swift_file in glob.glob(f'{repo_root}/**/*.swift', recursive=True):
          with open(swift_file, 'r', encoding='utf-8') as f:
              content = f.read()
          for match in pattern.finditer(content):
              results.append(match.group(1))
      return results

def main():
    parser = argparse.ArgumentParser(
        description='Generate .xcstrings files from strings.yaml and XLIFF translations'
    )
    parser.add_argument('strings_yaml', help='Path to src/translations/strings.yaml')
    parser.add_argument('i18n_dir', help='Path to 3rdparty/i18n directory')
    parser.add_argument('output_dir', help='Directory to write the .xcstrings files into')
    args = parser.parse_args()

    # Load all strings and filter to iosAppIntents.*    
    all_strings = parseYAMLTranslationStrings(args.strings_yaml)
    if not all_strings:
        print(f"Error parsing YAML")
        exit(1)

    activate_strings = {
        k: v for k, v in all_strings.items()
        if v['string_id'].startswith('vpn.iosAppIntentsActivate')
    }
    deactivate_strings = {
        k: v for k, v in all_strings.items()
        if v['string_id'].startswith('vpn.iosAppIntentsDeactivate')
    }
    main_strings = {
        k: v for k, v in all_strings.items()
        if v['string_id'].startswith('vpn.iosAppIntentsMain')
    }

    # Load translations for all locales
    locales = get_locales(args.i18n_dir)
    locale_translations = {}
    for locale in locales:
        xliff_path = os.path.join(args.i18n_dir, locale, 'mozillavpn.xliff')
        locale_translations[locale] = load_xliff_translations(xliff_path, locale == 'en')

    os.makedirs(args.output_dir, exist_ok=True)

    # Confirm each main string is in Swift code. Confirm that all keys in Swift code are in our translations
    keys_used_in_swift = find_localized_string_resources_in_swift()
    string_ids_from_main = [item['string_id'] for item in main_strings.values()]
    for string_id in string_ids_from_main:
        if string_id not in keys_used_in_swift:
            print(f"Translation not used in Swift files: {string_id}")
            exit(1)
    for string_id in keys_used_in_swift:
        if string_id not in string_ids_from_main:
            print(f"Key in Swift files not found in VPN translation pipeline: {string_id}")
            exit(1)

    # Write Localizable.xcstrings
    localizable = build_localizable_xcstrings(main_strings, locale_translations)
    localizable_path = os.path.join(args.output_dir, 'Localizable.xcstrings')
    with open(localizable_path, 'w', encoding='utf-8') as f:
        json.dump(localizable, f, indent=2, ensure_ascii=False)
        f.write('\n')
    print(f'Wrote {localizable_path}')

    # Write AppShortcuts.xcstrings
    appshortcuts = build_appshortcuts_xcstrings(activate_strings, deactivate_strings, locale_translations)
    appshortcuts_path = os.path.join(args.output_dir, 'AppShortcuts.xcstrings')
    with open(appshortcuts_path, 'w', encoding='utf-8') as f:
        json.dump(appshortcuts, f, indent=2, ensure_ascii=False)
        f.write('\n')
    print(f'Wrote {appshortcuts_path}')


if __name__ == '__main__':
    main()
