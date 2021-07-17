#!/usr/bin/env python3
from flask import Flask
from flask import request, redirect, Response
import argparse
import datetime
import requests
import re
import urllib.parse
import json

import logging
log = logging.getLogger('werkzeug')
log.disabled = True

app = Flask(__name__)
app.logger.disabled = True

## Proxy configuration
upstream = 'https://stage-vpn.guardian.nonprod.cloudops.mozgcp.net'

#----------------------------------------------------------
# Request handling and forwarding
#----------------------------------------------------------
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

def forward_upstream(req, desturl=None, verbose=None, mangle=None):
    # Determine if we should make this verbose
    if verbose is None:
        verbose = log_match(req.path)

    # Log the request
    print(f"REQUEST {req.method} -> {req.path}")
    if verbose:
        log_headers(req.headers)
    if verbose and request.json:
        print(f"REQUEST JSON -> {req.path}")
        print('\t' + json.dumps(request.json, indent=3).replace('\n', '\n\t'))

    # Determine the destination URL
    if desturl is None:
        desturl = upstream + req.path
    else:
        print(f"REQUEST TARGET -> {desturl}")

    # Forward the request to the staging service
    upstream_exclude = [
        'host'
    ]
    upstream_request = requests.request(
        method=req.method,
        url=desturl,
        headers={k: v for (k, v) in req.headers if k.lower() not in upstream_exclude},
        data=req.get_data(),
        cookies=req.cookies
    )

    # Mangle the response
    reply_exclude = [
        'content-encoding',
        'content-length',
        'transfer-encoding',
        'connection'
    ]
    reply_headers = [
        (k, v) for (k,v) in upstream_request.raw.headers.items() if k.lower() not in reply_exclude
    ]
    reply_content = upstream_request.content
    if callable(mangle):
        reply_content = mangle(upstream_request.content)

    # Log the response we're going to send
    print(f"RESPONSE {req.method} -> {req.path} -> {upstream_request.status_code}")
    if verbose:
        log_headers(reply_headers)
    if verbose and upstream_request.content:
        jscontent = json.loads(reply_content)
        print(f"RESPONSE JSON -> {req.path}")
        print('\t' + json.dumps(jscontent, indent=3).replace('\n', '\n\t'))

    return Response(reply_content, upstream_request.status_code, reply_headers)

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
# Device and account mocking
#----------------------------------------------------------
mock_devices = False
mock_device_set = {}

def mangle_account(jsdata):
    jsdata['devices'] = mock_device_set.values()
    return jsdata

@app.route('/api/v1/vpn/account')
def get_account():
    if not mock_devices:
        return forward_upstream(request)
    
    return forward_upstream(request, mangle=mangle_account)

@app.route('/api/v1/vpn/device', methods=['POST'])
def post_new_device():
    if not mock_devices:
        return forward_upstream(request)

    # Parse the request
    try:
        name = request.json['name']
        pubkey = request.json['pubkey']
        device = {
            'name': name,
            'pubkey': pubkey,
            'ipv4_address': '10.67.123.45/32',
            'ipv6_address': 'fc00:bbbb:bbbb:bb01::dead:beef/128',
            'created_at': datetime.utcnow().isoformat() + 'Z'
        }
    except e:
        return Response('', 400)

    # Return a 201 if successful
    return Response(mock_device_set[pubkey], 201)

@app.route('/api/v1/vpn/device/<path:encpubkey>', methods=['DELETE'])
def delete_device(encpubkey):
    pubkey = urllib.parse.quote(encpubkey, safe='')
    if not mock_devices:
        # Flask insists on URL-decoding, so we must re-encode the public key.
        return forward_upstream(request, desturl=f"{upstream}/api/v1/vpn/device/{pubkey}")

    if not mock_device_set.contains(pubkey):
        return Response('', 404)
    
    del mock_mock_device_set[pubkey]
    return Response('', 201)

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
    parser.add_argument('--mock-devices', action='store_true',
                        help='Mock out the devices API')

    args = parser.parse_args()
    for pattern in args.verbose:
        log_patterns.append(re.compile(pattern))

    app.run()
