#!/usr/bin/env python3
from flask import Flask
from flask import request, redirect, Response
import argparse
import requests
import re
import urllib.parse
import json

app = Flask(__name__)

upstream = 'https://stage-vpn.guardian.nonprod.cloudops.mozgcp.net'
log_patterns = []

def log_headers(headers):
    for (key, value) in headers:
        # If the value contains semicolons, split to multiple lines for readability
        if value.find(";") >= 0:
            print(f"\t{key}:")
            for token in value.split(';'):
                print(f"\t\t{token.strip()}")
        else:
            print(f"\t{key}: {value}")

def log_match(path):
    for prog in log_patterns:
        if prog.match(path):
            return True
    return False

def forward_upstream(req, desturl=None, verbose=None):
    # Determine if we should make this verbose
    if verbose is None:
        verbose = log_match(req.path)

    # Log the request
    print(f"REQUEST {req.method} -> {req.path}")
    if verbose:
        log_headers(req.headers)
    if verbose and request.json:
        print(f"REQUEST BODY -> {req.path}")
        print('\t' + json.dumps(request.json, indent=3).replace('\n', '\n\t'))

    # Parse the request headers  
    exclude_upstream = [
        'host'
    ]
    upstream_headers = {}
    for (key, value) in req.headers:
        if key == 'Host':
            continue
        else:
            upstream_headers[key] = value

    # Determine the destination URL
    if desturl is None:
        desturl = upstream + req.path
    else:
        print(f"REQUEST TARGET -> {desturl}")

    # Forward the request to the staging service
    upstream_request = requests.request(
        method=req.method,
        url=desturl,
        headers={k: v for (k, v) in req.headers if k.lower() not in exclude_upstream},
        data=req.get_data(),
        cookies=req.cookies
    )
    print(f"RESPONSE {req.method} -> {req.path} -> {upstream_request.status_code}")
    if log_match(req.path):
        log_headers(upstream_request.raw.headers.items())

    # Filter out certain response headers
    exclude_downstream = [
        'content-encoding',
        'content-length',
        'transfer-encoding',
        'connection'
    ]
    downstream_headers = [
        (k,v) for (k,v) in upstream_request.raw.headers.items() if k.lower() not in exclude_downstream
    ]

    return Response(upstream_request.content, upstream_request.status_code, downstream_headers)

#----------------------------------------------------------
# Redirect authentication to the staging server
#----------------------------------------------------------
@app.route('/api/v2/vpn/login/verify', methods=['POST'])
def intercept_login():
    return forward_upstream(request)

@app.route('/api/v2/vpn/login/<path:text>')
def redirect_login(text):
    print(f"REDIRECT {request.method} -> {request.path}")
    if (log_match(request.path)):
        log_headers(request.headers)

    url = f"{upstream}{request.path}"
    if request.query_string:
        url += f"?{request.query_string.decode('utf-8')}"
    return redirect(url)

#----------------------------------------------------------
# Workaround for /api/v1/device and URL escaping
#----------------------------------------------------------
@app.route('/api/v1/vpn/device/<path:pubkey>', methods=['DELETE'])
def delete_device(pubkey):
    # Flask insists on URL-decoding, so we must re-encode the public key.
    destination=f"{upstream}/api/v1/vpn/device/{urllib.parse.quote(pubkey, safe='')}"
    return forward_upstream(request, desturl=destination)

#----------------------------------------------------------
# Proxy anything we aren't handling to the staging server
#----------------------------------------------------------
default_methods=['GET', 'HEAD', 'POST', 'PUT', 'DELETE', 'OPTIONS']
@app.route('/<path:path>', methods=default_methods)
def default_handler(path):
    return forward_upstream(request)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Intercept and proxy VPN API requests')
    parser.add_argument('--verbose', metavar='PATTERN', action='append', default=[],
                        help='Enable verbose logging for paths matching PATTERN')

    args = parser.parse_args()
    for pattern in args.verbose:
        log_patterns.append(re.compile(pattern))

    app.run()
