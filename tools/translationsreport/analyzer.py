import os
import xml.etree.ElementTree as ET
import argparse
from jinja2 import Environment, FileSystemLoader


def extract_translated_strings(xliff_path):
    tree = ET.parse(xliff_path)
    root = tree.getroot()

    translated_strings = set()
    not_translated_strings = set()

    # Define the namespace used in XLIFF files
    namespace = {"xliff": "urn:oasis:names:tc:xliff:document:1.2"}

    for trans_unit in root.findall(".//xliff:trans-unit", namespace):
        source = trans_unit.find(".//xliff:source", namespace).text
        target_element = trans_unit.find(".//xliff:target", namespace)

        # Big hack, but this string is breaking the template.
        # Jinja feel obligated to add a newline before % and
        # I (Bea) don't know why
        if "Please confirm you would like to remove" in source:
            source = "Please confirm you would like to remove"

        if target_element is not None and target_element.text is not None:
            translated_strings.add(source)
        else:
            not_translated_strings.add(source)


    return (translated_strings, not_translated_strings)


def get_language_code_from_path(path):
    components = path.split(os.path.sep)

    # Find the index of "i18n" in the path
    i18n_index = components.index("i18n") if "i18n" in components else -1

    # If "i18n" is found and there is a folder after it, return that folder
    if i18n_index != -1 and i18n_index < len(components) - 1:
        return components[i18n_index + 1]
    else:
        raise Exception(f"Unexpected folder path! {path}")


def get_addon_name_from_xliff_path(path):
    # Split the path into components
    components = path.rstrip(os.path.sep).split(os.path.sep)

    # Return the last component, which is the last folder name
    return components[-2]


def analyze_folder(folder_path):
    main_app_translations = {}
    addon_translations = {}

    script_directory = os.path.dirname(os.path.abspath(__file__))
    addons_directory = os.path.join(script_directory, "../../addons")
    for folder in os.listdir(addons_directory):
        if os.path.isdir(os.path.join(addons_directory, folder)):
            addon_translations[folder] = {}

    for root, dirs, files in os.walk(folder_path):
        for file in files:
            if file.endswith(".xliff"):
                xliff_file = os.path.join(root, file)
                (translations, non_translations) = extract_translated_strings(xliff_file)
                language_code = get_language_code_from_path(xliff_file)

                if language_code == "en":
                    translations = non_translations

                destination = main_app_translations

                # Determine if the strings are from the root or an addon
                is_addon = "addons" in os.path.relpath(xliff_file, folder_path)
                if is_addon:
                    addon_name = get_addon_name_from_xliff_path(xliff_file)
                    if addon_name not in addon_translations:
                        addon_translations[addon_name] = {}

                    destination = addon_translations[addon_name]

                for translation in translations:
                    if translation not in destination:
                        destination[translation] = set()

                    destination[translation].add(language_code)


    return (main_app_translations, addon_translations)


def generate_html(result):
    script_directory = os.path.dirname(os.path.abspath(__file__))
    template_loader = FileSystemLoader(script_directory)
    env = Environment(loader=template_loader,autoescape=True)

    template = env.get_template('template.jinja')

    (main_app_translations, addon_translations) = result
    output = template.render(
        main_app_translations=main_app_translations,
        addon_translations=addon_translations
    )

    with open(os.path.join(script_directory, "output.json"), 'w') as f:
        f.write(output)


def main():
    script_directory = os.path.dirname(os.path.abspath(__file__))
    result = analyze_folder(os.path.join(script_directory, "../../src/translations/i18n"))
    generate_html(result)


if __name__ == "__main__":
    main()
