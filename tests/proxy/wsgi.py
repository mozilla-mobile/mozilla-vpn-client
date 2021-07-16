#!/usr/bin/env python3
from flask import Flask
from flask import request, redirect, Response
import requests
import json

app = Flask(__name__)

upstream = 'https://stage-vpn.guardian.nonprod.cloudops.mozgcp.net'

def log_headers(headers):
    for (key, value) in headers:
        # If the value contains semicolons, split to multiple lines for readability
        if value.find(";") >= 0:
            print(f"\t{key}:")
            for token in value.split(';'):
                print(f"\t\t{token.strip()}")
        else:
            print(f"\t{key}: {value}")

def forward_upstream(req, verbose=False):
    print(f"REQUEST {req.method} -> {req.path}")
    if verbose:
        log_headers(req.headers)

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

    # Forward the request to the staging service
    upstream_request = requests.request(
        method=req.method,
        url=upstream + req.path,
        headers={k: v for (k, v) in req.headers if k.lower() not in exclude_upstream},
        data=req.get_data(),
        cookies=req.cookies
    )
    print(f"RESPONSE {req.method} -> {req.path} -> {upstream_request.status_code}")
    if verbose:
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
# Proxy anything we aren't handling to the staging server
#----------------------------------------------------------
@app.errorhandler(404)
def default_url_handler(e):
    return forward_upstream(request)

#----------------------------------------------------------
# Redirect authentication to the staging server
#----------------------------------------------------------
@app.route('/api/v2/vpn/login/verify', methods=['POST'])
def intercept_login():
    return forward_upstream(request, verbose=True)

@app.route('/api/v2/vpn/login/<path:text>')
def redirect_login(text):
    print(f"REDIRECT {request.method} -> {request.path}")
    log_headers(request.headers)

    url = f"{upstream}{request.path}"
    if request.query_string:
        url += f"?{request.query_string.decode('utf-8')}"
    return redirect(url)

if __name__ == '__main__':
    app.run()
