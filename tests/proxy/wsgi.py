#!/usr/bin/env python3
from flask import Flask
from flask import request, redirect, Response
from datetime import datetime
import argparse
import requests
import re
import urllib.parse
import json

import logging
log = logging.getLogger('werkzeug')
log.disabled = True

app = Flask(__name__)

## Proxy configuration
upstream = 'https://stage-vpn.guardian.nonprod.cloudops.mozgcp.net'

## Device mocking
mock_devices = False
mock_device_set = {}

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
    if callable(mangle) and upstream_request.content:
        mangled_content = mangle(json.loads(upstream_request.content))
        reply_content = json.dumps(mangled_content)
    else:
        reply_content = upstream_request.content

    # Log the response we're going to send
    print(f"RESPONSE {req.method} -> {req.path} -> {upstream_request.status_code}")
    if verbose:
        log_headers(reply_headers)
    if verbose and reply_content:
        jscontent = json.loads(reply_content)
        print(f"RESPONSE JSON -> {req.path}")
        print('\t' + json.dumps(jscontent, indent=3).replace('\n', '\n\t'))

    return Response(reply_content, upstream_request.status_code, reply_headers)

# Return an API error
def api_error(errno, code=400, message='Bad Request'):
    return Response(json.dumps({
        'code': code,
        'errno': errno,
        'message': message
    }), code, mimetype='application/json')

#----------------------------------------------------------
# Redirect authentication to the staging server
#----------------------------------------------------------
def mangle_verify(jsdata):
    userdata = jsdata["user"]
    userdata["devices"] = [ device for device in mock_device_set.values() ]
    jsdata["user"] = userdata
    return jsdata

@app.route('/api/v2/vpn/login/verify', methods=['POST'])
def intercept_verify():
    if not mock_devices:
        return forward_upstream(request)
    else:
        return forward_upstream(request, mangle=mangle_verify)

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
def mangle_account(jsdata):
    jsdata['devices'] = [ device for device in mock_device_set.values() ]
    return jsdata

@app.route('/api/v1/vpn/account')
def get_account():
    if not mock_devices:
        return forward_upstream(request)
    else:
        return forward_upstream(request, mangle=mangle_account)

@app.route('/api/v1/vpn/device', methods=['POST'])
def post_new_device():
    if not mock_devices:
        return forward_upstream(request)

    # Log the request
    print(f"REQUEST {request.method} -> {request.path}")
    if log_match(request.path):
        log_headers(request.headers)

    # Parse the request
    try:
        name = request.json['name']
        pubkey = request.json['pubkey']
        mock_device_set[pubkey] = {
            'name': name,
            'pubkey': pubkey,
            'unique_id': "TestDevice-" + pubkey[0:6] + ".." + pubkey[-6:],
            'ipv4_address': '10.67.123.45/32',
            'ipv6_address': 'fc00:bbbb:bbbb:bb01::dead:beef/128',
            'created_at': datetime.utcnow().isoformat() + 'Z'
        }
    except:
        return api_error(errno=102, message="'pubkey' is not a valid WireGuard public key")

    return Response(json.dumps(mock_device_set[pubkey]), status=201, mimetype='application/json')

@app.route('/api/v1/vpn/device/<path:pubkey>', methods=['DELETE'])
def delete_device(pubkey):
    if not mock_devices:
        # Flask insists on URL-decoding, so we must re-encode the public key.
        encpubkey = urllib.parse.quote(pubkey, safe='')
        return forward_upstream(request, desturl=f"{upstream}/api/v1/vpn/device/{encpubkey}")

    if pubkey not in mock_device_set:
        return api_error(errno=122, code=404, message='Device not found')
    
    del mock_device_set[pubkey]
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
    mock_devices = args.mock_devices
    for pattern in args.verbose:
        log_patterns.append(re.compile(pattern))

    app.run()
