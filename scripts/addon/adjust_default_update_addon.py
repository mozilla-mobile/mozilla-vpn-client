import argparse
import os

if __name__ == "__main__":

    parser = argparse.ArgumentParser(description="Generate an addon package")
    parser.add_argument(
        "latest_update_manifest",
        metavar="LATEST",
        type=str,
        action="store",
        help="The latest update addon manifest",
    )
    parser.add_argument(
        "default_update_manifest",
        metavar="DEFAULT",
        type=str,
        action="store",
        help="The default update addon manifest",
    )
    parser.add_argument(
        "skipped_locales_file",
        metavar="SKIPPED",
        type=str,
        action="store",
        help="The text file with skipped locales for the latest update",
    )
    args = parser.parse_args()

    # get date, max_client_version, and short_version
    shortVersionLine = None
    maxClientLine = None
    dateLine = None
    with open(args.latest_update_manifest, "r", encoding="utf-8") as f:
        for line in f:
            if '"shortVersion"' in line:
                shortVersionLine = line
            if '"date"' in line:
                dateLine = line
            if '"max_client_version"' in line:
                maxClientLine = line

    localesLine = None
    with open(os.path.join(args.skipped_locales_file), "r", encoding="utf-8") as f:
        for line in f:
          localesLine = f'    "locales": [{line.strip()}],\n'

    if shortVersionLine is None or maxClientLine is None or dateLine is None or localesLine is None:
        print("Unable to find important data in latest update addon")
        exit(1)
      
    with open(args.default_update_manifest, "r", encoding="utf-8") as f:
        lines = f.readlines()

    for i, line in enumerate(lines):
        if '"shortVersion"' in line:
            lines[i] = shortVersionLine
        if '"date"' in line:
            lines[i] = dateLine
        if '"max_client_version"' in line:
            lines[i] = maxClientLine
        if '"locales"' in line:
            lines[i] = localesLine
              
    with open(args.default_update_manifest, "w", encoding="utf-8") as f:
          f.writelines(lines)
