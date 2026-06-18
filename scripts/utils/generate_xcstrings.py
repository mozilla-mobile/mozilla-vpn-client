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
import xml.etree.ElementTree as ET
from generate_strings import parseYAMLTranslationStrings

XLIFF_NS = "{urn:oasis:names:tc:xliff:document:1.2}"
_FALLBACK_LOCALES = None

def load_xliff_translations(xliff_path, isEnglish):
    """Returns dict of {string_id: translated_string} for one locale's XLIFF file."""
    if not os.path.isfile(xliff_path):
        print(f"XLIFF file not found: {xliff_path}")
        exit(1)
    translations = {}
    tree = ET.parse(xliff_path)
    for unit in tree.getroot().iter(f"{XLIFF_NS}trans-unit"):
        trid = unit.attrib.get("id", "")
        if isEnglish:
            translations[trid] = unit.findtext(f"{XLIFF_NS}source", default="")
        else:
            translations[trid] = unit.findtext(f"{XLIFF_NS}target", default="")
    if not translations:
        print(f"No translations found: {xliff_path}")
        exit(1)
    return translations


def using_app_placeholder(value):
    return value.replace("%@", "\\(.applicationName)")


SPECIAL_LOCALE_MAP = {
    "zh_CN": "zh-Hans",
    "zh_TW": "zh-Hant",
}


def normalize_locale(locale):
    return SPECIAL_LOCALE_MAP.get(locale, locale).replace("_", "-")


def get_locales(i18n_dir):
    """Returns list of locale codes that have a mozillavpn.xliff."""
    locales = [
        entry
        for entry in os.listdir(i18n_dir)
        if os.path.isfile(os.path.join(i18n_dir, entry, "mozillavpn.xliff"))
    ]
    if not locales:
        print("No other locales found")
        exit(1)
    return locales


def build_localizable_xcstrings(main_strings, locale_translations):
    """Build Localizable.xcstrings dict for iosAppIntents.main.* strings."""
    strings = build_main_strings(main_strings, locale_translations)
    return {"sourceLanguage": "en", "strings": strings, "version": "1.1"}


def build_main_strings(strings_to_translate, locale_translations, use_id_as_key = True):
    strings = {}
    for data in strings_to_translate.values():
        english_value = data["value"][0]
        string_id = data["string_id"]
        comment = data["comments"][0] if data["comments"] else None

        entry = {}
        if comment:
            entry["comment"] = comment

        localizations = {}
        for locale, translations in locale_translations.items():
            translated = translations.get(string_id)
            if not translated:
                # Apple will show the key if it doesn't have a string. Need to provide *some* string.
                translated = get_fallback_translation(string_id, locale, locale_translations)
            for placeholder in re.findall(r"%.{0,2}@", english_value):
                if placeholder not in translated:
                    print(
                        f"Error: placeholder '{placeholder}' missing from {locale} translation of '{string_id}'"
                    )
                    exit(1)
            localizations[normalize_locale(locale)] = {
                "stringUnit": {
                    "state": "translated",
                    "value": translated,
                }
            }
        entry["localizations"] = localizations

        if use_id_as_key:
            strings[string_id] = entry
        else:
            strings[english_value] = entry
    return strings


def fallback_locales():
    global _FALLBACK_LOCALES
    if _FALLBACK_LOCALES is None:
        repo_root = os.path.dirname(
            os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
        )
        path = os.path.join(
            repo_root, "src", "translations", "extras", "translations_fallback.json"
        )
        if not os.path.exists(path):
            print(f"Translations fallback file not found at {path}")
            exit(1)
        with open(path, encoding="utf-8") as f:
            _FALLBACK_LOCALES = json.load(f)
    return _FALLBACK_LOCALES


def get_fallback_translation(string_id, locale, locale_translations):
    for fallback_locale in fallback_locales().get(locale, []):
        translated = locale_translations.get(fallback_locale, {}).get(string_id)
        if translated:
            print(f"Falling back to {fallback_locale} for {locale} for {string_id}")
            return translated
    print(f"Falling back to English for {locale} for {string_id}")
    return locale_translations.get("en", {}).get(string_id, string_id)


def build_phrase_section(phrase_strings, locale_translations):
    """Build the xcstrings entry for a set of Siri phrases (stringSet format)."""

    phrase_blocks = phrase_strings.values()
    if len(phrase_blocks) != 1:
        print(
            f"Should only have 1 translation block for phrase strings, instead found {len(phrase_blocks)}"
        )
        exit(1)
    string_id = next(iter(phrase_blocks))["string_id"]

    localizations = {}
    error_locales = []
    for locale, translations in locale_translations.items():
        locale_values = []
        translation_block = translations.get(string_id)
        if translation_block is None:
            continue
        for translation in translation_block.split("\n"):
            if translation:
                if translation.count("%@") != 1:
                    print(f"Placeholder found {translation.count('%@')} times in {translation} for {locale}, expected 1")
                    error_locales.append(locale)
                locale_values.append(using_app_placeholder(translation))

        if locale_values:
            localizations[normalize_locale(locale)] = {
                "stringSet": {
                    "state": "translated",
                    "values": locale_values,
                }
            }
        else:
            print(f"No translations found for {string_id} in {locale}")

    return {"localizations": localizations}, error_locales


def build_appshortcuts_xcstrings(intent_phrase_array, intent_title_array, locale_translations):
    """Build AppShortcuts.xcstrings dict from phrase strings."""
    strings = {}

    overall_error_locales = {}
    for phrase_set in intent_phrase_array:
        phrase_section, error_locales = build_phrase_section(phrase_set, locale_translations)
        if error_locales:
            string_id = next(iter(phrase_set.values()))["string_id"]
            overall_error_locales[string_id] = error_locales
        if (len(phrase_section["localizations"]) > 0):
            # The xcstrings key for a phrase group is the first English phrase value. But this is so ugly, sorry.
            section_key = next(
                iter(phrase_section["localizations"]["en"]["stringSet"]["values"])
            )
            strings[section_key] = phrase_section
        else:
            print(f"Skipping {next(iter(phrase_set))} because no translations were found in xliff files. This should only occur for new strings that are yet to be ingested into the l10n repo.")

    if overall_error_locales:
        print("Incorrect translation phrases found in:")
        for string_id, locales in overall_error_locales.items():
            print(f"  {string_id}: {', '.join(locales)}")
        exit(1)

    title_strings = build_main_strings(intent_title_array, locale_translations, False)
    strings.update(title_strings)

    return {"sourceLanguage": "en", "strings": strings, "version": "1.1"}


def find_localized_string_resources_in_swift():
    """Find all LocalizedStringResource string literals in Swift files."""
    repo_root = os.path.dirname(
        os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    )
    pattern = re.compile(r'LocalizedStringResource\("([^"]*)"')
    results = []
    for swift_file in glob.glob(f"{repo_root}/**/*.swift", recursive=True):
        with open(swift_file, "r", encoding="utf-8") as f:
            content = f.read()
        for match in pattern.finditer(content):
            results.append(match.group(1))
    return results


def main():
    parser = argparse.ArgumentParser(
        description="Generate .xcstrings files from strings.yaml and XLIFF translations"
    )
    parser.add_argument("strings_yaml", help="Path to src/translations/strings.yaml")
    parser.add_argument("i18n_dir", help="Path to 3rdparty/i18n directory")
    parser.add_argument(
        "output_dir", help="Directory to write the .xcstrings files into"
    )
    args = parser.parse_args()

    # Load all strings and filter to iosAppIntents.*
    all_strings = parseYAMLTranslationStrings(args.strings_yaml)
    if not all_strings:
        print("Error parsing YAML")
        exit(1)

    shortcut_strings = []
    # ToggleIntent intentionally is not included here, as we do not use Siri phrases to activate it. It is only used to control widgets/controls.
    for translation_prefix in ['vpn.iosAppIntentsActivate', 'vpn.iosAppIntentsDeactivate', 'vpn.iosAppIntentsQueryStatus']:
        current_string_set = {
            k: v for k, v in all_strings.items()
            if v['string_id'].startswith(translation_prefix)
        }
        if not current_string_set:
            print(f"No strings found for {translation_prefix}")
            exit(1)
        shortcut_strings.append(current_string_set)

    WIDGET_STRING_PREFIXES = (
      "vpn.iosAppIntentsMain",
      "vpn.mobileOnboarding.panelOneTitle",
      "vpn.toggleWidget",
      "vpn.cityAccessoryWidget",
      "vpn.logoAccessoryWidget",
      "vpn.multiHopFeature.multiHopToggleCTA"
    )

    main_strings = {
        k: v
        for k, v in all_strings.items()
        if v["string_id"].startswith(WIDGET_STRING_PREFIXES)
    }

    intent_titles = ["vpn.iosAppIntentsMain.statusQueryTitle", "vpn.iosAppIntentsMain.turnOffAction", "vpn.iosAppIntentsMain.turnOnAction"]
    intent_title_array = {
        k: v
        for k, v in all_strings.items()
        if v["string_id"] in intent_titles
    }

    # Load translations for all locales
    locales = get_locales(args.i18n_dir)
    locale_translations = {}
    for locale in locales:
        xliff_path = os.path.join(args.i18n_dir, locale, "mozillavpn.xliff")
        locale_translations[locale] = load_xliff_translations(
            xliff_path, locale == "en"
        )

    os.makedirs(args.output_dir, exist_ok=True)

    # Confirm each main string is in Swift code. Confirm that all keys in Swift code are in our translations
    keys_used_in_swift = find_localized_string_resources_in_swift()
    string_ids_from_main = [item["string_id"] for item in main_strings.values()]
    for string_id in string_ids_from_main:
        if string_id not in keys_used_in_swift:
            print(f"Translation not used in Swift files: {string_id}")
            exit(1)
    for string_id in keys_used_in_swift:
        if string_id not in string_ids_from_main:
            print(
                f"Key in Swift files not found in VPN translation pipeline: {string_id}"
            )
            exit(1)

    # Write Localizable.xcstrings
    localizable = build_localizable_xcstrings(main_strings, locale_translations)
    localizable_path = os.path.join(args.output_dir, "Localizable.xcstrings")
    with open(localizable_path, "w", encoding="utf-8") as f:
        json.dump(localizable, f, indent=2, ensure_ascii=False)
        f.write("\n")
    print(f"Wrote {localizable_path}")

    # Write AppShortcuts.xcstrings
    appshortcuts = build_appshortcuts_xcstrings(shortcut_strings, intent_title_array, locale_translations)
    appshortcuts_path = os.path.join(args.output_dir, "AppShortcuts.xcstrings")
    with open(appshortcuts_path, "w", encoding="utf-8") as f:
        json.dump(appshortcuts, f, indent=2, ensure_ascii=False)
        f.write("\n")
    print(f"Wrote {appshortcuts_path}")


if __name__ == "__main__":
    main()
