#!/usr/bin/env python3
import hashlib
import hmac
import math
import getpass
import sys
import urllib
from urllib.parse import urlparse, parse_qs
from requests import Session
from datetime import datetime
import secrets
import base64
import json
import os

debug = True

# Authenticate as an Android client, so the server will pass the OAuth code as a query
# string parameter. Otherwise for desktop clients the OAuth code is passed around somewhere
# in the session context and I am too tired to read minified JS to find it.
vpn_login = 'https://stage-vpn.guardian.nonprod.cloudops.mozgcp.net/api/v2/vpn/login/android'
vpn_verify = 'https://stage-vpn.guardian.nonprod.cloudops.mozgcp.net/api/v2/vpn/login/verify'

# Firefox login API endpoints
fxa_login = "https://api-accounts.stage.mozaws.net/v1/account/login"
fxa_authz = "https://api-accounts.stage.mozaws.net/v1/oauth/authorization"
fxa_clientid = "https://oauth.stage.mozaws.net/v1/client"

def check(req, prompt=None):
    if (req.status_code >= 200) and (req.status_code < 300):
        return

    jsdata = req.json()
    if not prompt:
        print(f"Server returned {req.status_code}", file=sys.stderr)
        print(json.dumps(req.json(), indent=3), file=sys.stderr)
    elif 'errno' in jsdata:
        message = f"Errno {jsdata['errno']}"
        if 'message' in jsdata:
            message = jsdata['message']
        elif 'error':
            message = jsdata['error']
        print(f"{prompt}: {message}", file=sys.stderr)
    sys.exit(1)

#--------------------------------------
# Cryptographic Algorithms
#--------------------------------------
def hkdf(length: int, keydata, salt: bytes = b'', info: bytes = b''):
    if len(salt) == 0:
        salt = bytes([0] * 32)
    privkey = hmac.new(salt, keydata, hashlib.sha256).digest()
    output = b''
    prev = b''
    for i in range(math.ceil(length / 32)):
        prev = hmac.new(privkey, prev + info + bytes([i + 1]), hashlib.sha256).digest()
        output += prev
    return output[:length]

def hawk_hash(payload, contenttype='text/plain'):
    h = hashlib.sha256()
    h.update(b'hawk.1.payload\n')
    h.update(contenttype.lower().encode('utf-8') + b'\n')
    h.update(payload + b'\n')
    return base64.b64encode(h.digest()).decode('utf-8')

def hawk_header(url, nonce, ts, contenthash=None):
    parsed = urlparse(url)
    default_port = 443 if (parsed.scheme == 'https') else 80

    normalized = [
        'hawk.1.header',
        f"{ts}",
        f"{nonce}",
        'POST',
        parsed.path,
        parsed.hostname,
        str(parsed.port or default_port),
        contenthash if contenthash else '',
        '', # Extensions would go here
        ''
    ]
    return '\n'.join(normalized)

# Generate the Hawk authentication header
def hawk_auth(sessiontoken, url, nonce=None, contenthash=None):
    keydata = hkdf(64, sessiontoken, info=b'identity.mozilla.com/picl/v1/sessionToken')
    nonce = nonce or secrets.token_urlsafe(4)
    timestamp = int(datetime.utcnow().timestamp())
    header = hawk_header(url, nonce, timestamp, contenthash)
    digest = hmac.new(keydata[32:64], header.encode('utf-8'), hashlib.sha256).digest()

    fields = {
        'id': keydata[0:32].hex(),
        'ts': timestamp,
        'nonce': nonce,
        'mac': base64.b64encode(digest).decode('utf-8')
    }
    if contenthash:
        fields['hash'] = contenthash

    return "Hawk " + ', '.join([f"{k}=\"{v}\"" for (k, v) in fields.items()])

#------------------------------------------------
# Compute OAuth challenge and request a login
#------------------------------------------------
code_verifier = secrets.token_urlsafe(96)
h = hashlib.sha256()
h.update(code_verifier.encode('utf-8'))
code_challenge = base64.b64encode(h.digest()).decode('utf-8')

session = Session()
session.headers['User-Agent'] = 'MozillaVPN/2.4.0'

login_request = session.get(vpn_login, params={
    'code_challenge': code_challenge,
    'code_challenge_method': 'S256',
    'user_agent': session.headers['User-Agent']
})
check(login_request)
login_parsed = urlparse(login_request.url)
login_params = { k: v[0] for (k, v) in parse_qs(login_parsed.query).items() }

#--------------------------------------
# Prompt for the login credentials
#--------------------------------------
# Get info about the service
client_request = session.get(f"{fxa_clientid}/{login_params['client_id']}")
check(client_request)
client_data=client_request.json()

# Prompt for the user's credentials or get them from the environment
print(f"Logging into {client_data['name']}")
if 'ACCOUNT_EMAIL' in os.environ:
    email = os.environ['ACCOUNT_EMAIL']
    print(f"Logging in as {email}")
else:
    email = input('User Email: ')

if 'ACCOUNT_PASSWORD' in os.environ:
    password = os.environ['ACCOUNT_PASSWORD']
    print('Password: ********')
else:
    password = getpass.getpass()

# Perform PBKDF2 password stretching
salt=f"identity.mozilla.com/picl/v1/quickStretch:{email}"
quickstretch = hashlib.pbkdf2_hmac('sha256', password.encode('utf-8'), salt.encode('utf-8'), 1000)
authpw = hkdf(32, quickstretch, info=b'identity.mozilla.com/picl/v1/authPW')
if debug:
    print(f"salt => {salt}")
    print(f"pbkdf => {quickstretch.hex()}")
    print(f"authpw => {authpw.hex()}")

#------------------------------------------------
# Acquire a session token by logging in
#------------------------------------------------
token_request = session.post(fxa_login, json={
    'email': email,
    'authPW': authpw.hex(),
    'reason': 'signin',
    'service': login_params['client_id'],
    'skipCaseError': True,
    'verificationMethod': 'email-otp',
    'metricsContext': {
        'deviceId': login_params['device_id'],
        'flowBeginTime': int(login_params['flow_begin_time']),
        'flowId': login_params['flow_id']
    }
})
check(token_request, prompt='Login failed')
token_json = token_request.json()
sess_token = bytes.fromhex(token_json['sessionToken'])
if debug:
    print(f"session => {sess_token.hex()}")

#------------------------------------------------
# Request authorization
#------------------------------------------------
authz_payload = json.dumps({
    'client_id': login_params['client_id'],
    'state': login_params['state'],
    'scope': login_params['scope'],
    'access_type': login_params['access_type']
}).encode('utf-8')
authz_hash=hawk_hash(authz_payload, 'application/json')

authz_request = session.post(fxa_authz, data=authz_payload, headers={
    'Authorization': hawk_auth(sess_token, fxa_authz, contenthash=authz_hash),
    'Content-Type': 'application/json'
})
check(authz_request, prompt='Authorization failed')
authz_code=authz_request.json().get('code')
if debug:
    print(f"code => {authz_code}")
    print(f"verify => {code_verifier}")

#------------------------------------------------
# Perform OAuth redirection
#------------------------------------------------
redirect_request = session.get(login_params['redirect_uri'], params={
    'code': authz_code,
    'state': login_params['state'],
    'action': 'signin',
    'device_id': login_params['device_id'],
    'flow_begin_time': int(login_params['flow_begin_time']),
    'flow_id': login_params['flow_id']
})
# Resolve redirects to extract the OAuth code
check(redirect_request, prompt='OAuth redirect failed')
redirect_parsed = urlparse(redirect_request.url)
redirect_params = { k: v[0] for (k, v) in parse_qs(redirect_parsed.query).items() }

#------------------------------------------------
# Verify the OAuth code, and get the token
#------------------------------------------------
oauth_request = session.post(vpn_verify, json={'code': redirect_params['code'], 'code_verifier': code_verifier})
check(oauth_request, prompt="OAuth verify failed")
oauth_json=oauth_request.json()
oauth_token=oauth_request.json().get('token')
if debug:
    print(f"user => {json.dumps(oauth_json['user'], indent=3)}")

# And finally, the result we came here for
print(f"token => {oauth_json['token']}")
