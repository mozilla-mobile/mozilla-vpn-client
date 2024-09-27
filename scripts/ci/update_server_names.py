#! /usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import os
import re
import requests
import sys
import xml.etree.ElementTree as etree

MULLVAD_SERVER_LIST_API = "https://api.mullvad.net/public/relays/wireguard/v2"
XLIFF_NAMESPACE = "{urn:oasis:names:tc:xliff:document:1.2}"


def fetch_server_list():
    country_names = {}
    city_names = []

    try:
        response = requests.get(MULLVAD_SERVER_LIST_API)

        if response.status_code == 200:
            data = response.json()

            locations = data.get("locations", {})
            for country_code, location in locations.items():
                country_code = country_code.split("-")[0]
                country_name = location.get("country", None)

                # We don't want duplicates,
                # so let's check if this country code is in the list already
                if country_name and country_code not in country_names:
                    country_names[country_code] = country_name

                city = location.get("city", None)
                if city and city not in city_names:
                    city_names.append(city)

        else:
            response.raise_for_status()
    except requests.exceptions.RequestException as e:
        print("Error fetch server list:", e)
    except ValueError as e:
        print("Error parsing server list:", e)

    return {
        "countries": country_names,
        "cities": city_names,
    }


if __name__ == "__main__":
    ###
    # 1. Fetch the latest list of servers from Mullvad.

    countries, cities = fetch_server_list().values()

    # Create a map like so "<xliff_id>": "<xliff_source>"
    # Lucky for us, the coutries dict is already in the shape we need
    string_map = countries
    for city in cities:
        # Remove state suffix, capitalize each work, remove spaces.
        id = city.split(",")[0].strip().title().replace(" ", "")
        string_map[id] = city

    ###
    # 2. Fetch the list of servers in extras.xliff

    script_path = os.path.dirname(os.path.abspath(__file__))
    xliff_path = os.path.join(
        script_path,
        os.path.pardir,
        os.path.pardir,
        "src",
        "translations",
        "extras",
        "extras.xliff",
    )
    if not os.path.exists(xliff_path):
        sys.exit(f"extras.xliff not found in path {xliff_path}")

    tree = etree.parse(xliff_path)
    root = tree.getroot()

    found_server_names = []
    for trans_unit in root.findall(f".//{XLIFF_NAMESPACE}trans-unit"):
        unit_id = trans_unit.get("id")
        if unit_id.startswith("servers."):
            found_server_names.append(unit_id.split(".")[1])

    ###
    # 3. Get the list of missing servers strings and update extras.xliff

    missing = list(set(string_map.keys()) - set(found_server_names))
    # No missing strings, return early
    if not missing:
        print("Server list is up to date!")
        sys.exit(0)

    missing_string_map = {
        key: value for key, value in string_map.items() if key in missing
    }

    # Iterate over the entries and add them to the XLIFF tree
    servers_node = root.find(
        f".//{XLIFF_NAMESPACE}file[@original='../src/apps/vpn/ui/screens/home/ViewServers.qml']//{XLIFF_NAMESPACE}body"
    )
    if servers_node is None:
        sys.exit("Unable to find servers node. Has the extras.xliff file been changed?")

    for id, source in missing_string_map.items():
        new_unit = etree.SubElement(
            servers_node, "{urn:oasis:names:tc:xliff:document:1.2}trans-unit"
        )
        new_unit.set("id", f"servers.{id}")

        new_unit.set("{urn:oasis:names:tc:xliff:document:1.2}xml:space", "preserve")

        source_node = etree.SubElement(
            new_unit, "{urn:oasis:names:tc:xliff:document:1.2}source"
        )
        source_node.text = source

    # Save the new tree to extras.xliff file
    with open(xliff_path, "w", encoding="utf-8") as fp:
        etree.register_namespace("", "urn:oasis:names:tc:xliff:document:1.2")
        # Fix indentation
        etree.indent(root)
        # Create the XLIFF string we are going to write to the file
        xml_string = etree.tostring(
            root,
            encoding="UTF-8",
            method="xml",
            xml_declaration=True,
        ).decode("utf-8")
        # Manually change all quotes to double quotes
        xml_string = xml_string.replace("'", '"')
        # Make sure the xliff tag doesn't get unnecessarily changed
        xml_string = xml_string.replace(
            '<xliff xmlns="urn:oasis:names:tc:xliff:document:1.2" version="1.2">',
            '<xliff version="1.2" xmlns="urn:oasis:names:tc:xliff:document:1.2">',
        )
        # Truncate the original file for rewriting
        fp.truncate(0)
        # Manually add in the newline at the end of the file upon writing
        fp.write(f"{xml_string}\n")
