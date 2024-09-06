import os
import sys
import xml.etree.ElementTree as ET

def write_en_language(filename, strings):
    ts = ET.Element("TS")
    ts.set("version", "2.1")
    ts.set("language", "en")

    context = ET.SubElement(ts, "context")
    ET.SubElement(context, "name")

    for key, value in strings.items():
        message = ET.SubElement(context, "message")
        message.set("id", key)

        location = ET.SubElement(message, "location")
        location.set("filename", "addon.qml")

        source = ET.SubElement(message, "source")
        source.text = value["value"]

        translation = ET.SubElement(message, "translation")
        translation.set("type", "unfinished")

        if len(value["comments"]) > 0:
            extracomment = ET.SubElement(message, "extracomment")
            extracomment.text = value["comments"]

    with open(filename, "w", encoding="utf-8") as f:
        f.write(ET.tostring(ts, encoding="unicode"))

def find_qtbinpath(potential_path):
    qtbinpath = potential_path
    if qtbinpath is None:
        qtbinpath = qtquery('qmake', 'QT_INSTALL_BINS')
    if qtbinpath is None:
        qtbinpath = qtquery('qmake6', 'QT_INSTALL_BINS')
    if qtbinpath is None:
        sys.exit('Unable to locate qmake tool.')
    if not os.path.isdir(qtbinpath):
        sys.exit(f"QT path is not a directory: {qtbinpath}")
    return qtbinpath

def qtquery(qmake, propname):
    try:
        qtquery = os.popen(f"{qmake} -query {propname}")
        qtpath = qtquery.read().strip()
        if len(qtpath) > 0:
            return qtpath
    finally:
        pass
    return None
