import argparse
import bz2
import concurrent.futures
import contextlib
import datetime
import gzip
import hashlib
import json
import lzma
import multiprocessing
import os
import pathlib
import random
import stat
import subprocess
import sys
import tarfile
import tempfile
import time
import urllib.parse
import urllib.request
import zipfile
import ssl
import socket
import pprint
from pathlib import Path

import shutil

print("HOME:"+str(Path.home()))
print(os.environ.get('USERNAME'))
print(os.environ.get('USER'))


# No Need to Investigate drive usage.
# DRIVES = ["a:","b:","c:","d:","e:","f:","g:","h:","i:","j:","k:","l:","m:","n:","o:","p:","q:","r:","s:","t:","u:","v:","w:","x:","y:","z:"]


# for x in DRIVES:
#     try:
#         total, used, free = shutil.disk_usage(x)
#         print("{} \t total:{}gb \t used:{}gb \t free: {}gb".format(x, (total// (2**30)), (used// (2**30)),(free// (2**30))))
#     except:
#         print("{} \t total:0gb \t used:0gb \t free:0gb".format(x))

#print("Total: %d GiB" % (total // (2**30)))
#print("Used: %d GiB" % (used // (2**30)))
#print("Free: %d GiB" % (free // (2**30)))


def log(x):
    pprint.pprint(x)

def stream_download(url, sha256=None, size=None):
    log('Downloading %s' % url)
    h = hashlib.sha256()
    length = 0

    t0 = time.time()
    with urllib.request.urlopen(url) as fh:
        while True:
            chunk = fh.read(65536)
            if not chunk:
                break
            length += len(chunk)

    duration = time.time() - t0
    digest = h.hexdigest()

    log('%s resolved to %d bytes with sha256 %s in %.3fs' % (
        url, length, digest, duration))

try:
    print("get download")
    stream_download("https://firefox-ci-tc.services.mozilla.com/api/queue/v1/task/KZfYmRnbTkauG_DCOVqAzQ/runs/0/artifacts/public/build/vpn/mozillavpn-x86-release.apk")
except BaseException as err:
    log(f" Failed to download using urllib.request.urlopen {err}, {type(err)}") 

import certifi
def stream_download_certify(url, sha256=None, size=None):
    log('Downloading %s' % url)
    h = hashlib.sha256()
    length = 0
    t0 = time.time()
    context = ssl.create_default_context(cafile=certifi.where())
    with urllib.request.urlopen(url,context=context) as fh:
        while True:
            chunk = fh.read(65536)
            if not chunk:
                break
            length += len(chunk)

    duration = time.time() - t0
    digest = h.hexdigest()

    log('%s resolved to %d bytes with sha256 %s in %.3fs' % (
        url, length, digest, duration))
    log('Certify was able to download stuff')
try:
    print("get download")
    stream_download_certify("https://firefox-ci-tc.services.mozilla.com/api/queue/v1/task/KZfYmRnbTkauG_DCOVqAzQ/runs/0/artifacts/public/build/vpn/mozillavpn-x86-release.apk")
except BaseException as err:
    log(f" Failed to download using urllib.request.urlopen {err}, {type(err)}") 





log("Loading certs from: ")
ssl.get_default_verify_paths()

context = ssl._create_default_https_context()
log("SSL CONTEXT")
log(context.cert_store_stats())

def getCert(host):
    try:
        conn = context.wrap_socket(socket.socket(socket.AF_INET),server_hostname=host)
        conn.connect((host, 443))
        cert = conn.getpeercert()
        pprint.pprint(cert)
    except ssl.SSLCertVerificationError as err:
        log(f"Validation error {err}, {type(err)}")
log("SSL CONTEXT: Fetch cert forfirefox-ci-tc.services.mozilla.com ")
getCert("firefox-ci-tc.services.mozilla.com")
