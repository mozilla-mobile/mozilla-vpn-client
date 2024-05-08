#! /usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import macpkg
import argparse
import os
import stat
import shutil

# Extract a PBZX payload from a Mac package
def extract_payload(fileobj, args):
    hardlinks = {}
    for path, st, content in macpkg.uncpio(macpkg.Pbzx(fileobj)):
        if not path:
            continue
        path = path.decode().lstrip('/')
        if args.prefix is None:
            matches = True
        else:
            matches = path.startswith(args.prefix)
            path = path[len(args.prefix):].lstrip("/")

        # When there are hardlinks, normally a cpio stream is supposed to
        # contain the data for all of them, but, even with compression, that
        # can be a waste of space, so in some cpio streams (*cough* *cough*,
        # Apple's, e.g. in Xcode), the files after the first one have dummy
        # data.
        # As we may be filtering the first file out (if it doesn't match
        # extract_prefix), we need to keep its data around (we're not going
        # to be able to rewind).
        if stat.S_ISREG(st.mode) and st.nlink > 1:
            key = (st.dev, st.ino)
            hardlink = hardlinks.get(key)
            if hardlink:
                hardlink[0] -= 1
                if hardlink[0] == 0:
                    del hardlinks[key]
                content = hardlink[1]
                if isinstance(content, BytesIO):
                    content.seek(0)
                    if matches:
                        hardlink[1] = path
            elif matches:
                hardlink = hardlinks[key] = [st.nlink - 1, path]
            else:
                hardlink = hardlinks[key] = [st.nlink - 1, BytesIO(content.read())]
                content = hardlink[1]

        if not matches:
            continue

        outpath = os.path.join(args.output, path)
        if args.verbose:
            print(f"Extracting {stat.filemode(st.mode)} {path}")
        if stat.S_ISDIR(st.mode):
            os.makedirs(outpath, exist_ok=True)
        else:
            parent = os.path.dirname(outpath)
            if parent:
                os.makedirs(parent, exist_ok=True)

            if stat.S_ISLNK(st.mode):
                os.symlink(content.read(), outpath)
            elif stat.S_ISREG(st.mode):
                if isinstance(content, str):
                    targetpath = os.path.join(args.output, content)
                    os.link(targetpath, outpath)
                else:
                    with open(outpath, "wb") as out:
                        shutil.copyfileobj(content, out)
            else:
                raise Exception(f"File mode {st.mode:o} is not supported")

def extract_other(fileobj, name, args):
    outpath = os.path.join(args.output, name)
    parent = os.path.dirname(outpath)
    if parent:
        os.makedirs(parent, exist_ok=True)

    with open(os.path.join(args.output, name), "wb") as out:
        shutil.copyfileobj(fileobj, out)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Unpack a Mac PKG file')
    parser.add_argument('input', metavar='INFILE', type=str,
        help='Package file to extract')
    parser.add_argument('-o', '--output', metavar='DST', type=str, action='store',
        help='Write output to DST')
    parser.add_argument('-p', '--prefix', metavar='PREFIX', type=str, action='store',
        help='Extract files from PREFIX inside archive')
    parser.add_argument('-a', '--all', default=False, action='store_true',
        help='Extract all metadata files from the archive')
    parser.add_argument('-v', '--verbose', default=False, action='store_true',
        help='Print verbose information about the extracted files')
    args = parser.parse_args()

    if args.output is None:
        args.output = os.getcwd()

    # Open the package
    with open(args.input, 'rb') as fp:
        for name, content in macpkg.unxar(fp):
            if name in ("Payload", "Content"):
                extract_payload(content, args)
            elif args.all:
                extract_other(content, name, args)
